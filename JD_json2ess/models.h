#pragma once
#include <unordered_map>
#include "jsoncpp/json/json.h"
#include "ei.h"
#include "ElaraHomeAPI.h"
#include "data.h"


#define DEFAULT_MTL "default_mtl"
#define MESH_NAME "c_mesh"
#define MAT_NAME "c_material"


typedef std::tr1::unordered_map<int, std::string> ModelMap;

extern void getModelList(Json::Value &model_list);
extern void getIncludedModels(Json::Value &model, EH_Context *ctx);
extern void getCustomModels(Json::Value &model, EH_Context *ctx);
extern void createDefaultMaterial(EH_Context *ctx);