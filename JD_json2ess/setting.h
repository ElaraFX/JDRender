#pragma once
#include "jsoncpp/json/json.h"
#include "ei.h"
#include "ElaraHomeAPI.h"
#include "data.h"


struct GlobalSettings
{
	int camera_type;           // 0: normal, 1: panorama
	int res_x;
	int res_y;
	EH_Camera eh_cam;
	GlobalSettings()
	{
		camera_type = 0;
		res_x = 1024;
		res_y = 768;
	}
};


extern void setCamera(EH_Context *ctx);
extern void getGlobalCameras(Json::Value &cameras, EH_Context *ctx);
extern void getGlobalSettings(Json::Value &global_settings, EH_Context *ctx);
extern void getEnvironment(Json::Value &envi, EH_Context *ctx);