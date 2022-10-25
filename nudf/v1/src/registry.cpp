

#include <Windows.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\registry.hpp>
#include <nudf\convert.hpp>


using namespace nudf::win;


//
// CRegLocalMachine
//
CRegLocalMachine::CRegLocalMachine() : CRegKey()
{
    _key = HKEY_LOCAL_MACHINE;
}

CRegLocalMachine::~CRegLocalMachine()
{
}

bool CRegLocalMachine::Open(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    return false;
}

bool CRegLocalMachine::Create(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    return false;
}

bool CRegLocalMachine::CreateEx(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    UNREFERENCED_PARAMETER(volatile_key);
    return false;
}

void CRegLocalMachine::Close()
{
}

//
// CRegCurrentUser
//
CRegCurrentUser::CRegCurrentUser() : CRegKey()
{
    _key = HKEY_CURRENT_USER;
}

CRegCurrentUser::~CRegCurrentUser()
{
}

bool CRegCurrentUser::Open(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    return false;
}

bool CRegCurrentUser::Create(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    return false;
}

bool CRegCurrentUser::CreateEx(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    UNREFERENCED_PARAMETER(volatile_key);
    return false;
}

void CRegCurrentUser::Close()
{
    assert(FALSE);
}


//
// CRegUsers
//
CRegUsers::CRegUsers() : CRegKey()
{
    _key = HKEY_USERS;
}

CRegUsers::~CRegUsers()
{
}

bool CRegUsers::Open(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    return false;
}

bool CRegUsers::Create(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    return false;
}

bool CRegUsers::CreateEx(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    UNREFERENCED_PARAMETER(volatile_key);
    return false;
}

void CRegUsers::Close()
{
    assert(FALSE);
}


//
// CRegClassesRoot
//
CRegClassesRoot::CRegClassesRoot() : CRegKey()
{
    _key = HKEY_CLASSES_ROOT;
}

CRegClassesRoot::~CRegClassesRoot()
{
}

bool CRegClassesRoot::Open(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    return false;
}

bool CRegClassesRoot::Create(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    return false;
}

bool CRegClassesRoot::CreateEx(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key) throw()
{
    UNREFERENCED_PARAMETER(hParentKey);
    UNREFERENCED_PARAMETER(name);
    UNREFERENCED_PARAMETER(samDesired);
    UNREFERENCED_PARAMETER(volatile_key);
    return false;
}

void CRegClassesRoot::Close()
{
    assert(FALSE);
}


//
// CRegKey
//
CRegKey::CRegKey() : _key(NULL), _created(false)
{
}

CRegKey::~CRegKey()
{
    Close();
}

bool CRegKey::Open(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    return (0 == ::RegOpenKeyExW(hParentKey, name, 0, samDesired, &_key));
}

bool CRegKey::Create(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired) throw()
{
    return CreateEx(hParentKey, name, samDesired, FALSE);
}

bool CRegKey::CreateEx(_In_ HKEY hParentKey, _In_ LPCWSTR name, _In_ REGSAM samDesired, _In_ BOOL volatile_key) throw()
{
    DWORD dwDisposition = 0;
    LONG lRet = ::RegCreateKeyExW(hParentKey, name, 0, NULL, volatile_key? REG_OPTION_NON_VOLATILE : REG_OPTION_VOLATILE, samDesired, NULL, &_key, &dwDisposition);
    if(0 != lRet) {
        return false;
    }
    _created = (REG_CREATED_NEW_KEY == dwDisposition) ? true : false;
    return true;
}

void CRegKey::Close() throw()
{
    if(NULL != _key && HKEY_USERS != _key && HKEY_CLASSES_ROOT != _key && HKEY_CURRENT_CONFIG != _key && HKEY_CURRENT_USER != _key && HKEY_LOCAL_MACHINE != _key) {
        RegCloseKey(_key);
        _key = NULL;
    }
    _created = false;
}

bool CRegKey::GetValue(_In_opt_ LPCWSTR name, _Out_ unsigned long* v) throw()
{
    DWORD type = 0;
    DWORD data = 0;
    std::vector<unsigned char> buf;

    if(!GetValueEx(name, &type, buf)) {
        return false;
    }

    if(buf.size() < sizeof(ULONG)) {
        SetLastError(ERROR_INVALID_DATA);
        return false;
    }

    switch(type)
    {
    case REG_DWORD_BIG_ENDIAN:
        nudf::util::convert::SwapBytes(&buf[0], sizeof(ULONG));
    case REG_QWORD:
    case REG_DWORD: // REG_DWORD_LITTLE_ENDIAN
        *v = *((unsigned long*)(&buf[0]));
        break;
    default:
        SetLastError(ERROR_INVALID_DATA);
        return false;
    }

    return true;
}

bool CRegKey::GetValue(_In_opt_ LPCWSTR name, _Out_ unsigned __int64* v) throw()
{
    DWORD type = 0;
    std::vector<unsigned char> buf;

    if(!GetValueEx(name, &type, buf)) {
        return false;
    }

    if(buf.size() < sizeof(ULONG)) {
        SetLastError(ERROR_INVALID_DATA);
        return false;
    }

    switch(type)
    {
    case REG_DWORD_BIG_ENDIAN:
        nudf::util::convert::SwapBytes(&buf[0], sizeof(ULONG));
    case REG_DWORD: // REG_DWORD_LITTLE_ENDIAN
        *v = *((unsigned long*)(&buf[0]));
        break;
    case REG_QWORD:
        *v = *((unsigned __int64*)(&buf[0]));
        break;
    default:
        SetLastError(ERROR_INVALID_DATA);
        return false;
    }

    return true;
}

bool CRegKey::GetValue(_In_opt_ LPCWSTR name, _Out_ std::wstring& v, _Out_opt_ bool* expandable) throw()
{
    DWORD type = 0;
    std::vector<unsigned char> buf;

    if(NULL != expandable) {
        *expandable = false;
    }

    if(!GetValueEx(name, &type, buf)) {
        return false;
    }

    if(REG_SZ != type && REG_EXPAND_SZ != type) {
        SetLastError(ERROR_INVALID_DATA);
        return false;
    }

    if(NULL != expandable) {
        *expandable = (REG_EXPAND_SZ == type) ? true : false;
    }

    if(!buf.empty()) {
        v = (const wchar_t*)(&buf[0]);
    }
    return true;
}

bool CRegKey::GetValue(_In_opt_ LPCWSTR name, _Out_ std::vector<unsigned char>& v) throw()
{
    DWORD dwType = 0;
    return GetValueEx(name, &dwType, v);
}

bool CRegKey::GetValue(_In_ LPCWSTR name, _Out_ std::vector<std::wstring>& v) throw()
{
    DWORD dwType = 0;
    std::vector<unsigned char> buf;
    const wchar_t* s = NULL;

    if(!GetValueEx(name, &dwType, buf)) {
        return false;
    }

    if(REG_MULTI_SZ != dwType) {
        SetLastError(ERROR_INVALID_DATA);
        return false;
    }

    s = (const wchar_t*)(&buf[0]);
    while(L'\0' != s[0]) {
        std::wstring ws(s);
        v.push_back(ws);
        s += (ws.length() + 1);
    }
    return true;
}

bool CRegKey::GetValueEx(_In_opt_ LPCWSTR name, _Out_ ULONG* type, _Out_ std::vector<unsigned char>& v) throw()
{
    DWORD dwSize = 0;
    LONG lRet = 0;

    *type = REG_NONE;
    v.clear();

    if(NULL == _key) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

    lRet = RegQueryValueExW(_key, name, NULL, type, NULL, &dwSize);
    if(0 != lRet) {
        SetLastError(lRet);
        return false;
    }

    if(0 == dwSize) {
        return true;
    }

    v.resize(dwSize, 0);
    lRet = RegQueryValueExW(_key, name, NULL, type, &v[0], &dwSize);
    if(0 != lRet) {
        v.clear();
        SetLastError(lRet);
        return false;
    }

    return true;
}

bool CRegKey::SetValue(_In_opt_ LPCWSTR name, _In_ unsigned long v) throw()
{
    return SetValueEx(name, REG_DWORD, (const BYTE*)&v, (ULONG)sizeof(unsigned long));
}

bool CRegKey::SetValue(_In_opt_ LPCWSTR name, _In_ unsigned __int64 v) throw()
{
    return SetValueEx(name, REG_QWORD, (const BYTE*)&v, (ULONG)sizeof(unsigned __int64));
}

bool CRegKey::SetValue(_In_opt_ LPCWSTR name, _In_ const std::wstring& v, _In_ bool expandable) throw()
{
    return SetValueEx(name, expandable?REG_EXPAND_SZ:REG_SZ, (const BYTE*)v.c_str(), (ULONG)((v.length()+1)*sizeof(wchar_t)));
}

bool CRegKey::SetValue(_In_opt_ LPCWSTR name, _In_ const std::vector<unsigned char>& v) throw()
{
    return SetValueEx(name, REG_BINARY, ((0 == v.size()) ? NULL : (&v[0])), (ULONG)v.size());
}

bool CRegKey::SetValue(_In_opt_ LPCWSTR name, _In_ const std::vector<std::wstring>& v) throw()
{
    ULONG size = 0;
    std::vector<unsigned char> buf;
    unsigned char* p = NULL;

    for(std::vector<std::wstring>::const_iterator it=v.begin(); it != v.end(); ++it) {
        size += (ULONG)(((*it).length() + 1) * sizeof(wchar_t));
    }
    size += sizeof(wchar_t);

    buf.resize(size, 0);
    p = &buf[0];
    for(std::vector<std::wstring>::const_iterator it=v.begin(); it != v.end(); ++it) {
        ULONG strsize = (ULONG)(((*it).length() + 1) * sizeof(wchar_t));
        memcpy(p, (*it).c_str(), strsize);
        p += strsize;
    }

    return SetValueEx(name, REG_MULTI_SZ, &buf[0], (ULONG)buf.size());
}

bool CRegKey::SetValueEx(_In_opt_ LPCWSTR name, _In_ ULONG type, _In_ const BYTE* data, _In_ ULONG size) throw()
{    
    if(NULL == _key) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }

    LONG lRet = RegSetValueExW(_key, name, 0, type, data, size);
    if(0 != lRet) {
        SetLastError(lRet);
        return false;
    }

    return true;
}

bool CRegKey::ValueExists(_In_opt_ LPCWSTR name, _In_opt_ ULONG* type)
{
    DWORD dwSize = 0;

    if(NULL == _key) {
        throw WIN32ERROR2(ERROR_INVALID_HANDLE);
    }
    
    LONG lRet = RegQueryValueExW(_key, name, NULL, type, NULL, &dwSize);
    if(0 != lRet) {
        if(ERROR_FILE_NOT_FOUND == lRet) {
            return false;
        }
        throw WIN32ERROR2(lRet);
    }
    return true;
}

bool CRegKey::SubKeyExists(_In_ LPCWSTR name)
{
    HKEY hSubKey = NULL;
    LONG lRet = ::RegOpenKeyExW(_key, name, 0, KEY_READ, &hSubKey);
    if(0 != lRet) {
        if(ERROR_FILE_NOT_FOUND == lRet) {
            return false;
        }
        throw WIN32ERROR2(lRet);
    }

    RegCloseKey(hSubKey);
    return true;
}

bool CRegKey::DeleteValue(_In_opt_ LPCWSTR name) throw()
{
    if(NULL == _key) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }
    LONG lRet = ::RegDeleteValueW(_key, name);
    if (0 != lRet) {
        SetLastError(lRet);
        return false;
    }
    return true;
}

bool CRegKey::DeleteSubKey(_In_ LPCWSTR name) throw()
{
    if (NULL == _key) {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }    
    LONG lRet = ::RegDeleteKeyW(_key, name);
    if (0 != lRet) {
        SetLastError(lRet);
        return false;
    }
    return true;
}
