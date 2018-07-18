#pragma once
#include "jsoncpp/json/json.h"
#include "ei.h"
#include "ElaraHomeAPI.h"
#include "data.h"


#define CAMERA_NAME "SceneCamera_"

struct GlobalSettings
{
	int camera_type;           // 0: normal, 1: panorama
	int camera_number;
	int res_x;
	int res_y;
	EH_RenderQuality quality;
	EH_Camera eh_cam;
	GlobalSettings()
	{
		camera_type = 0;
		camera_number = 0;
		res_x = 1024;
		res_y = 768;
		eh_cam.near_clip = 0.1; 
		eh_cam.far_clip = 100000; 
		quality = EH_MEDIUM;
	}
};


extern void setParameter(EH_Context *ctx);
extern void getGlobalCameras(Json::Value &cameras, EH_Context *ctx);
extern void getGlobalSettings(Json::Value &global_settings, EH_Context *ctx);
extern void getEnvironment(Json::Value &envi, EH_Context *ctx);