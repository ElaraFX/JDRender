#include "setting.h"


GlobalSettings g_s;


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
	}
}

void getEnvironment(Json::Value &envi, EH_Context *ctx)
{
	if (envi.isMember("environmental"))
	{
		if (envi["environmental"].isMember("background"))
		{
			EH_Sky sky;
			sky.enabled = true;
			std::string hdr_path = envi["environmental"]["background"].asString();
			sky.hdri_name = hdr_path.c_str();
			sky.hdri_rotation = 0.0f;
			sky.intensity = 1.0f;
			EH_set_sky(ctx, &sky);
		}
	}
}


void getGlobalCameras(Json::Value &cameras, EH_Context *ctx)
{
	if (cameras.isMember("camera"))
	{
		if (cameras["camera"].size() > 0)
		{
			// get default camera
			if (cameras["camera"][0].isMember("fov"))
			{
				g_s.eh_cam.fov = cameras["camera"][0]["fov"].asFloat() / 180 * EI_PI;
			}
			if (cameras["camera"][0].isMember("far"))
			{
				g_s.eh_cam.far_clip = cameras["camera"][0]["far"].asFloat();
			}
			/*if (cameras["camera"][0].isMember("aspect"))
			{
				g_s.eh_cam.aspect = cameras["camera"][0]["aspect"].asFloat();
			}*/
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
		}

		// get multiple cameras
		if (g_s.eh_cam.cubemap_render == true)
		{
			for (unsigned int i = 1; i < cameras["camera"].size(); i++)
			{

			}
		}
	}
}

void setCamera(EH_Context *ctx)
{
	EH_set_camera(ctx, &g_s.eh_cam);
}