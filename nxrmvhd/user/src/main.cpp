

#include <Windows.h>

#include <string>
#include <boost\algorithm\string.hpp>

#include "vhdmgr.hpp"



void help()
{
    printf("Usage: nxrmvhdmgr.exe <create|load|remove|list> [options]\n");
    printf("CREATE - create new virtual disk\n");
    printf("    nxrmvhdmgr.exe create <File>\n");
    printf("LOAD - load existing virtual disk\n");
    printf("    nxrmvhdmgr.exe load <File> [Drive]\n");
    printf("    For example: nxrmvhdmgr.exe load C:\\Users\\Vhd\\test.nsd\n");
    printf("    For example: nxrmvhdmgr.exe load C:\\Users\\Vhd\\testK.nsd K:\n");
    printf("REMOVE\n");
    printf("    nxrmvhdmgr.exe remove [DiskId]\n");
    printf("    For example: nxrmvhdmgr.exe remove 6\n");
    printf("LIST\n");
    printf("    nxrmvhdmgr.exe list\n");
}

void vhddump(const CVhdInfo& vhdi)
{
    printf("  - File: %S\n", vhdi.GetFile().c_str());
    printf("  - DiskId: %d\n", vhdi.GetDiskId());
    printf("  - NtName: %S\n", vhdi.GetNtName().c_str());
    printf("  - Removable: %s\n", vhdi.IsRemovable() ? "true" : "false");
    if(vhdi.IsVisible()) {
        printf("  - Visible: true\n");
        printf("  - DosName: %S\n", vhdi.GetDosName().c_str());
        printf("  - DriveLetter: %c\n", (char)vhdi.GetDriveLetter());
    }
    else {
        printf("  - Visible: false\n");
        printf("  - DosName: \n");
        printf("  - DriveLetter: \n");
    }
}

std::wstring get_fullpath(const wchar_t* path)
{
    std::wstring file(path ? path : L"");

    if (!file.empty()) {
        if (NULL == wcschr(path, L'\\')) {
            WCHAR wzCurrentDir[MAX_PATH] = { 0 };
            GetCurrentDirectoryW(MAX_PATH, wzCurrentDir);
            file = wzCurrentDir;
            if (!boost::algorithm::ends_with(file, L"\\")) {
                file += L"\\";
            }
            file += path;
        }
    }
    return std::move(file);
}

int wmain(int argc, wchar_t** argv)
{
    int nRet = 0;

    if(argc < 2) {
        help();
        return 0;
    }

#ifdef _DEBUG
    system("pause");
#endif

    if(0 == _wcsicmp(argv[1], L"create")) {

        if(argc < 3) {
            printf("ERROR: Invalid parameters\n");
            return ERROR_INVALID_PARAMETER;
        }

        std::wstring file(get_fullpath(argv[2]));
        CVhdMgr      mgr;

        if(!mgr.CreateVhd(file)) {
            return GetLastError();
        }
    }
    else if(0 == _wcsicmp(argv[1], L"load")) {

        if(argc < 3) {
            printf("ERROR: Invalid parameters\n");
            return ERROR_INVALID_PARAMETER;
        }

        std::wstring file(get_fullpath(argv[2]));
        WCHAR        drive = 0;

        if(argc > 3 && ((argv[3][0] >= L'A' && argv[3][0] <= L'Z') || (argv[3][0] >= L'a' && argv[3][0] <= L'z')) && argv[3][1] == L':') {
            // The drive letter has been specified
            drive = argv[3][0];
        }

        CVhdMgr             mgr;
        CVhdInfo            vhdi;

        if(!mgr.Connect(FALSE)) {
            nRet = GetLastError();
            printf("ERROR: Fail to connect to NextLabs VHD Service (%d)\n", nRet);
            return nRet;
        }

        if(!mgr.LoadVhd(file, drive, vhdi)) {
            nRet = GetLastError();
            printf("ERROR: Fail to load vhd file (%d)\n", nRet);
            mgr.Disconnect();
            return nRet;
        }
        mgr.Disconnect();

        // Succeed
        printf("Vhd has been loaded successfully:\n");
        vhddump(vhdi);
    }
    else if(0 == _wcsicmp(argv[1], L"remove")) {

        if(argc != 3) {
            printf("ERROR: Invalid parameters\n");
            return ERROR_INVALID_PARAMETER;
        }

        int disk_id = _wtoi(argv[2]);
        if(disk_id < 0) {
            printf("ERROR: Invalid parameters\n");
            return ERROR_INVALID_PARAMETER;
        }

        CVhdMgr             mgr;

        if(!mgr.Connect(FALSE)) {
            nRet = GetLastError();
            printf("ERROR: Fail to connect to NextLabs VHD Service (%d)\n", nRet);
            return nRet;
        }

        if(!mgr.UnloadVhd(disk_id)) {
            nRet = GetLastError();
            printf("ERROR: Fail to unload vhd file (%d)\n", nRet);
            mgr.Disconnect();
            return nRet;
        }

        mgr.Disconnect();
    }
    else if(0 == _wcsicmp(argv[1], L"list")) {

        CVhdMgr             mgr;
        std::vector<CVhdInfo> vhdis;

        if(!mgr.Connect(TRUE)) {
            nRet = GetLastError();
            printf("ERROR: Fail to connect to NextLabs VHD Service (%d)\n", nRet);
            return nRet;
        }

        if(!mgr.QueryAllVhds(vhdis)) {
            nRet = GetLastError();
            printf("ERROR: Fail to query active virtual disks (%d)\n", nRet);
            mgr.Disconnect();
            return nRet;
        }
        mgr.Disconnect();

        if(vhdis.empty()) {
            printf("There is no VHD has been loaded yet\n");
            return 0;
        }

        printf("There %s %d VHD%s been loaded\n", (vhdis.size()>1)?"are":"is", (int)vhdis.size(), (vhdis.size()>1)?"s have":" has");
        int i = 0;
        for(std::vector<CVhdInfo>::const_iterator it=vhdis.begin(); it!=vhdis.end(); ++it) {
            printf("\nVHD %04d\n", i++);
            vhddump(*it);
        }

        return 0;
    }
    else if(0 == _wcsicmp(argv[1], L"help") || 0 == _wcsicmp(argv[1], L"/h") || 0 == _wcsicmp(argv[1], L"--help") || 0 == _wcsicmp(argv[1], L"-h")) {
        help();
        return 0;
    }
    else {
        help();
        return ERROR_INVALID_PARAMETER;
    }

    return nRet;
}