#pragma once
#include "jsoncpp/json/json.h"
#include "ei.h"
#include "ElaraHomeAPI.h"
#include "data.h"


extern void getLight(Json::Value &light, EH_Context *ctx);
extern void getIESList(Json::Value &ies_list);