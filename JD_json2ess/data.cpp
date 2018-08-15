#include "data.h"



const eiMatrix l2r = ei_matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
const eiMatrix y2z = ei_matrix(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1);
const eiMatrix inv_y = ei_matrix(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
const eiMatrix mm2m = ei_matrix(0.001f, 0, 0, 0, 0, 0.001f, 0, 0, 0, 0, 0.001f, 0, 0, 0, 0, 1);