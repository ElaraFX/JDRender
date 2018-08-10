//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "MeshToolPlugin.h"
#ifdef MAX_RELEASE_R14 // Max 2012
#include <maxscript/maxscript.h>
#else
#include <maxscrpt/maxscrpt.h>
#endif

#define MeshToolPlugin_CLASS_ID	Class_ID(0x79a7abce, 0x65446e84)


class MeshToolPlugin : public GUP
{
public:
	//Constructor/Destructor
	MeshToolPlugin();
	virtual ~MeshToolPlugin();

	// GUP Methods
	virtual DWORD     Start();
	virtual void      Stop();
	virtual DWORD_PTR Control(DWORD parameter);
	virtual void      DeleteThis();

	// Loading/Saving
	virtual IOResult Save(ISave* isave);
	virtual IOResult Load(ILoad* iload);
};



class MeshToolPluginClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 		{ return new MeshToolPlugin(); }
	virtual const TCHAR *	ClassName() 			{ return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID SuperClassID() 				{ return GUP_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return MeshToolPlugin_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return GetString(IDS_CATEGORY); }

	virtual const TCHAR* InternalName() 			{ return _T("MeshToolPlugin"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hInstance; }					// returns owning module handle
	

};


ClassDesc2* GetMeshToolPluginDesc() { 
	static MeshToolPluginClassDesc MeshToolPluginDesc;
	return &MeshToolPluginDesc; 
}




MeshToolPlugin::MeshToolPlugin()
{

}

MeshToolPlugin::~MeshToolPlugin()
{

}

void MeshToolPlugin::DeleteThis()
{
	delete this;
}

// Activate and Stay Resident
DWORD MeshToolPlugin::Start()
{
    HGLOBAL     res_handle = NULL;
    HRSRC       res;
    char*       res_data;
    DWORD       res_size;

    res = FindResource(hInstance, MAKEINTRESOURCE(IDR_TEXT1), TEXT("TEXT"));
    if (!res)
        return GUPRESULT_KEEP;
    res_handle = LoadResource(hInstance, res);
    if (!res_handle)
        return GUPRESULT_KEEP;
    res_data = (char*)LockResource(res_handle);
    res_size = SizeofResource(hInstance, res);

    char* text = (char*)malloc(res_size + 1);
    memcpy(text, res_data, res_size);
    text[res_size] = 0;

    TSTR m_str = TSTR::FromUTF8(text);
    FPValue fpv;
    ExecuteMAXScriptScript(m_str, TRUE, &fpv);
	return GUPRESULT_KEEP;
}

void MeshToolPlugin::Stop()
{
	#pragma message(TODO("Do plugin un-initialization here"))
}

DWORD_PTR MeshToolPlugin::Control( DWORD /*parameter*/ )
{
	return 0;
}

IOResult MeshToolPlugin::Save(ISave* /*isave*/)
{
	return IO_OK;
}

IOResult MeshToolPlugin::Load(ILoad* /*iload*/)
{
	return IO_OK;
}

