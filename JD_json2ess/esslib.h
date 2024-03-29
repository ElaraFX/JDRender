/**************************************************************************
 * Copyright (C) 2017 Rendease Co., Ltd.
 * All rights reserved.
 *
 * This program is commercial software: you must not redistribute it 
 * and/or modify it without written permission from Rendease Co., Ltd.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * End User License Agreement for more details.
 *
 * You should have received a copy of the End User License Agreement along 
 * with this program.  If not, see <http://www.rendease.com/licensing/>
 *************************************************************************/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ESSLIB_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ESSLIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
//#ifdef ESSLIB_EXPORTS
//#define ESSLIB_API __declspec(dllexport)
//#else
//#define ESSLIB_API __declspec(dllimport)
//#endif

#ifndef ESS_LIB_H
#define ESS_LIB_H

#pragma once

#include <vector>
#include <string>
#include "esswriter.h"
#include "ElaraHomeAPI.h"


class EssExporter {
private:
	std::vector<std::string> mElInstances;
	std::vector<std::string> mElMaterials;
	std::string mCamName;
	std::vector<std::string> mDefaultWallMatName;
	std::vector<std::string> mDefaultMatName;
	std::string mEnvName;
	std::string mRootPath;
	std::string mOptionName;
	EssWriter mWriter;
	int mLightSamples;
	bool mCheckNormal;
	bool mIsLeftHand;
	bool mUseDisplacement;
	bool mIsNeedEmitGI;

public:
	EH_display_callback display_callback;
	EH_ProgressCallback progress_callback;
	EH_LogCallback log_callback;
	
public:
	EssExporter(void);
	~EssExporter();
	bool BeginExport(std::string &filename, const EH_ExportOptions &option, const bool check_normal);
	void SetLightSamples(const int samples);
	bool AddCamera(const EH_Camera &cam, bool panorama, int panorama_size, std::string &NodeName);
	void AddMesh(const EH_Mesh& model, const std::string &modelName);
	void AddMeshInstance(const char *instName, const EH_MeshInstance &meshInst);
	bool AddLight(const EH_Light& light, std::string &lightName, bool is_show_area);
	bool AddMaterial(const EH_Material& mat, std::string &matName);
	bool AddVrayMaterial(const EH_Vray_Material& mat, std::string &matName);
	void SetOptionName(std::string &name);
	void AddDefaultOption();
	void AddMediumOption();
	void AddLowOption();
	void AddHighOption();
	void SetExposure(const EH_Exposure &exposure);
	void SetGamma(const EH_Gamma &gamma);
	bool AddDefaultMaterial();
	void AddCustomOption(const EH_CustomRenderOptions &option);
	bool AddBackground(const EH_Sky *sky, bool enable_emit_GI);
	bool AddSun(const EH_Sun &sun);
	bool AddSun(const EH_Sun &sun, eiMatrix &mat);
	void SetTexPath(std::string &path);
	void AddAssemblyInstance(const char *name, const EH_AssemblyInstance &assembly_inst);
	void DeclareInstanceParam(const char *name, const char *type, const char *attribute, const char *storage_class);
	void AddCustomNode(const char *type, const char *name);
	void AddMaterialFromEss(const EH_Material &mat, std::string matName, const char *essName);
	void EndExport();
};

#endif
