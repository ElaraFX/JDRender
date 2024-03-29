#include "lights.h"

ElementMap g_ies_map;

#define ILLUM_TO_INTENSITY_PARAM				0.0015f
#define LUMINOUS_TO_RADIANT						683.0f
#define ILLUM_TO_INTENSITY_PARAM_RADIANCE		0.699f

enum LIGHT_UNITS_TYPE
{
	LIGHT_UNITS_IMAGE = 0,
	LIGHT_UNITS_LUM_POWER,
	LIGHT_UNITS_LUM,
	LIGHT_UNITS_RAD_POWER,
	LIGHT_UNITS_RAD,

	LIGHT_UNITS_MAX
};

float convert_light_intensity(
	float intens, 
	float light_area,
	LIGHT_UNITS_TYPE units)
{
	float intensity = 0.0f;
	float meters_scale_param = (0.001f / METERS_SCALE) * (0.001f / METERS_SCALE);

	if (units == LIGHT_UNITS_IMAGE) // Image
	{
		intensity = intens * light_area;
	}
	else if (units == LIGHT_UNITS_LUM_POWER) // Luminous power (lm)
	{
		intensity = intens * meters_scale_param / ILLUM_TO_INTENSITY_PARAM;
	}
	else if (units == LIGHT_UNITS_LUM) // Luminance (lm/m^2/sr)
	{
		intensity = intens * light_area / (LUMINOUS_TO_RADIANT * ILLUM_TO_INTENSITY_PARAM_RADIANCE);
	}
	else if (units == LIGHT_UNITS_RAD_POWER) // Radiant power (W)
	{
		intensity = intens * meters_scale_param * LUMINOUS_TO_RADIANT / ILLUM_TO_INTENSITY_PARAM;
	}
	else if (units == LIGHT_UNITS_RAD) // Radiance (W/m^2/sr)
	{
		intensity = intens * light_area / ILLUM_TO_INTENSITY_PARAM_RADIANCE;
	}

	return intensity;
}

void getIESList(Json::Value &ies_list)
{
	g_ies_map.clear();
	if (ies_list.isMember("iesInfo"))
	{
		for (unsigned int i = 0; i < ies_list["iesInfo"].size(); i++)
		{
			if (ies_list["iesInfo"][i].isMember("iesId") && ies_list["iesInfo"][i].isMember("iesUrl"))
			{
				g_ies_map.insert(ElementMap::value_type(ies_list["iesInfo"][i]["iesId"].asInt(), ies_list["iesInfo"][i]["iesUrl"].asString()));
				printf("%d: %s\n", ies_list["iesInfo"][i]["iesId"].asInt(), ies_list["iesInfo"][i]["iesUrl"].asString().c_str());
			}
		}
	}	
}

void getLight(Json::Value &light, EH_Context *ctx)
{
	if (light.isMember("customLights"))
	{
		for (unsigned int i = 0; i < light["customLights"].size(); i++)
		{
			EH_Light l;	
			// default value
			l.type = EH_LIGHT_POINT;
			l.intensity = 1.0f;			
			l.size[0] = 1.0f; /* width */
			l.size[1] = 1.0f; /* height */
			std::string str;

			if (light["customLights"][i].isMember("type"))
			{
				if (light["customLights"][i]["type"].asString() == "sphere")
				{
					l.type = EH_LIGHT_SPHERE;
					if (light["customLights"][i].isMember("radius"))
					{
						l.size[0] = max(MIN_LIGHT_SIZE, light["customLights"][i]["radius"].asFloat());
					}
				}
				else if (light["customLights"][i]["type"].asString() == "quad")
				{
					l.type = EH_LIGHT_QUAD;
					if (light["customLights"][i].isMember("width"))
					{
						l.size[0] = max(MIN_LIGHT_SIZE, light["customLights"][i]["length"].asFloat());
					}
					if (light["customLights"][i].isMember("height"))
					{
						l.size[1] = max(MIN_LIGHT_SIZE, light["customLights"][i]["height"].asFloat());
					}
				}
				else if (light["customLights"][i]["type"].asString() == "spotlight")
				{
					l.type = EH_LIGHT_IES;
					if (light["customLights"][i].isMember("iesId"))
					{
						int id = light["customLights"][i]["iesId"].asInt();
						ElementMap::iterator iter = g_ies_map.find(id);
						if (iter == g_ies_map.end())
						{
							continue;
						}
						l.ies_filename = iter->second.c_str();
					}
				}

				// common attributes
				if (light["customLights"][i].isMember("strength"))
				{
					l.intensity = light["customLights"][i]["strength"].asFloat();
				}
				if (light["customLights"][i].isMember("color"))
				{
					int c = light["customLights"][i]["color"].asInt();
					l.light_color[0] = float((c / 256 / 256) % 256) / 255.0f;
					l.light_color[1] = float((c / 256) % 256) / 255.0f;
					l.light_color[2] = float(c % 256) / 255.0f;
				}
				if (light["customLights"][i].isMember("unit"))
				{
					if (light["customLights"][i]["unit"].asString() == "lm")
					{
						if (l.type == EH_LIGHT_QUAD)
						{
							l.intensity = convert_light_intensity(l.intensity, l.size[0] * l.size[1], LIGHT_UNITS_LUM_POWER);
						}
						else if (l.type == EH_LIGHT_SPHERE)
						{
							l.intensity = convert_light_intensity(l.intensity, 4.0f * EI_PI * l.size[0] * l.size[0], LIGHT_UNITS_LUM_POWER);
						}
						else if (l.type == EH_LIGHT_IES)
						{
							l.intensity /= METERS_SCALE * METERS_SCALE;
						}
					}
					else
					{
						l.intensity /= METERS_SCALE * METERS_SCALE;
					}
				}
				else
				{
					l.intensity /= METERS_SCALE * METERS_SCALE;
				}

				if (l.type == EH_LIGHT_IES)
				{
					l.intensity /= 1000.0f;
				}

				float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
				if (light["customLights"][i].isMember("matrixWorld"))
				{
					for (unsigned int j = 0; j < light["customLights"][i]["matrixWorld"].size() && j < 16; j++)
					{
						mat[j] = light["customLights"][i]["matrixWorld"][j].asFloat();
					}
				}
				eiMatrix l_tran = ei_matrix(
					mat[0], mat[1], mat[2], mat[3],
					mat[4], mat[5], mat[6], mat[7],
					mat[8], mat[9], mat[10], mat[11],
					mat[12], mat[13], mat[14], mat[15]
					);

				l_tran = y2z * l2r * l_tran * l2r * y2z;
				memcpy(l.light_to_world, l_tran.m, sizeof(l.light_to_world));

				char light_name[32] = "";
				sprintf(light_name, "customLight_%d", i);
				EH_add_light(ctx, light_name, &l);
			}
		}
	}
}