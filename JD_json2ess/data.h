#pragma once
#include <unordered_map>
#include "ei.h"

#define METERS_SCALE 0.001
#define MIN_LIGHT_SIZE 4.0f

extern const eiMatrix l2r;
extern const eiMatrix y2z;
extern const eiMatrix mm2m;


typedef std::tr1::unordered_map<int, std::string> ElementMap;