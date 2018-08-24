#include <vector>
#include "models.h"


ElementMap g_model_map;
ElementMap g_texture_map;
EH_Material default_material;


void createDefaultMaterial(EH_Context *ctx)
{
	// create default material
	const char *simple_mtl = DEFAULT_MTL;
	float diffuse[3] = {1, 1, 1};
	memcpy(default_material.diffuse_color, diffuse, sizeof(EH_RGB));
	EH_add_material(ctx, simple_mtl, &default_material);
}

void getModelList(Json::Value &model_list)
{
	g_model_map.clear();
	if (model_list.isMember("modelInfo"))
	{
		for (unsigned int i = 0; i < model_list["modelInfo"].size(); i++)
		{
			if (model_list["modelInfo"][i].isMember("modelId") && model_list["modelInfo"][i].isMember("modelUrl"))
			{
				g_model_map.insert(ElementMap::value_type(model_list["modelInfo"][i]["modelId"].asInt(), model_list["modelInfo"][i]["modelUrl"].asString()));
				//printf("%d: %s\n", model_list["modelInfo"][i]["modelId"].asInt(), model_list["modelInfo"][i]["modelUrl"].asString().c_str());
			}
		}
	}	
}

void getTextureList(Json::Value &texture_list)
{
	g_texture_map.clear();
	if (texture_list.isMember("textureInfo"))
	{
		for (unsigned int i = 0; i < texture_list["textureInfo"].size(); i++)
		{
			if (texture_list["textureInfo"][i].isMember("textureId") && texture_list["textureInfo"][i].isMember("textureUrl"))
			{
				g_texture_map.insert(ElementMap::value_type(texture_list["textureInfo"][i]["textureId"].asInt(), texture_list["textureInfo"][i]["textureUrl"].asString()));
				printf("%d: %s\n", texture_list["textureInfo"][i]["textureId"].asInt(), texture_list["textureInfo"][i]["textureUrl"].asString().c_str());
			}
		}
	}	
}

void getIncludedModels(Json::Value &model, EH_Context *ctx)
{
	if (model.isMember("models"))
	{
		for (unsigned int i = 0; i < model["models"].size(); i++)
		{
			float mat[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
			if (model["models"][i].isMember("matrixWorld"))
			{
				for (unsigned int j = 0; j < model["models"][i]["matrixWorld"].size() && j < 16; j++)
				{
					mat[j] = model["models"][i]["matrixWorld"][j].asFloat();
				}
			}

			if (model["models"][i].isMember("modelId"))
			{
				int modelId = model["models"][i]["modelId"].asInt();
				ElementMap::iterator iter = g_model_map.find(modelId);
				if (iter == g_model_map.end())
				{
					continue;
				}

				/**< 加载外部ESS */
				eiMatrix include_ess_mat = ei_matrix( /* 引用外部ESS模型的变化矩阵 */
					mat[0], mat[1], mat[2], mat[3],
					mat[4], mat[5], mat[6], mat[7],
					mat[8], mat[9], mat[10], mat[11],
					mat[12], mat[13], mat[14], mat[15]
				);
				
				include_ess_mat = mm2m * y2z * l2r * include_ess_mat * l2r * y2z;
				EH_AssemblyInstance include_inst;

				include_inst.filename = (iter->second).c_str(); /* 需要包含的ESS */
				std::string include_inst_name(include_inst.filename);
				char num[20] = "";
				sprintf(num, "_%d", i);
				include_inst_name += num;
				memcpy(include_inst.mesh_to_world, (include_ess_mat/* * inch2mm*/).m, sizeof(include_inst.mesh_to_world));
				EH_add_assembly_instance(ctx, include_inst_name.c_str(), &include_inst); /* include_test_ess 是ESS中节点的名字不能重名 */
				
				// declare object_id color
				eiColor object_id = ei_color(0, 0, 0);
				if (model["models"][i].isMember("object_id"))
				{
					int c = model["models"][i]["object_id"].asInt();
					object_id.r = float((c / 256 / 256) % 256) / 255.0f;
					object_id.g = float((c / 256) % 256) / 255.0f;
					object_id.b = float(c % 256) / 255.0f;
					char color_str[256] = {'\0'};
					sprintf(color_str, "%f %f %f", object_id.r, object_id.g, object_id.b);
 					EH_declare_instance_param(ctx, include_inst_name.c_str(), "color", "MaxObjectID", color_str);
				}
			}
		}
	}
}

// ------------- custom models -------------- //
bool isDigital(char c)
{
	if (c <= '9' && c >= '0')
	{
		return true;
	}
	if (c == '.' || c == '-' || c == 'E' || c == 'e')
	{
		return true;
	}
	return false;
}

int get_int(std::string &str, size_t &pos)
{
	while (!isDigital(str[pos]))
	{
		pos++;
	}
	size_t last_pos = pos;
	while (isDigital(str[pos]))
	{
		pos++;
	}
	std::string sub_str = str.substr(last_pos, pos - last_pos);
	return atoi(sub_str.c_str());
}

void get_indice(std::string &str, size_t &pos, std::vector<unsigned int> &t_idx, std::vector<unsigned int> &n_idx, std::vector<unsigned int> &uv_idx)
{
	// one face contains three vertices
	t_idx.push_back((unsigned int)get_int(str, pos) - 1);
	n_idx.push_back((unsigned int)get_int(str, pos) - 1);
	uv_idx.push_back((unsigned int)get_int(str, pos) - 1);
	t_idx.push_back((unsigned int)get_int(str, pos) - 1);
	n_idx.push_back((unsigned int)get_int(str, pos) - 1);
	uv_idx.push_back((unsigned int)get_int(str, pos) - 1);
	t_idx.push_back((unsigned int)get_int(str, pos) - 1);
	n_idx.push_back((unsigned int)get_int(str, pos) - 1);
	uv_idx.push_back((unsigned int)get_int(str, pos) - 1);
}

float get_float(std::string &str, size_t &pos)
{
	while (!isDigital(str[pos]))
	{
		pos++;
	}
	size_t last_pos = pos;
	while (isDigital(str[pos]))
	{
		pos++;
	}
	std::string sub_str = str.substr(last_pos, pos - last_pos);
	return atof(sub_str.c_str());
}

void get_vector(std::string &str, size_t &pos, std::vector<float> &container)
{
	container.push_back(get_float(str, pos));
	container.push_back(get_float(str, pos));
	container.push_back(get_float(str, pos));
}

void get_vector2(std::string &str, size_t &pos, std::vector<float> &container)
{
	container.push_back(get_float(str, pos));
	container.push_back(get_float(str, pos));
}
void parseCustomModel(std::string &mesh, EH_Context *ctx, int idx, EH_Mesh &custom_mesh)
{
	size_t pos = 0, last_pos = 0;
	eiVector val;
	std::vector<float> vertex_data;
	std::vector<float> normal_data;
	std::vector<float> uv_data;
	std::vector<unsigned int> t_indice_data;
	std::vector<unsigned int> n_indice_data;
	std::vector<unsigned int> uv_indice_data;
	
	// get revertNormal info

	// get vertices
	while (1)
	{
		pos = mesh.find("v ", last_pos);
		if (pos == std::string::npos)
		{
			break;
		}
		get_vector(mesh, pos, vertex_data);
		last_pos = pos + 1;
	}

	// get normals
	pos = last_pos = 0;
	while (1)
	{
		pos = mesh.find("vn ", last_pos);
		if (pos == std::string::npos)
		{
			break;
		}
		get_vector(mesh, pos, normal_data);
		last_pos = pos + 1;
	}
	
	// get uv 
	pos = last_pos = 0;
	while (1)
	{
		pos = mesh.find("vt ", last_pos);
		if (pos == std::string::npos)
		{
			break;
		}
		get_vector2(mesh, pos, uv_data);
		last_pos = pos + 1;
	}

	// get indice 
	unsigned int face_num = 0;
	pos = last_pos = 0;
	while (1)
	{
		pos = mesh.find("f ", last_pos);
		if (pos == std::string::npos)
		{
			break;
		}
		get_indice(mesh, pos, t_indice_data, n_indice_data, uv_indice_data);
		last_pos = pos + 1;
		face_num++;
	}
	
	char mesh_name[32] = "";
	sprintf(mesh_name, "%s_%d", MESH_NAME, idx);
	custom_mesh.num_verts = vertex_data.size() / 3;
	custom_mesh.num_faces = face_num;
	custom_mesh.verts = (EH_Vec*)(&vertex_data[0]);
	custom_mesh.face_indices = (uint_t*)(&t_indice_data[0]);
	custom_mesh.uvs = (EH_Vec2*)(&uv_data[0]);
	custom_mesh.normals = (EH_Vec*)(&normal_data[0]);
	custom_mesh.n_indices = (uint_t*)(&n_indice_data[0]);
	custom_mesh.uv_indices = (uint_t*)(&uv_indice_data[0]);
	EH_add_mesh(ctx, mesh_name, &custom_mesh);
}

void getCustomModels(Json::Value &model, EH_Context *ctx)
{
	if (model.isMember("customModels"))
	{
		for (unsigned int i = 0; i < model["customModels"].size(); i++)
		{
			if (model["customModels"][i].isMember("model"))
			{
				EH_Mesh mesh;
				parseCustomModel(model["customModels"][i]["model"].asString(), ctx, i, mesh);

				// matrix
				eiMatrix m_tran = l2r * y2z;

				// generate material
				EH_Vray_Material mat;
				std::string diffuse_tex, bump_tex;
				if (model["customModels"][i].isMember("material"))
				{
					if (model["customModels"][i]["material"].isMember("id"))
					{
						if (model["customModels"][i]["material"].isMember("diffuse"))
						{
							int c = model["customModels"][i]["material"]["diffuse"].asInt();
							mat.diffuse_color[0] = float((c / 256 / 256) % 256) / 255.0f;
							mat.diffuse_color[1] = float((c / 256) % 256) / 255.0f;
							mat.diffuse_color[2] = float(c % 256) / 255.0f;
						}
						if (model["customModels"][i]["material"].isMember("specular"))
						{
							int c = model["customModels"][i]["material"]["specular"].asInt();
							mat.diffuse_color[0] = float((c / 256 / 256) % 256) / 255.0f;
							mat.diffuse_color[1] = float((c / 256) % 256) / 255.0f;
							mat.diffuse_color[2] = float(c % 256) / 255.0f;
						}
						if (model["customModels"][i]["material"].isMember("glossiness"))
						{
							mat.glossiness = model["customModels"][i]["material"]["glossiness"].asFloat();
						}
						if (model["customModels"][i]["material"].isMember("ior"))
						{
							mat.specular_fresnel = model["customModels"][i]["material"]["ior"].asFloat();
						}
					}
				}
				if (model["customModels"][i].isMember("diffuse_textureId"))
				{
					int id = model["customModels"][i]["diffuse_textureId"].asInt();
					ElementMap::iterator iter = g_texture_map.find(id);
					if (iter != g_texture_map.end())
					{
						mat.diffuse_tex.filename = iter->second.c_str();
					}
				}
				if (model["customModels"][i].isMember("bump_textureId"))
				{
					int id = model["customModels"][i]["bump_textureId"].asInt();
					ElementMap::iterator iter = g_texture_map.find(id);
					if (iter != g_texture_map.end())
					{
						mat.normal_bump = false;
						mat.bump_tex.filename = iter->second.c_str();
					}
				}
				if (model["customModels"][i].isMember("bump_mult"))
				{
					mat.bump_weight = model["customModels"][i]["bump_mult"].asFloat();
				}
				if (model["customModels"][i].isMember("repeat"))
				{
					mat.diffuse_tex.repeat_u = model["customModels"][i]["repeat"]["x"].asFloat();
					mat.diffuse_tex.repeat_v = model["customModels"][i]["repeat"]["y"].asFloat();
				}
				if (model["customModels"][i].isMember("offset"))
				{
					mat.diffuse_tex.offset_u = model["customModels"][i]["offset"]["x"].asFloat();
					mat.diffuse_tex.offset_v = model["customModels"][i]["offset"]["y"].asFloat();
				}
				char mat_name[32] = "";
				sprintf(mat_name, "%s_%d", MAT_NAME, i);
				EH_add_vray_material(ctx, mat_name, &mat);

				// generate instance
				char mesh_name[32] = "";
				sprintf(mesh_name, "%s_%d", MESH_NAME, i);
				char inst_name[32] = "";
				sprintf(inst_name, "inst_%s", mesh_name);
				EH_MeshInstance inst;
				inst.mesh_name = mesh_name;
				inst.mtl_names[0] = mat_name;
				memcpy(inst.mesh_to_world, m_tran.m, sizeof(inst.mesh_to_world));
				EH_add_mesh_instance(ctx, inst_name, &inst);

				// declare object_id color
				eiColor object_id = ei_color(0, 0, 0);
				if (model["customModels"][i].isMember("object_id"))
				{
					int c = model["customModels"][i]["object_id"].asInt();
					object_id.r = float((c / 256 / 256) % 256) / 255.0f;
					object_id.g = float((c / 256) % 256) / 255.0f;
					object_id.b = float(c % 256) / 255.0f;
					char color_str[256] = {'\0'};
					sprintf(color_str, "%f %f %f", object_id.r, object_id.g, object_id.b);
 					EH_declare_instance_param(ctx, inst_name, "color", "MaxObjectID", color_str);
				}
			}
		}
	}
}