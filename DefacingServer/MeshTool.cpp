// MeshTool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <string>
#include <assert.h>
#include <io.h>
#include <algorithm>

using namespace std;
HWND mainwindow;
ULONGLONG maxFileSize;

BOOL CALLBACK FWindow1(HWND h, LPARAM lp)
{
    TCHAR str[256];
    ::GetWindowText(h, str, 64);

    if (_tcsstr(str, _T("Autodesk 3ds Max")))
    {
        mainwindow = h;
    }
    return 1;
}

typedef std::basic_string<TCHAR> tstring;

tstring FindMaxFile(tstring path)
{
    tstring	maxFilename;
    maxFilename = path + _T("*.max");
    _tfinddata_t fileInfo;
    long handle = _tfindfirst(maxFilename.c_str(), &fileInfo);
    if (handle == -1L)
    {
        return tstring();
    }
    maxFilename = path + fileInfo.name;
    return maxFilename;
}

tstring FindMaxFile2(tstring folderPath)
{
    _tfinddata_t FileInfo;
    tstring strfind = folderPath + _T("\\*");
    intptr_t Handle = _tfindfirst(strfind.c_str(), &FileInfo);

    if (Handle == -1L)
    {
        return tstring();
    }
    do {
        //判断是否有子目录  
        if (FileInfo.attrib & _A_SUBDIR)
        {
            //这个语句很重要  
            if ((_tcscmp(FileInfo.name, _T(".")) != 0) && (_tcscmp(FileInfo.name, _T("..")) != 0))
            {
                tstring newPath = folderPath + _T("\\") + FileInfo.name;
                tstring maxPath = FindMaxFile2(newPath);
                if (!maxPath.empty())
                {
                    _findclose(Handle);
                    return maxPath;
                }
            }
        }
        else
        {
            tstring fileName = FileInfo.name;
            tstring maxExt = fileName.substr(fileName.size() - 3, 3);
            if (maxExt == _T("max"))
            {
                maxFileSize = FileInfo.size;
                tstring maxPath = folderPath + _T("\\") + FileInfo.name;
                _findclose(Handle);
                return maxPath;
            }
        }
    } while (_tfindnext(Handle, &FileInfo) == 0);

    _findclose(Handle);
    return tstring();
}

bool IsProcessRunning(const TCHAR *processName)
{
    bool exists = false;
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry))
        while (Process32Next(snapshot, &entry))
            if (!wcsicmp(entry.szExeFile, processName))
                exists = true;

    CloseHandle(snapshot);
    return exists;
}

std::wstring GetStringValueFromHKLM(const std::wstring& regSubKey, const std::wstring& regValue)
{
    TCHAR	valueBuf[MAX_PATH];
    DWORD len = MAX_PATH;
    LONG rc = RegGetValueW(
        HKEY_LOCAL_MACHINE,
        regSubKey.c_str(),
        regValue.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        &valueBuf,
        &len
    );
    if (rc == 0)
    {
        return valueBuf;
    }
    else
    {
        return _T("");
    }    
}

void startup(const TCHAR* lpApplicationName)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcessW(NULL,   // No module name (use command line)
        (TCHAR*)lpApplicationName,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }

    // Wait until child process exits.
    //WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int _tmain(int argc, TCHAR* argv[])
{
    if (argc != 4)
    {
        printf("Params don't correct!\n");
        return 0;
    }
    TCHAR exePath[MAX_PATH];
    //TCHAR* exportPath = "E:\\Mesh\\";
    TCHAR* zipFilename = argv[1];
    TCHAR* optimizeLevel = argv[2];
    TCHAR* taskID = argv[3];
    GetCurrentDirectory(MAX_PATH, exePath);

    tstring removeMaxs = tstring(_T("rd /S/Q ")) + exePath + tstring(_T("\\MAX"));
    tstring removeEsss = tstring(_T("rd /S/Q ")) + exePath + tstring(_T("\\ESS"));
    tstring removeObjs = tstring(_T("rd /S/Q ")) + exePath + tstring(_T("\\OBJ"));
    _tsystem(removeMaxs.c_str());
    _tsystem(removeEsss.c_str());
    _tsystem(removeObjs.c_str());

    tstring makeMAXFolder = tstring(_T("md ")) + exePath + tstring(_T("\\MAX\\")) + taskID;
    tstring makeESSFolder = tstring(_T("md ")) + exePath + tstring(_T("\\ESS"));
    tstring makeOBJFolder = tstring(_T("md ")) + exePath + tstring(_T("\\OBJ"));
    _tsystem(makeMAXFolder.c_str());
    _tsystem(makeESSFolder.c_str());
    _tsystem(makeOBJFolder.c_str());

    //tstring max2016Path = GetStringValueFromHKLM(_T("SOFTWARE\\Autodesk\\3dsMax\\18.0"), _T("Installdir"));
    //tstring max2017Path = GetStringValueFromHKLM(_T("SOFTWARE\\Autodesk\\3dsMax\\19.0"), _T("Installdir"));
    tstring max2018Path = GetStringValueFromHKLM(_T("SOFTWARE\\Autodesk\\3dsMax\\20.0"), _T("Installdir"));

    if (!IsProcessRunning(_T("3dsmax.exe")))
    {
        if (max2018Path.empty())
        {
            printf("3DsMax 2018 don't install!\n");
            return 0;
        }
        tstring maxExePath = _T("\"") + max2018Path + _T("3dsmax.exe\"");
        startup(maxExePath.c_str());
        Sleep(5000);
        if (!IsProcessRunning(_T("3dsmax.exe")))
        {
            printf("3DsMax don't running!\n");
            return 0;
        }        
    }
    
    TCHAR	zipCommandStr[MAX_PATH];
    _stprintf_s(zipCommandStr, _T("7z x \"%s\" -y -aoa -o\"%s\\Max\\%s\\\""), zipFilename, exePath, taskID);

    _tsystem(zipCommandStr);

    TCHAR	maxPathStr[MAX_PATH];
    _stprintf_s(maxPathStr, _T("%s\\Max\\%s\\"), exePath, taskID);
    TCHAR	maxFilenameStr[MAX_PATH];
    maxFileSize = 0;
    tstring	maxFilename = FindMaxFile2(maxPathStr);
    if (maxFilename.empty())
    {
        printf("Can't find max file!\n");
        tstring errorCommand = _T("UploadESSAndObj 0 0 0 0 0 0 0 0 3 ") + tstring(taskID);
        _tsystem(errorCommand.c_str());
        return 0;
    }

    if (maxFileSize > 100000000)
    {
        printf("max file is big!\n");
        tstring errorCommand = _T("UploadESSAndObj 0 0 0 0 0 0 0 0 5 ") + tstring(taskID);
        _tsystem(errorCommand.c_str());
        return 0;
    }

	TCHAR scriptText[5000];
    for (size_t i = 0; i < 200; i++)
    {
        mainwindow = nullptr;
        EnumWindows(FWindow1, 0);

        if (mainwindow != nullptr)
        {
            HWND scriptHWnd;
            scriptHWnd = FindWindowEx(mainwindow, NULL, _T("StatusPanel"), NULL);
            if (scriptHWnd != NULL)
            {
                scriptHWnd = FindWindowEx(scriptHWnd, NULL, _T("#32770"), NULL);
                if (scriptHWnd != NULL)
                {
                    scriptHWnd = FindWindowEx(scriptHWnd, NULL, _T("MXS_Scintilla"), NULL);
                    if (scriptHWnd != nullptr)
                    {
						//for (size_t i = 0; i < 60; i++)
						{
							//SendMessage(scriptHWnd, WM_GETTEXT, 255, (LPARAM)scriptText);//向该窗口发送消息，获取该窗口的文字。
							//if (_tcscmp(scriptText, _T("\"MeshTool Finished Init\"")) == 0)
							{
								tstring runScript = _T("fileIn \"") + tstring(exePath) + _T("\\MeshTool.mse\"\n");
								std::replace(runScript.begin(), runScript.end(), _T('\\'), _T('/'));
								SendMessage(scriptHWnd, WM_SETTEXT, 0, (LPARAM)runScript.c_str());
								SendMessage(scriptHWnd, WM_CHAR, VK_RETURN, 0);

                                std::replace(maxFilename.begin(), maxFilename.end(), _T('\\'), _T('/'));
								tstring callMeshtool = _T("CallMeshTool \"") + maxFilename + _T("\" \"") + tstring(exePath) + _T("\" \"") + zipFilename + _T("\" ")  + optimizeLevel +  _T(" \"") + taskID + _T("\"\n");
								SendMessage(scriptHWnd, WM_SETTEXT, 0, (LPARAM)callMeshtool.c_str());
								SendMessage(scriptHWnd, WM_CHAR, VK_RETURN, 0);
							}
							//Sleep(500);
						}

                        scriptHWnd = nullptr;
                        break;
                    }
                }
            }

        }
        else
        {
            Sleep(5000);
            if (!IsProcessRunning(_T("3dsmax.exe")))
            {
                tstring maxExePath = _T("\"") + max2018Path + _T("3dsmax.exe\"");
                startup(maxExePath.c_str());
            }
        }
        Sleep(5000);
    }


    return 0;
}

