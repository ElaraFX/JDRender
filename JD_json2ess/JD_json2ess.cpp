// JD_json2ess.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <fstream>
#include <sstream>
#include "JD_json2ess.h"
#include "models.h"
#include "lights.h"
#include "setting.h"


// 按照特定字符串将json文件读入并且分割
// 按照京东要求是需要分割为两个json
void read_and_split_json_file(const char *filename, std::string &json1, std::string &json2, std::string split)
{
	//从文件读入到string里  
	std::ifstream ifile(filename);  
	//将文件读入到ostringstream对象buf中  
	std::ostringstream buf1, buf2;  
	char ch;  
	while(buf1 && ifile.get(ch))  
	{
		if (split.size() == 0)
		{
			buf1.put(ch);
		}
		else if (ch != split[0])
		{
			buf1.put(ch); 
		}
		else
		{
			json1 = buf1.str();
			break;
		}
	}

	while(buf2 && ifile.get(ch))  
	{
		buf2.put(ch);
	}
	json2 = buf2.str();  
}

int getCameraNumber()
{
	return getCameraNum();
}

int getCameraName(int num, char *out)
{
	return getCameraNameStr(num, out);
}

bool translateJson(const char* json_file_path, const char* out_file_path)
{
	// initial
	try {
		EH_Context *pContext = EH_create();
		EH_ExportOptions option;
		option.base85_encoding = true;
		option.left_handed = false;
		EH_begin_export(pContext, out_file_path, &option);
		createDefaultMaterial(pContext);

		// read and translate json
		std::string json1, json2, split;
		read_and_split_json_file(json_file_path, json1, json2, std::string("|"));
		Json::Value root1, root2;
		Json::Reader reader;
		if(!reader.parse(json1, root1))
		{
			std::cout<<"return json1 error."<<std::endl;
			return false;
		}

		if(!reader.parse(json2, root2))
		{
			std::cout<<"return json2 error."<<std::endl;
			return false;
		}

		getModelList(root2);
		getTextureList(root2);
		getHDRList(root2);
		getIESList(root2);
		getIncludedModels(root1, pContext);
		getCustomModels(root1, pContext);
		getLight(root1, pContext);
		getGlobalSettings(root1, pContext);
		getEnvironment(root1, pContext);
		getGlobalCameras(root1, pContext);
		setParameter(pContext);

		EH_end_export(pContext);
		EH_delete(pContext);
		return true;
	}
	catch (...)
	{
		return false;
	}
}
//
int main()
{
	if (!translateJson("D:/work/JDRender/JD_json2ess/new1.json", "d:/jd1.ess"))
	{
		printf("json parse false!\n");
	}
	char xx[128] = "";
	getCameraName(0, xx);
	system("pause");
	return 1;
}
