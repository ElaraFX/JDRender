#include "lights.h"

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

			if (light["customLights"][i].isMember("type"))
			{
				if (light["customLights"][i]["type"].asString() == "point")
				{
					l.type = EH_LIGHT_POINT;
				}
				else if (light["customLights"][i]["type"].asString() == "quad")
				{
					l.type = EH_LIGHT_QUAD;
				}
				else
				{
					l.type = EH_LIGHT_POINT;
				}
			}

			if (light["customLights"][i].isMember("strength"))
			{
				l.intensity = light["customLights"][i]["strength"].asFloat();
			}
			if (light["customLights"][i].isMember("width"))
			{
				l.size[0] = light["customLights"][i]["length"].asFloat();
			}
			if (light["customLights"][i].isMember("height"))
			{
				l.size[1] = light["customLights"][i]["height"].asFloat();
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
			memcpy(l.light_to_world, l_tran.m, sizeof(l.light_to_world));

			char light_name[32] = "";
			sprintf(light_name, "customLight_%d", i);
			EH_add_light(ctx, light_name, &l);
		}
	}
}