
#include <Windows.h>
#include <stdio.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\path.hpp>

using namespace nudf;
using namespace nudf::win;


CFilePath::CFilePath() : CPath()
{
}

CFilePath::CFilePath(_In_ LPCWSTR wzPath) : CPath()
{
    try {
        SetPath(wzPath);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

CFilePath::CFilePath(_In_ const std::wstring& wsPath) : CPath()
{
    try {
        SetPath(wsPath.c_str());
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
    }
}

CFilePath::~CFilePath()
{
}

VOID CFilePath::SetPath(_In_ LPCWSTR wzPath)
{
    if(NULL == wzPath || L'\0' == wzPath[0]) {
        CPath::SetPath(L"");
        return;
    }
    else if(nudf::win::IsDosPath(wzPath) || nudf::win::IsUncPath(wzPath)) {
        CPath::SetPath(wzPath);
    }
    else if(nudf::win::IsGlobalDosPath(wzPath)) {
        CPath::SetPath(&wzPath[4]);
    }
    else if(nudf::win::IsGlobalUncPath(wzPath)) {
        std::wstring wstr(L"\\");
        wstr += (&wzPath[7]);
    }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
        throw WIN32ERROR();
    }
}

CFilePath& CFilePath::operator = (_In_ LPCWSTR wzPath)
{
    SetPath(wzPath);
    return *this;
}

CFilePath& CFilePath::operator = (_In_ const std::wstring& wsPath)
{
    SetPath(wsPath.c_str());
    return *this;
}

CFilePath& CFilePath::operator = (_In_ const CPath& Path)
{
    SetPath(Path.GetPath().c_str());
    return *this;
}

CFilePath& CFilePath::operator = (_In_ const CFilePath& Path) throw()
{
    CPath::SetPath(Path.GetPath().c_str());
    return *this;
}

BOOL CFilePath::IsUncPath() const throw()
{
    return nudf::win::IsUncPath(GetPath().c_str());
}

BOOL CFilePath::IsDosPath() const throw()
{
    return nudf::win::IsDosPath(GetPath().c_str());
}

BOOL CFilePath::IsRoot() const throw()
{
    if(IsUncPath()) {
        const WCHAR* pos = wcschr((GetPath().c_str()+2), L'\\');
        if(pos == NULL) {
            return TRUE;
        }
        return (L'\0' == (*(pos+1)));
    }
    else if(IsDosPath()) {
        return (GetPath().length() == 3);
    }
    else {
        return FALSE;
    }
}

std::wstring CFilePath::GetParentDir() const throw()
{
    std::wstring::size_type pos = GetPath().find_last_of(L"\\");
    std::wstring wsParentDir;

    if(std::wstring::npos == pos) {
        wsParentDir = L"";
        return wsParentDir;
    }

    wsParentDir = GetPath().substr(0, pos+1);   // Include last L'\\'
    if(IsUncPath()) {
        const WCHAR* pos = wcschr((wsParentDir.c_str()+2), L'\\');
        assert(NULL != pos);
        if(L'\0' != (*(pos+1))) {
            // This is not root, remove last L'\\'
            wsParentDir = wsParentDir.substr(0, wsParentDir.length()-1);
        }
    }
    else {
        if(wsParentDir.length() > 3) {
            // This is not root, remove last L'\\'
            wsParentDir = wsParentDir.substr(0, wsParentDir.length()-1);
        }
    }

    return wsParentDir;
}

std::wstring CFilePath::GetFileName() const throw()
{
    const WCHAR* pos = wcsrchr(GetPath().c_str(), L'\\');
    return (NULL==pos) ? GetPath().c_str() : (pos+1);
}

INT CFilePath::CompareFileName(_In_ LPCWSTR FileName) const throw()
{
    const WCHAR* pos = wcsrchr(GetPath().c_str(), L'\\');
    assert(NULL != pos);
    if(NULL == pos) {
        return _wcsicmp(GetPath().c_str(), FileName);
    }
    else {
        pos++;
        return _wcsicmp(pos, FileName);
    }
}

//
//
//
CFileName::CFileName()
{
}

CFileName::CFileName(_In_ LPCWSTR wzFileName)
{
    SetFileName(wzFileName);
}

CFileName::~CFileName()
{
}

VOID CFileName::SetPath(_In_ LPCWSTR wzPath) throw()
{
    SetFileName(wzPath);
}

VOID CFileName::SetFileName(_In_ LPCWSTR wzFileName) throw()
{
    const WCHAR* pos = wcsrchr(wzFileName, L'\\');
    CPath::SetPath((NULL==pos) ? wzFileName : (pos+1));
}

const std::wstring& CFileName::GetFileName() const throw()
{
    return CPath::GetPath();
}

std::wstring CFileName::GetExtension() const throw()
{
    const WCHAR* pos = wcsrchr(GetFileName().c_str(), L'.');
    return (NULL==pos) ? L"" : (pos+1);
}

INT CFileName::CompareExtension(_In_ LPCWSTR Extension) const throw()
{
    const WCHAR* pos = wcsrchr(GetFileName().c_str(), L'.');
    if(NULL == pos) {
        return (NULL==Extension || L'\0'==Extension[0]);
    }
    else {
        return _wcsicmp(pos, Extension);
    }
}

//
//  class CModulePath
//
CModulePath::CModulePath() : CFilePath()
{
}

CModulePath::CModulePath(_In_opt_ HMODULE hModule) : CFilePath()
{
    SetModule(hModule);
}

VOID CModulePath::SetModule(_In_opt_ HMODULE hModule) throw()
{
    WCHAR wzPath[MAX_PATH+1] = {0};
    GetModuleFileNameW(hModule, wzPath, MAX_PATH);
    CFilePath::SetPath(wzPath);
}
