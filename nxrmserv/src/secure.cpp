

#include <Windows.h>

#define SECURITY_WIN32
#include <security.h>

#include <string>
#include <algorithm>
#include <sstream>

#include <nudf\web\json.hpp>
#include <nudf\string.hpp>

#include <rapidxml\rapidxml.hpp>

#include "secure.hpp"


using namespace NX;




secure_mem::secure_mem() : m_size(0)
{
}

secure_mem::secure_mem(const std::vector<unsigned char> &data) : m_size(0)
{
    encrypt(data);
}

secure_mem::~secure_mem()
{
    clear();
}

void secure_mem::encrypt(const std::vector<unsigned char>& data)
{
    m_size = data.size();

    // Buffer must be a multiple of CRYPTPROTECTMEMORY_BLOCK_SIZE
    const auto mod = m_size % CRYPTPROTECTMEMORY_BLOCK_SIZE;
    if (mod != 0) {
        m_buffer.resize(m_size + CRYPTPROTECTMEMORY_BLOCK_SIZE - mod, 0);
    }
    else {
        m_buffer.resize(m_size, 0);
    }
    // Copy data
    memcpy_s(m_buffer.data(), m_buffer.size(), &data[0], m_size);
    // Encrypt data
    if (!CryptProtectMemory(m_buffer.data(), static_cast<DWORD>(m_buffer.size()), CRYPTPROTECTMEMORY_SAME_PROCESS)) {
        throw std::exception("secure_mem: fail to encrypt memory");
    }
}

std::vector<unsigned char> secure_mem::decrypt() const
{
    std::vector<unsigned char> data = m_buffer;
    if (!data.empty()) {
        if (!CryptUnprotectMemory(data.data(), static_cast<DWORD>(m_buffer.size()), CRYPTPROTECTMEMORY_SAME_PROCESS)) {
            throw std::exception("secure_mem: fail to decrypt memory");
        }
        data.resize(m_size);
    }
    return std::move(data);
}

void secure_mem::clear() noexcept
{
    if (!m_buffer.empty()) {
        SecureZeroMemory(m_buffer.data(), m_buffer.size());
        m_buffer.clear();
        m_size = 0;
    }
}




winsid::winsid() : _sid(NULL)
{
}

winsid::winsid(const std::wstring& sid)
{
    from_string(sid);
}

winsid::winsid(PSID sid)
{
    (void)from_sid(sid);
}

winsid::~winsid()
{
    clear();
}

void winsid::clear()
{
    if (NULL != _sid) {
        LocalFree(_sid);
        _sid = NULL;
    }
}

winsid& winsid::operator = (const winsid& other) noexcept
{
    if (this != &other) {
        (void)from_sid(other);
    }
    return *this;
}

std::wstring winsid::to_string() const noexcept
{
    LPWSTR pwzSid = NULL;
    std::wstring wsSid;

    if (NULL == _sid) {
        return wsSid;
    }
    if (!ConvertSidToStringSidW(_sid, &pwzSid)) {
        return wsSid;
    }

    wsSid = pwzSid;
    LocalFree(pwzSid); pwzSid = NULL;
    return wsSid;
}

bool winsid::from_string(const std::wstring& sid) noexcept
{
    clear();
    if (!ConvertStringSidToSidW(sid.c_str(), &_sid)) {
        assert(_sid == NULL);
        return false;
    }
    return true;
}

bool winsid::from_sid(PSID sid) noexcept
{
    clear();
    if (IsValidSid(sid)) {
        return false;
    }
    unsigned long sid_length = GetLengthSid(sid);
    _sid = (PSID)::LocalAlloc(LPTR, sid_length);
    if (NULL == _sid) {
        return false;
    }
    if (!CopySid(sid_length, _sid, sid)) {
        LocalFree(_sid);
        _sid = NULL;
        return false;
    }
    return true;
}



security_attribute::security_attribute() : _pdacl(NULL)
{
    memset(&_sa, 0, sizeof(_sa));
    _sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    _sa.bInheritHandle = FALSE;
}

security_attribute::~security_attribute()
{
    clear();
}

void security_attribute::clear() noexcept
{
    if (_pdacl != NULL) {
        LocalFree(_pdacl); _pdacl = NULL;
    }
    if (NULL != _sa.lpSecurityDescriptor) {
        LocalFree(_sa.lpSecurityDescriptor);
        _sa.lpSecurityDescriptor = NULL;
    }

    std::for_each(_eas.begin(), _eas.end(), [&](EXPLICIT_ACCESS& ea) {
        if (NULL != ea.Trustee.ptstrName) {
            FreeSid((PSID)ea.Trustee.ptstrName);
            ea.Trustee.ptstrName = NULL;
        }
    });
    memset(&_sa, 0, sizeof(_sa));
}

bool security_attribute::generate() noexcept
{
    // Reset existing data
    if (NULL != _pdacl) {
        LocalFree(_pdacl);
        _pdacl = NULL;
    }
    if (NULL != _sa.lpSecurityDescriptor) {
        LocalFree(_sa.lpSecurityDescriptor);
        _sa.lpSecurityDescriptor = NULL;
    }

    // Initialize SA
    _sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    _sa.bInheritHandle = FALSE;
    _sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (NULL == _sa.lpSecurityDescriptor) {
        return false;
    }
    InitializeSecurityDescriptor(_sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);

    // Create ACL
    SetEntriesInAclW((unsigned long)_eas.size(), &_eas[0], NULL, &_pdacl);
    if (NULL == _pdacl) {
        assert(NULL != _sa.lpSecurityDescriptor);
        LocalFree(_sa.lpSecurityDescriptor);
        _sa.lpSecurityDescriptor = NULL;
        return false;
    }

    // Set ACL
    if (!SetSecurityDescriptorDacl(_sa.lpSecurityDescriptor, TRUE, _pdacl, FALSE)) {
        assert(NULL != _pdacl);
        assert(NULL != _sa.lpSecurityDescriptor);
        LocalFree(_pdacl);
        _pdacl = NULL;
        LocalFree(_sa.lpSecurityDescriptor);
        _sa.lpSecurityDescriptor = NULL;
        return false;
    }

    return true;
}

bool security_attribute::add_acl(PSID sid, unsigned long access_permissions, TRUSTEE_TYPE trustee_type, ACCESS_MODE access_mode/* = SET_ACCESS*/, unsigned long inheritance/* = NO_INHERITANCE*/) noexcept
{
    unsigned long sid_length = 0;
    PSID tmp_sid = NULL;

    sid_length = GetLengthSid(sid);
    tmp_sid = (PSID)::LocalAlloc(LPTR, sid_length);
    if (NULL == tmp_sid) {
        SetLastError(ERROR_OUTOFMEMORY);
        return false;
    }
    memset(tmp_sid, 0, sid_length);
    if (!CopySid(sid_length, tmp_sid, sid)) {
        LocalFree(tmp_sid);
        tmp_sid = NULL;
        return false;
    }

    inter_add_acl(tmp_sid, access_permissions, trustee_type, access_mode, inheritance);
    tmp_sid = NULL;
    return true;
}

bool security_attribute::add_acl(const std::wstring& sid, unsigned long access_permissions, TRUSTEE_TYPE trustee_type, ACCESS_MODE access_mode/* = SET_ACCESS*/, unsigned long inheritance/* = NO_INHERITANCE*/) noexcept
{
    PSID tmp_sid = NULL;

    if (!ConvertStringSidToSidW(sid.c_str(), &tmp_sid)) {
        return false;
    }

    inter_add_acl(tmp_sid, access_permissions, trustee_type, access_mode, inheritance);
    tmp_sid = NULL;
    return true;
}

bool security_attribute::add_acl_for_wellknown_group(SID_IDENTIFIER_AUTHORITY authority, unsigned long rid, unsigned long access_permissions, unsigned long inheritance/* = NO_INHERITANCE*/) noexcept
{
    PSID psid = NULL;
    if (AllocateAndInitializeSid(&authority, 1, rid, 0, 0, 0, 0, 0, 0, 0, &psid)) {
        inter_add_acl(psid, access_permissions, TRUSTEE_IS_WELL_KNOWN_GROUP, SET_ACCESS, inheritance);
        return true;
    }
    return false;
}

bool security_attribute::add_acl_for_wellknown_group2(SID_IDENTIFIER_AUTHORITY authority, unsigned long rid1, unsigned long rid2, unsigned long access_permissions, unsigned long inheritance/* = NO_INHERITANCE*/) noexcept
{
    PSID psid = NULL;
    if (AllocateAndInitializeSid(&authority, 2, rid1, rid2, 0, 0, 0, 0, 0, 0, &psid)) {
        inter_add_acl(psid, access_permissions, TRUSTEE_IS_WELL_KNOWN_GROUP, SET_ACCESS, inheritance);
        return true;
    }
    return false;
}

void security_attribute::inter_add_acl(PSID sid, unsigned long access_permissions, TRUSTEE_TYPE trustee_type, ACCESS_MODE access_mode, unsigned long inheritance) noexcept
{
    EXPLICIT_ACCESS ea;
    memset(&ea, 0, sizeof(ea));
    ea.grfAccessPermissions = access_permissions;
    ea.grfAccessMode = access_mode;
    ea.grfInheritance = inheritance;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = trustee_type;
    ea.Trustee.ptstrName = (LPWSTR)sid;  // transfer ownership
                                         // add this one to buffer
    _eas.push_back(ea);
}


sa_everyone::sa_everyone(unsigned long access_permissions) : security_attribute()
{
    PSID sid_everyone = NULL;
    static SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    if (AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &sid_everyone)) {
        inter_add_acl(sid_everyone, access_permissions, TRUSTEE_IS_WELL_KNOWN_GROUP, SET_ACCESS, NO_INHERITANCE);
        (void)generate();
    }
}