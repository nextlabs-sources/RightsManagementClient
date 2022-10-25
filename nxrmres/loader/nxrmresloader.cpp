

#include <Windows.h>
#include <iostream>

#include <nudf\nxrmres.h>
#include <nudf\resutil.hpp>


HMODULE LoadResDll();

void main()
{
    HMODULE hResDll = NULL;

    hResDll = LoadResDll();
    if(NULL == hResDll) {
        printf("Fail to load resource DLL\n");
        return;
    }
    std::wstring wsMsgEnUS;
    std::wstring wsMsgZhCN;
    std::wstring wsMsgZhTW;
    std::wstring wsMsg;
    
    wsMsgEnUS = nudf::util::res::LoadMessage(hResDll, IDS_NXRM_STARTED, 0x0409);
    wsMsgZhCN = nudf::util::res::LoadMessage(hResDll, IDS_NXRM_STARTED, 0x0804);
    wsMsgZhTW = nudf::util::res::LoadMessage(hResDll, IDS_NXRM_STARTED, 0x0404);

    wsMsg += L"Message: IDS_NXRM_STARTED\n";
    wsMsg += L"  English:      " + wsMsgEnUS + L"\n";
    wsMsg += L"  简体中文: " + wsMsgZhCN + L"\n";
    wsMsg += L"  繁體中文: " + wsMsgZhTW + L"\n";

    MessageBoxW(NULL, wsMsg.c_str(), L"MUI Test", MB_OK);
}

HMODULE LoadResDll()
{
    WCHAR wzPath[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, wzPath, MAX_PATH);
    WCHAR* pos = wcsrchr(wzPath, L'\\');
    *(pos+1) = L'\0';
    wcscat_s(wzPath, L"nxrmres.dll");
    return ::LoadLibraryW(wzPath);
}
