

#include <Windows.h>

#include <assert.h>
#include <tlhelp32.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <boost\algorithm\string.hpp>

#include <nudf\exception.hpp>
#include <nudf\nxrmuihlp.hpp>
#include <nudf\nxlutil.hpp>
#include <nudf\pe.hpp>

#include "nxrmdrvman.h"

#include "servctrl.hpp"
#include "corehlp.hpp"

static int Protect(const std::wstring& file, bool slient, const std::vector<std::pair<std::wstring,std::wstring>>& tags);
static int GetClassifyData(CCoreObject& co, std::wstring& xmlfile, std::wstring& group);
static void PairToBuffer(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& pairs, _Out_ std::vector<WCHAR>& buf);
static BOOL CallerValidate();
static int ReadTag(const std::wstring& file, std::vector<std::pair<std::wstring, std::wstring>>& tags);

class CProcessInfo
{
public:
    CProcessInfo() : _pid(0xFFFFFFFF),_mid(0),_nthreads(0),_parent_pid(0xFFFFFFFF),_pribase(0),_flags(0)
    {
    }

    CProcessInfo(const PROCESSENTRY32W* pe32) : _pid(0xFFFFFFFF),_mid(0),_nthreads(0),_parent_pid(0xFFFFFFFF),_pribase(0),_flags(0)
    {
        if(NULL != pe32) {
            _pid = pe32->th32ProcessID;
            _mid = pe32->th32ModuleID;
            _nthreads = pe32->cntThreads;
            _parent_pid = pe32->th32ParentProcessID;
            _pribase = pe32->pcPriClassBase;
            _flags = pe32->dwFlags;
            _exe = pe32->szExeFile;
        }
    }

    virtual ~CProcessInfo()
    {
    }

    CProcessInfo& operator = (const CProcessInfo& info)
    {
        if(this != &info) {
            _pid = info.GetProcessId();
            _mid = info.GetModuleId();
            _nthreads = info.GetThreadsCount();
            _parent_pid = info.GetParentProcessId();
            _pribase = info.GetPriorityClassBase();
            _flags = info.GetFlags();
            _exe = info.GetExeName();
        }
        return *this;
    }

    inline DWORD GetProcessId() const throw() {return _pid;}
    inline DWORD GetModuleId() const throw() {return _mid;}
    inline DWORD GetParentProcessId() const throw() {return _parent_pid;}
    inline DWORD GetThreadsCount() const throw() {return _nthreads;}
    inline LONG GetPriorityClassBase() const throw() {return _pribase;}
    inline DWORD GetFlags() const throw() {return _flags;}
    inline const std::wstring& GetExeName() const throw() {return _exe;}

private:
    DWORD           _pid;           // this process
    DWORD           _mid;           // associated exe
    DWORD           _nthreads;
    DWORD           _parent_pid;    // this process's parent process
    LONG            _pribase;       // Base priority of process's threads
    DWORD           _flags;
    std::wstring    _exe;
};


int WINAPI WinMain(_In_ HINSTANCE hInstance,
                   _In_ HINSTANCE hPrevInstance,
                   _In_ LPSTR     lpCmdLine,
                   _In_ int       nCmdShow)
{
    LPWSTR* wzArgs = NULL;
    int     nArgs  = 0;
    int     nRet = 0;
    bool    bDebug = false;
    
    
    wzArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if(nArgs < 2) {
        return FALSE;
    }

    for(int i=0; i<nArgs; i++) {
        if(0 == _wcsicmp(wzArgs[i], L"/dbg")) {
            MessageBoxW(NULL, L"Debug Breakpoint", L"NextLabs", MB_OK);
            break;
        }
    }

    // Validate Caller
#ifdef _VALIDATE_CALLER
    CallerValidate();
#endif

    if(0 == _wcsicmp(wzArgs[1], L"protect")) {

        std::wstring file;
        std::vector<std::pair<std::wstring,std::wstring>> tags;
        bool slient = false;
        int  i = 0;

        if(nArgs < 3) {
            return ERROR_INVALID_PARAMETER;
        }

        // target file
        file = wzArgs[2];

        // options
        i = 3;
        while(i < nArgs) {

            if(0 == _wcsicmp(wzArgs[i], L"/s")) {
                slient = true;
            }
            else if(0 == _wcsicmp(wzArgs[i], L"/t")) {
                if(++i >= nArgs) {
                    return ERROR_INVALID_PARAMETER;
                }

                std::wstring wsPair = wzArgs[i];
                std::wstring::size_type pos = wsPair.find(L"=");
                if(std::wstring::npos == pos) {
                    return ERROR_INVALID_PARAMETER;
                }
                tags.push_back(std::pair<std::wstring,std::wstring>(wsPair.substr(0, pos), wsPair.substr(pos+1)));
            }
            else {
                ; // Nothing
            }

            ++i;
        }

        // Good, protect this file
        nRet = Protect(file, slient, tags);
    }
    else if(0 == _wcsicmp(wzArgs[1], L"unprotect")) {
        return ERROR_INVALID_FUNCTION;
    }
    else if (0 == _wcsicmp(wzArgs[1], L"export")) {
        if (nArgs < 3) {
            return ERROR_INVALID_FUNCTION;
        }

        if (0 == _wcsicmp(wzArgs[2], L"activitylog")) {
            std::wstring logfile(wzArgs[3]);
            if (!NXSERV::export_activity_log(logfile)) {
                printf("Fail to export activity log");
            }
            else {
                printf("Succeeded");
            }
        }
    }
    else if (0 == _wcsicmp(wzArgs[1], L"readtag")) {
        if (nArgs < 3) {
            return ERROR_INVALID_FUNCTION;
        }

        std::vector<std::pair<std::wstring, std::wstring>> tags;

        nRet = ReadTag(wzArgs[2], tags);
        if (0 == nRet) {
            std::for_each(tags.begin(), tags.end(), [](const std::pair<std::wstring, std::wstring>& it) {
                std::wcout << it.first << L" = " << it.second << std::endl;
            });
        }
    }
    else {
        return ERROR_INVALID_PARAMETER;
    }

    return nRet;
}

static int Protect(const std::wstring& file, bool slient, const std::vector<std::pair<std::wstring,std::wstring>>& tags)
{
    int         nRet = 0;
    HRESULT     hr = S_OK;
    std::vector<std::pair<std::wstring,std::wstring>> finaltags;
    std::vector<WCHAR> tagsbuf;

    // Initialize Tags
    finaltags = tags;

    // If it is not slient
    // Pop up classify UI
    if(!slient) {

        CCoreObject             co;
        nudf::util::CRmuObject  uo;
        std::wstring            xmlfile;
        std::wstring            group;
        WCHAR                   modpath[MAX_PATH+1] = {0};

        nRet = co.Connect();
        if(0 != nRet) {
            return nRet;
        }

        nRet = GetClassifyData(co, xmlfile, group);
        if(0 != nRet) {
            return nRet;
        }

        GetModuleFileNameW(NULL, modpath, MAX_PATH);
        WCHAR* pos = wcsrchr(modpath, L'\\');
        if(NULL == pos) {
            return ERROR_MOD_NOT_FOUND;
        }
        *(pos+1) = L'\0';
        wcsncat_s(modpath, MAX_PATH, L"nxrmcmui.dll", _TRUNCATE);
        uo.Initialize(modpath);

        if(!uo.RmuInitialize()) {
            return GetLastError();
        }

        hr = uo.RmuShowClassifyDialogEx(NULL, file.c_str(), xmlfile.c_str(), group.c_str(), GetUserDefaultLangID(), FALSE, finaltags);
        if(S_OK != hr) {
            uo.RmuDeinitialize();
            return hr;
        }
    }

    if(!finaltags.empty()) {
        PairToBuffer(finaltags, tagsbuf);
    }
    hr = nudf::util::nxl::NxrmEncryptFileEx(file.c_str(), NXL_SECTION_TAGS, tagsbuf.empty() ? NULL : (UCHAR*)(&tagsbuf[0]), (USHORT)(tagsbuf.size() * sizeof(WCHAR)));

    return (int)hr;
}

static int GetClassifyData(CCoreObject& co, std::wstring& xmlfile, std::wstring& group)
{
	GET_CLASSIFY_UI_REQUEST		Request = {0};
	GET_CLASSIFY_UI_RESPONSE	Resp = {0};
	NXCONTEXT                   Ctx = NULL;
	ULONG                       RespSize = sizeof(Resp);
	LANGID	                    LangId = {0};

    if(!co.IsTransportEnabled()) {
        SetLastError(ERROR_DEVICE_NOT_AVAILABLE);
        return ERROR_DEVICE_NOT_AVAILABLE;
    }


    LangId = GetUserDefaultLangID();
    Request.LanguageId	= LangId;
    Request.ProcessId	= GetCurrentProcessId();
    Request.ThreadId	= GetCurrentThreadId();

    if(!ProcessIdToSessionId(Request.ProcessId, &Request.SessionId)) {
        return GetLastError();
    }

    if (!GetTempPathW(sizeof(Request.TempPath)/sizeof(WCHAR), Request.TempPath)) {
        return GetLastError();
    }

    if (!GetLongPathNameW(Request.TempPath, Request.TempPath, sizeof(Request.TempPath)/sizeof(WCHAR))) {
        return GetLastError();
    }

    Ctx = co.SubmitRequest(NXRMDRV_MSG_TYPE_GET_CLASSIFYUI, &Request, sizeof(Request));
    if (NULL == Ctx) {
        SetLastError(ERROR_DEVICE_NOT_AVAILABLE);
        return ERROR_DEVICE_NOT_AVAILABLE;
    }

    if (!co.WaitForResponse(Ctx, &Resp, sizeof(Resp), &RespSize)) {
        Ctx = NULL;
        SetLastError(ERROR_DEVICE_NOT_AVAILABLE);
        return ERROR_DEVICE_NOT_AVAILABLE;
    }

    xmlfile = Resp.ClassifyUIFileName;
    group   = Resp.GroupName;
    return 0L;
}

static void PairToBuffer(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& pairs, _Out_ std::vector<WCHAR>& buf)
{
	buf.clear();
	for(int i=0; i<(int)pairs.size(); i++) {
		if(pairs[i].first.empty() || pairs[i].second.empty()) {
			continue;
		}
		std::wstring ws(pairs[i].first);
		ws += L"=";
		ws += pairs[i].second;
		for(int j=0; j<(int)ws.length(); j++) {
			buf.push_back(ws.c_str()[j]);
		}
		buf.push_back(L'\0');
	}
	if(!buf.empty()) {
		buf.push_back(L'\0');
	}
}

static BOOL GetProcessList(std::map<DWORD,CProcessInfo>& pl)
{
    HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
    PROCESSENTRY32W pe32 = {0};

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessSnap == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if( !Process32FirstW(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);          // clean the snapshot object
        return FALSE;
    }

    do {
        CProcessInfo pi(&pe32);
        pl[pi.GetProcessId()] = pi;
    } while(Process32NextW(hProcessSnap, &pe32));
    
    CloseHandle(hProcessSnap);
    return TRUE;
}

static BOOL GetProcessImagePath(DWORD dwPid, std::wstring& path)
{
    HANDLE hProcess = NULL;
    WCHAR wzPath[MAX_PATH] = {0};
    DWORD dwSize = MAX_PATH;

    hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);
    if(NULL == hProcess) {
        return FALSE;
    }

    memset(wzPath, 0, sizeof(wzPath));
    if(!QueryFullProcessImageNameW(hProcess, 0, wzPath, &dwSize)) {
        CloseHandle(hProcess);
        return FALSE;
    }

    path = wzPath;
    return TRUE;
}

static BOOL CallerValidate()
{
    std::map<DWORD,CProcessInfo> pl;
    DWORD   parent_pid = 0xFFFFFFFF;
    std::wstring    publisher;

    if(!GetProcessList(pl)) {
        return FALSE;
    }

    // Find parent
    const CProcessInfo& pi = pl[GetCurrentProcessId()];
    if(GetCurrentProcessId() != pi.GetProcessId() || 0xFFFFFFFF == pi.GetParentProcessId()) {
        return FALSE;
    }

    const CProcessInfo& parent = pl[pi.GetParentProcessId()];
    if(pi.GetParentProcessId() != parent.GetProcessId() || parent.GetExeName().empty()) {
        return FALSE;
    }


    // Check app name
    if(0 != _wcsicmp(parent.GetExeName().c_str(), L"outlook.exe")) {
#ifdef _DEBUG
        if(0 != _wcsicmp(parent.GetExeName().c_str(), L"cmd.exe")) {
#endif
        return FALSE;
#ifdef _DEBUG
        }
#endif
    }

    // Get Signature
    try {
        nudf::win::CPEFile pe;
        std::wstring path;

        if(GetProcessImagePath(parent.GetProcessId(), path)) {
            pe.Load(path.c_str());
            publisher = pe.GetCert().GetSubject();
        }
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
    
#ifndef _DEBUG
    if(0 != _wcsicmp(publisher.c_str(), L"Microsoft Corporation")) {
        return FALSE;
    }
#endif
    
    return TRUE;
}

static int ReadTag(const std::wstring& file, std::vector<std::pair<std::wstring, std::wstring>>& tags)
{
    std::wstring target(file);
    HANDLE h = INVALID_HANDLE_VALUE;

    if (!boost::algorithm::iends_with(target, L".nxl")) {
        target += L".nxl";
    }

    do {

        PCNXL_HEADER header = NULL;
        std::vector<UCHAR> header_data;
        DWORD dwBytesRead = 0;

        h = ::CreateFileW(target.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == h) {
            break;
        }

        header_data.resize(sizeof(NXL_HEADER), 0);
        if (!ReadFile(h, header_data.data(), sizeof(NXL_HEADER), &dwBytesRead, NULL)) {
            break;
        }
        if (dwBytesRead < (DWORD)sizeof(NXL_HEADER)) {
            SetLastError(ERROR_INVALID_DATA);
            break;
        }
        header = (PCNXL_HEADER)header_data.data();

        if(header->Signature.Code.LowPart != NXL_SIGNATURE_LOW || header->Signature.Code.HighPart != NXL_SIGNATURE_HIGH) {
            SetLastError(ERROR_INVALID_DATA);
            break;
        }
        if (header->Basic.Version != NXL_VERSION_10) {
            SetLastError(ERROR_INVALID_DATA);
            break;
        }

        ULONG TagSectionOffset = sizeof(NXL_HEADER);
        ULONG TagSectionSize = 0;

        for (int i = 0; i < (int)header->Sections.Count; i++) {
            if (0 != _stricmp(header->Sections.Sections[i].Name, NXL_SECTION_TAGS)) {
                TagSectionOffset += header->Sections.Sections[i].Size;
                continue;
            }
            TagSectionSize = header->Sections.Sections[i].Size;
            break;
        }

        if (0 == TagSectionSize) {
            // Not found
            SetLastError(ERROR_NOT_FOUND);
            break;
        }

        if (INVALID_SET_FILE_POINTER == SetFilePointer(h, TagSectionOffset, NULL, FILE_BEGIN)) {
            break;
        }

        std::vector<WCHAR> tag_data;
        tag_data.resize(TagSectionSize / 2 + 2, 0);
        if (!ReadFile(h, tag_data.data(), TagSectionSize, &dwBytesRead, NULL)) {
            break;
        }
        if (dwBytesRead < TagSectionSize) {
            SetLastError(ERROR_INVALID_DATA);
            break;
        }

        const WCHAR* p = tag_data.data();
        while (*p) {
            std::wstring tag = p;
            p += (tag.length() + 1);
            std::wstring::size_type pos = tag.find(L'=');
            if (pos == std::wstring::npos) {
                continue;
            }
            std::wstring tag_name = tag.substr(0, pos);
            std::wstring tag_value = tag.substr(pos + 1);
            boost::algorithm::trim(tag_name);
            boost::algorithm::trim(tag_value);
            tags.push_back(std::pair<std::wstring, std::wstring>(tag_name, tag_value));
        }

        SetLastError(ERROR_SUCCESS);

    } while (FALSE);

    if (INVALID_HANDLE_VALUE != h) {
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
    }

    return GetLastError();
}