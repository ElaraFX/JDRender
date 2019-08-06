#include "setting.h"


#define GLOBAL_STDELEM_SHADER_NAME "global_stdelem_shader"

GlobalSettings g_s;
ElementMap g_hdr_map;


void getHDRList(Json::Value &hdr_list)
{
	g_hdr_map.clear();
	if (hdr_list.isMember("hdrInfo"))
	{
		for (unsigned int i = 0; i < hdr_list["hdrInfo"].size(); i++)
		{
			if (hdr_list["hdrInfo"][i].isMember("hdrId") && hdr_list["hdrInfo"][i].isMember("hdrUrl"))
			{
				g_hdr_map.insert(ElementMap::value_type(hdr_list["hdrInfo"][i]["hdrId"].asInt(), hdr_list["hdrInfo"][i]["hdrUrl"].asString()));
				printf("%d: %s\n", hdr_list["hdrInfo"][i]["hdrId"].asInt(), hdr_list["hdrInfo"][i]["hdrUrl"].asString().c_str());
			}
		}
	}	
}

void getGlobalSettings(Json::Value &global_settings, EH_Context *ctx)
{
	if (global_settings.isMember("renderSettings"))
	{
		if (global_settings["renderSettings"].isMember("type"))
		{
			if (global_settings["renderSettings"]["type"].asString() == "normal")
			{
				g_s.eh_cam.cubemap_render = false;
			}
			else if (global_settings["renderSettings"]["type"].asString() == "panorama")
			{
				g_s.eh_cam.cubemap_render = true;
			}
		}
		if (global_settings["renderSettings"].isMember("width"))
		{
			g_s.eh_cam.image_width = global_settings["renderSettings"]["width"].asInt();
		}
		if (global_settings["renderSettings"].isMember("height"))
		{
			g_s.eh_cam.image_height = global_settings["renderSettings"]["height"].asInt();
		}
		if (global_settings["renderSettings"].isMember("quality"))
		{
			// 0: fast; 1: quality
			int q = EH_RenderQuality(global_settings["renderSettings"]["quality"].asInt());
			if (q == 0)
			{
				g_s.quality = EH_MEDIUM;
			}
			else
			{
				g_s.quality = EH_HIGH; 
			}
		}
	}
}

void getEnvironment(Json::Value &envi, EH_Context *ctx)
{
	if (envi.isMember("environmental"))
	{
		EH_Sky sky;
		sky.enabled = true;
		if (envi["environmental"].isMember("backgroundId"))
		{
			int id = envi["environmental"]["backgroundId"].asInt();
			ElementMap::iterator iter = g_hdr_map.find(id);
			if (iter != g_hdr_map.end())
			{
				sky.hdri_name = iter->second.c_str();
				sky.hdri_rotation = 0.0f;
			}
		}
		if (envi["environmental"].isMember("strength"))
		{
			sky.intensity = envi["environmental"]["strength"].asFloat();
		}
		else
		{
			sky.intensity = 1.0f;
		}
		if (envi["environmental"].isMember("color"))
		{
			int c = envi["environmental"]["color"].asInt();
			sky.color[0] = sky.intensity * float((c / 256 / 256) % 256) / 255.0f;
			sky.color[1] = sky.intensity * ((c / 256) % 256) / 255.0f;
			sky.color[2] = sky.intensity * float(c % 256) / 255.0f;
		}
		
		EH_set_sky(ctx, &sky);

		// sunlight
		if (envi["environmental"].isMember("sun_strength"))
		{
			EH_Sun sun;
			float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			if (envi["environmental"].isMember("sun_matrix"))
			{
				for (unsigned int j = 0; j < envi["environmental"]["sun_matrix"].size() && j < 16; j++)
				{
					mat[j] = envi["environmental"]["sun_matrix"][j].asFloat();
				}
			}
			eiMatrix s_tran = ei_matrix(
				mat[0], mat[1], mat[2], mat[3],
				mat[4], mat[5], mat[6], mat[7],
				mat[8], mat[9], mat[10], mat[11],
				mat[12], mat[13], mat[14], mat[15]
				);
			s_tran = y2z * l2r * s_tran * l2r * y2z;

			float color[3] = {0, 0, 0};
			if (envi["environmental"].isMember("sun_color"))
			{
				int c = envi["environmental"]["sun_color"].asInt();
				color[0] = float((c / 256 / 256) % 256) / 255.0f;
				color[1] = float((c / 256) % 256) / 255.0f;
				color[2] = float(c % 256) / 255.0f;
			}
			memcpy(sun.color, color, sizeof(color));
			sun.intensity = envi["environmental"]["sun_strength"].asFloat();
			sun.soft_shadow = 1.0f;
			EH_set_sun_with_matrix(ctx, &sun, (float*)(s_tran.m));
		}
	}
}


void getGlobalCameras(Json::Value &cameras, EH_Context *ctx)
{
	if (cameras.isMember("camera"))
	{
		EH_add_custom_node(ctx, "max_stdelem", GLOBAL_STDELEM_SHADER_NAME);
		// get default camera
		if (g_s.eh_cam.cubemap_render == false && cameras["camera"].size() > 0)
		{
			if (cameras["camera"][0].isMember("fov"))
			{
				g_s.eh_cam.fov = cameras["camera"][0]["fov"].asFloat() / 180 * EI_PI;
			}
			if (cameras["camera"][0].isMember("far"))
			{
				g_s.eh_cam.far_clip = cameras["camera"][0]["far"].asFloat();
			}
			if (cameras["camera"][0].isMember("aspect"))
			{
				g_s.eh_cam.aspect = cameras["camera"][0]["aspect"].asFloat();
			}
			float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			if (cameras["camera"][0].isMember("matrixWorld"))
			{
				for (unsigned int j = 0; j < cameras["camera"][0]["matrixWorld"].size() && j < 16; j++)
				{
					mat[j] = cameras["camera"][0]["matrixWorld"][j].asFloat();
				}
			}
			eiMatrix c_tran = ei_matrix(
				mat[0], mat[1], mat[2], mat[3],
				mat[4], mat[5], mat[6], mat[7],
				mat[8], mat[9], mat[10], mat[11],
				mat[12], mat[13], mat[14], mat[15]
				);

			c_tran = c_tran * l2r * y2z;
			memcpy(g_s.eh_cam.view_to_world, c_tran.m, sizeof(g_s.eh_cam.view_to_world));
			g_s.camera_number = 1;
		}

		// get multiple cubemap cameras
		if (g_s.eh_cam.cubemap_render == true)
		{
			EH_Camera eh_cam;
			eh_cam.cubemap_render = true;
			eh_cam.image_height = g_s.eh_cam.image_width;
			eh_cam.image_width = g_s.eh_cam.image_width * 6;
			for (unsigned int i = 0; i < cameras["camera"].size(); i++)
			{
				if (cameras["camera"][i].isMember("fov"))
				{
					eh_cam.fov = cameras["camera"][i]["fov"].asFloat() / 180 * EI_PI;
				}
				if (cameras["camera"][i].isMember("far"))
				{
					eh_cam.far_clip = cameras["camera"][0]["far"].asFloat();
				}
				if (cameras["camera"][i].isMember("aspect"))
				{
					eh_cam.aspect = cameras["camera"][i]["aspect"].asFloat();
				}
				float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
				if (cameras["camera"][i].isMember("matrixWorld"))
				{
					for (unsigned int j = 0; j < cameras["camera"][i]["matrixWorld"].size() && j < 16; j++)
					{
						mat[j] = cameras["camera"][i]["matrixWorld"][j].asFloat();
					}
				}
				eiMatrix c_tran = ei_matrix(
					mat[0], mat[1], mat[2], mat[3],
					mat[4], mat[5], mat[6], mat[7],
					mat[8], mat[9], mat[10], mat[11],
					mat[12], mat[13], mat[14], mat[15]
					);

				c_tran = c_tran * l2r * y2z;
				memcpy(eh_cam.view_to_world, c_tran.m, sizeof(eh_cam.view_to_world));
				char inst_name[128] = "";
				sprintf(inst_name, "%s%d", CAMERA_NAME, i);
				EH_add_camera(ctx, &eh_cam, inst_name);
			}
			g_s.camera_number = cameras["camera"].size();
		}
	}
}

void setParameter(EH_Context *ctx)
{
	EH_set_camera(ctx, &g_s.eh_cam);

	// set options
	EH_RenderOptions render_op;
	render_op.quality = g_s.quality;
	EH_set_options_name(ctx, "GlobalOptionsName");
	EH_set_render_options(ctx, &render_op);
}

int getCameraNum()
{
	return g_s.camera_number;
}

int getCameraNameStr(int num, char *out)
{
	if (num < g_s.camera_number)
	{
		if (g_s.eh_cam.cubemap_render)
		{
			sprintf(out, "%s%d", CAMERA_NAME, num);
		}
		else
		{
			memcpy(out, "GlobalCameraInstanceName", 24);
			out[24] = '\0';
		}
		return strlen(out);
	}
	return 0;
}