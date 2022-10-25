

#include <Windows.h>

#include <boost\crc.hpp>

#include <nudf\shared\nxlfmt.h>
#include <nudf\web\json.hpp>
#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\crypto.hpp>

#include "nxrmserv.h"
#include "nxlfile.hpp"


using namespace NX;
using namespace NX::NXL;





//
//  class nxl_key_id
//
nxl_key_id::nxl_key_id() : _algorithm(0)
{
}

nxl_key_id::nxl_key_id(unsigned long algorithm, const unsigned char* id, unsigned long size) : _algorithm(algorithm), _id(id, id + size)
{
}

nxl_key_id::nxl_key_id(unsigned long algorithm, const std::vector<unsigned char>& id) : _algorithm(algorithm), _id(id)
{
}

nxl_key_id::~nxl_key_id()
{
}

nxl_key_id& nxl_key_id::operator = (const  nxl_key_id& other)
{
    if (this != &other) {
        if (other.empty()) {
            clear();
        }
        else {
            _algorithm = other.algorithm();
            assert(NULL != other.id());
            _id = std::vector<unsigned char>(other.id(), other.id() + other.size());
        }
    }
    return *this;
}



//
//  class nxl_header
//
nxl_header::nxl_header() : 
    _version(0), _flags(0), _alignment(0), _content_offset(0), _algorithm(0), _cbc_size(0), _content_size(0), _allocation_size(0)
{
}

nxl_header::~nxl_header()
{
}

void nxl_header::clear() noexcept
{
    _message.clear();
    _thumbprint.clear();
    _version = 0;
    _flags = 0;
    _alignment = 0;
    _content_offset = 0;
    _algorithm = 0;
    _cbc_size = 0;
    _primary_key_id.clear();
    _primary_content_key.clear();
    _recovery_key_id.clear();
    _recovery_content_key.clear();
    _content_size = 0;
    _allocation_size = 0;
    _sections.clear();
    _sections_checksum.clear();
}

void nxl_header::load(const std::wstring& file)
{
    HANDLE h = INVALID_HANDLE_VALUE;

    h = ::CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == h) {
        throw std::exception("fail to open file");
    }

    try {
        load(h);
    }
    catch (std::exception e) {
        if (INVALID_HANDLE_VALUE != h) {
            CloseHandle(h);
            h = INVALID_HANDLE_VALUE;
        }
        throw e;
    }

    if (INVALID_HANDLE_VALUE != h) {
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
    }
}

static void validate_header(const NXL_HEADER* header)
{
    if (header->Signature.Code.HighPart != NXL_SIGNATURE_HIGH || header->Signature.Code.LowPart != NXL_SIGNATURE_LOW) {
        throw std::exception("bad signature");
    }

    if (header->Basic.Version != NXL_VERSION_10) {
        throw std::exception("bad version");
    }

    if (header->Basic.Alignment != NXL_PAGE_SIZE) {
        throw std::exception("bad page size");
    }

    if (header->Basic.PointerOfContent < NXL_MIN_SIZE) {
        throw std::exception("bad data offset");
    }

    if (header->Crypto.Algorithm != NXL_ALGORITHM_AES128 && header->Crypto.Algorithm != NXL_ALGORITHM_AES256) {
        throw std::exception("bad encrypt algorithm");
    }

    if (header->Crypto.CbcSize != NXL_CBC_SIZE) {
        throw std::exception("bad cbc size");
    }

    if (header->Crypto.PrimaryKey.KeKeyId.Algorithm != NXL_ALGORITHM_AES128 && header->Crypto.PrimaryKey.KeKeyId.Algorithm != NXL_ALGORITHM_AES256) {
        throw std::exception("bad key algorithm");
    }

    if (header->Crypto.PrimaryKey.KeKeyId.IdSize == 0) {
        throw std::exception("bad key id size");
    }

    if (header->Sections.Count < 3) {
        throw std::exception("two few sections");
    }
}

void nxl_header::load(HANDLE h)
{
    NXL_HEADER header = { 0 };
    unsigned long bytes_read = 0;

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h, 0, NULL, FILE_BEGIN)) {
        throw std::exception("fail to move to begining of file");
    }

    memset(&header, 0, sizeof(header));
    if (!::ReadFile(h, &header, sizeof(header), &bytes_read, NULL)) {
        throw std::exception("fail to read file header");
    }

    validate_header(&header);

    // copy header information
    _message        = header.Signature.Message;
    _thumbprint     = nudf::string::FromBytes<wchar_t>(header.Basic.Thumbprint, 16);
    _version        = header.Basic.Version;
    _flags          = header.Basic.Flags;
    _alignment      = header.Basic.Alignment;
    _content_offset = header.Basic.PointerOfContent;
    _algorithm      = header.Crypto.Algorithm;
    _cbc_size       = header.Crypto.CbcSize;

    // get key
    _primary_key_id = nxl_key_id(header.Crypto.PrimaryKey.KeKeyId.Algorithm, header.Crypto.PrimaryKey.KeKeyId.Id, header.Crypto.PrimaryKey.KeKeyId.IdSize);
    _primary_content_key = std::vector<unsigned char>(header.Crypto.PrimaryKey.CeKey, header.Crypto.PrimaryKey.CeKey + NX_CEK_MAX_LEN);
    if (header.Crypto.RecoveryKey.KeKeyId.Algorithm != 0 && header.Crypto.RecoveryKey.KeKeyId.IdSize != 0) {
        _recovery_key_id = nxl_key_id(header.Crypto.RecoveryKey.KeKeyId.Algorithm, header.Crypto.RecoveryKey.KeKeyId.Id, header.Crypto.RecoveryKey.KeKeyId.IdSize);
        _recovery_content_key = std::vector<unsigned char>(header.Crypto.RecoveryKey.CeKey, header.Crypto.RecoveryKey.CeKey + NX_CEK_MAX_LEN);
    }

    // get sections
    unsigned session_offset = NXL_SCNDATA_OFFSET;
    for (int i = 0; i < (int)header.Sections.Count; i++) {
        // section N
        std::string s(header.Sections.Sections[i].Name);
        if (s.empty()) {
            break;
        }
        _sections.push_back(nxl_section(NX::utility::conversions::utf8_to_utf16(s), session_offset, header.Sections.Sections[i].Size, header.Sections.Sections[i].Checksum));
        session_offset += header.Sections.Sections[i].Size;
    }
    _sections_checksum = std::vector<unsigned char>(header.Sections.Checksum, header.Sections.Checksum + 16);
}

void nxl_header::validate_sections(const std::vector<unsigned char>& key, HANDLE h /*optional*/)
{
    unsigned long real_checksum = calc_sections_checksum();
    unsigned long checksum_in_table = decrypt_sections_checksum(key);
    if (real_checksum != checksum_in_table) {
        throw std::exception("checksum does not match");
    }
    if (h != INVALID_HANDLE_VALUE && h != NULL) {
        for (auto it = _sections.cbegin(); it != _sections.cend(); ++it) {
            validate_section(*it, h);
        }
    }
}

void nxl_header::validate_section(const std::wstring& name, HANDLE h)
{
    auto pos = std::find_if(_sections.begin(), _sections.end(), [&](const nxl_section& sc) ->bool {
        return (0 == _wcsicmp(sc.name().c_str(), name.c_str()));
    });
    if (pos == _sections.end()) {
        throw std::exception("secction not exists");
    }
    validate_section(*pos, h);
}

void nxl_header::validate_section(const nxl_section& sc, HANDLE h)
{
    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h, sc.offset(), NULL, FILE_BEGIN)) {
        throw std::exception("fail to move to begining of file");
    }

    std::vector<unsigned char> data;
    unsigned long bytes_read = 0;

    data.resize(sc.size(), 0);
    if (!::ReadFile(h, &data[0], (unsigned long)data.size(), &bytes_read, NULL)) {
        throw std::exception("fail to read secction data");
    }

    //boost::crc_32_type  result;
    //result.process_bytes(data.data(), data.size());
    //unsigned long real_checksum = result.checksum();
    unsigned long real_checksum = nudf::crypto::ToCrc32(0, data.data(), (unsigned long)data.size());
    if (sc.checksum() != real_checksum) {
        throw std::exception("checksum doesn't match");
    }
}

bool nxl_header::is_content_key_aes128() const noexcept
{
    return (algorithm() == NXL_ALGORITHM_AES128);
}
bool nxl_header::is_content_key_aes256() const noexcept
{
    return (algorithm() == NXL_ALGORITHM_AES256);
}
bool nxl_header::is_primary_key_aes128() const noexcept
{
    return (primary_key_id().algorithm() == NXL_ALGORITHM_AES128);
}
bool nxl_header::is_primary_key_aes256() const noexcept
{
    return (primary_key_id().algorithm() == NXL_ALGORITHM_AES256);
}
bool nxl_header::is_recovery_key_aes128() const noexcept
{
    return (has_recovery_key() && recovery_key_id().algorithm() == NXL_ALGORITHM_AES128);
}
bool nxl_header::is_recovery_key_aes256() const noexcept
{
    return (has_recovery_key() && recovery_key_id().algorithm() == NXL_ALGORITHM_AES256);
}
bool nxl_header::is_recovery_key_rsa1024() const noexcept
{
    return (has_recovery_key() && recovery_key_id().algorithm() == NXL_ALGORITHM_RSA1024);
}
bool nxl_header::is_recovery_key_rsa2048() const noexcept
{
    return (has_recovery_key() && recovery_key_id().algorithm() == NXL_ALGORITHM_RSA2048);
}

std::vector<unsigned char> nxl_header::decrypt_content_key(const std::vector<unsigned char>& key) const noexcept
{
    std::vector<unsigned char> content_key;

    if ((primary_key_id().algorithm() == NXL_ALGORITHM_AES128 && key.size() != 16) || (primary_key_id().algorithm() == NXL_ALGORITHM_AES256 && key.size() != 32)) {
        return content_key;
    }

    try {
        std::vector<unsigned char> content_key_buf;
        nudf::crypto::CAesKeyBlob kb;
        kb.SetKey(key.data(), (unsigned long)key.size());
        if (!nudf::crypto::AesDecrypt(kb, _primary_content_key.data(), (unsigned long)_primary_content_key.size(), 0, content_key_buf)) {
            throw WIN32ERROR();
        }
        if(content_key_buf.size() != _primary_content_key.size()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        unsigned long content_key_size = (NXL_ALGORITHM_AES128 == algorithm()) ? 16 : 32;
        content_key.resize(content_key_size, 0);
        memcpy(&content_key[0], content_key_buf.data(), content_key_size);
    }
    catch (nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        content_key.clear();
    }

    return content_key;
}

std::vector<unsigned char> nxl_header::recovery_content_key(const std::vector<unsigned char>& key) const noexcept
{
    std::vector<unsigned char> content_key;
    return content_key;
}

unsigned long nxl_header::decrypt_sections_checksum(const std::vector<unsigned char>& key) const noexcept
{
    unsigned long checksum = 0;

    if (key.size() != 16 && key.size() != 32) {
        return 0;
    }

    try {
        std::vector<unsigned char> checksum_buf;
        nudf::crypto::CAesKeyBlob kb;
        kb.SetKey(key.data(), 16);

        if (!nudf::crypto::AesDecrypt(kb, _sections_checksum.data(), 16, 0, checksum_buf)) {
            throw WIN32ERROR();
        }
        if (checksum_buf.size() != 16) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        checksum = *((unsigned long*)checksum_buf.data());
    }
    catch (nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        checksum = 0;
    }

    return checksum;
}

void nxl_header::update_sections_checksum(const std::vector<unsigned char>& key, HANDLE h)
{
    unsigned long checksum = 0;

    if (key.size() != 16 && key.size() != 32) {
        throw std::exception("invalid key size");
    }
    if (h == INVALID_HANDLE_VALUE || h == NULL) {
        throw std::exception("invalid handle");
    }

    checksum = calc_sections_checksum();
    unsigned char cipher_checksum[16] = { 0 };
    memset(cipher_checksum, 0, sizeof(cipher_checksum));
    memcpy(cipher_checksum, &checksum, sizeof(unsigned long));

    nudf::crypto::CAesKeyBlob kb;
    kb.SetKey(key.data(), 16);
    if (!nudf::crypto::AesEncrypt(kb, cipher_checksum, 16, 0)) {
        throw std::exception("fail to encrypt section table's checksum");
    }

    unsigned long checksum_offset = FIELD_OFFSET(NXL_HEADER, Sections.Checksum);

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h, checksum_offset, NULL, FILE_BEGIN)) {
        throw std::exception("fail to move to begining of section table's checksum in file");
    }

    unsigned long bytes_written = 0;
    if (!::WriteFile(h, cipher_checksum, 16, &bytes_written, NULL)) {
        throw std::exception("fail to write secction table's checksum to file");
    }
}

unsigned long nxl_header::calc_sections_checksum() const noexcept
{
    NXL_SECTION_TABLE section_table;

    memset(&section_table, 0, sizeof(NXL_SECTION_TABLE));
    section_table.Count = (unsigned long)_sections.size();
    for (int i = 0; i < (int)_sections.size(); i++) {
        std::string s = NX::utility::conversions::utf16_to_utf8(_sections[i].name());
        assert(s.length() <= 8);
        memcpy(section_table.Sections[i].Name, s.c_str(), s.length());
        section_table.Sections[i].Checksum = _sections[i].checksum();
        section_table.Sections[i].Size = _sections[i].size();
    }

    unsigned long checksum = nudf::crypto::ToCrc32(0, &section_table.Count, sizeof(ULONG));
    if (0 != section_table.Count) {
        checksum = nudf::crypto::ToCrc32(checksum, &section_table.Sections[0], sizeof(NXL_SECTION) * section_table.Count);
    }
    return checksum;

    //boost::crc_32_type  result;
    //result.process_bytes(&section_table.Count, sizeof(NXL_SECTION_TABLE) - FIELD_OFFSET(NXL_SECTION_TABLE, Count));
    //return result.checksum();
}

void nxl_header::get_section_data(const std::wstring& name, std::vector<unsigned char>& data, HANDLE h, bool* validated)
{
    auto pos = std::find_if(_sections.begin(), _sections.end(), [&](const nxl_section& sc) ->bool {
        return (0 == _wcsicmp(sc.name().c_str(), name.c_str()));
    });
    if (pos == _sections.end()) {
        throw std::exception("secction not exists");
    }

    get_section_data(*pos, data, h, validated);
}

void nxl_header::get_section_data(const nxl_section& sc, std::vector<unsigned char>& data, HANDLE h, bool* validated)
{
    if (h == INVALID_HANDLE_VALUE || h == NULL) {
        throw std::exception("invalid handle");
    }

    // no data
    if (sc.size() == 0) {
        return;
    }

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h, sc.offset(), NULL, FILE_BEGIN)) {
        throw std::exception("fail to move to begining of section data");
    }

    unsigned long bytes_read = 0;

    data.resize(sc.size(), 0);
    if (!::ReadFile(h, &data[0], (unsigned long)data.size(), &bytes_read, NULL)) {
        data.clear();
        throw std::exception("fail to read secction data");
    }

    if (NULL != validated) {
        //boost::crc_32_type  result;
        //result.process_bytes(data.data(), data.size());
        //unsigned long real_checksum = result.checksum();
        unsigned long real_checksum = nudf::crypto::ToCrc32(0, data.data(), (unsigned long)data.size());
        *validated = (sc.checksum() == real_checksum) ? true : false;
    }
}

void nxl_header::set_section_data(const std::wstring& name, const std::vector<unsigned char>& data, const std::vector<unsigned char>& key, HANDLE h)
{
    bool found = false;
    int section_index = 0;

    if (h == INVALID_HANDLE_VALUE || h == NULL) {
        throw std::exception("invalid handle");
    }

    for (auto it = _sections.begin(); it != _sections.end(); ++it) {
        if (0 != _wcsicmp((*it).name().c_str(), name.c_str())) {
            found = true;
            break;;
        }
        ++section_index;
    }

    if (!found) {
        throw std::exception("secction not exists");
    }

    nxl_section& sc = _sections[section_index];

    // no data
    if (sc.size() == 0) {
        throw std::exception("invalid section size");
    }

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h, sc.offset(), NULL, FILE_BEGIN)) {
        throw std::exception("fail to move to begining of section data");
    }

    std::vector<unsigned char> sc_data;
    sc_data.resize(sc.size(), 0);
    if (data.size() >= sc.size()) {
        memcpy(&sc_data[0], data.data(), sc.size() - 1);    // it must be end with '\0'
    }
    else {
        memcpy(&sc_data[0], data.data(), data.size());
    }

    unsigned long bytes_written = 0;
    if (!::WriteFile(h, sc_data.data(), (unsigned long)sc_data.size(), &bytes_written, NULL)) {
        throw std::exception("fail to write secction data");
    }

    unsigned long new_checksum = nudf::crypto::ToCrc32(0, sc_data.data(), (unsigned long)sc_data.size());
    sc.set_checksum(new_checksum);

    // write checksum
    unsigned long checksum_offset = (unsigned long)(FIELD_OFFSET(NXL_HEADER, Sections.Sections) + section_index * sizeof(NXL_SECTION) + FIELD_OFFSET(NXL_SECTION, Checksum));
    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(h, checksum_offset, NULL, FILE_BEGIN)) {
        throw std::exception("fail to move to begining of section's checksum");
    }
    if (!::WriteFile(h, &checksum_offset, (unsigned long)sizeof(unsigned long), &bytes_written, NULL)) {
        throw std::exception("fail to write secction checksum");
    }

    // update section table's checksum
    update_sections_checksum(key, h);
}

void nxl_header::set_section_data(nxl_section& sc, const std::vector<unsigned char>& data, const std::vector<unsigned char>& key, HANDLE h)
{
    set_section_data(sc.name(), data, key, h);
}

std::map<std::wstring, std::wstring> nxl_header::load_section_attributes(HANDLE h, bool* validated)
{
    std::map<std::wstring, std::wstring> attributes;
    std::vector<unsigned char> data;
    get_section_data(NXL_SECTION_ATTRIBUTESW, data, h, validated);

    if (data.size() == 0) {
        return attributes;
    }
    
    if (data[0] == '{' && data[1] != 0) {
        assert(0 == data[data.size() - 1]);
        if (0 != data[data.size() - 1]) {
            data[data.size() - 1] = 0;
        }
        std::string s = (char*)data.data();
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        assert(v.is_object());
        const NX::web::json::object& obj = v.as_object();

        std::for_each(obj.begin(), obj.end(), [&](const std::pair<std::wstring,NX::web::json::value>& it) {
            attributes[it.first] = it.second.as_string();
        });
    }
    else if (data[0] == 0 && data[1] == 0) {
        ; // empty
    }
    else {
        // wide char*, end with double zero
        assert(0 == data[data.size() - 1]);
        assert(0 == data[data.size() - 2]);
        assert(0 == data[data.size() - 3]);
        assert(0 == data[data.size() - 4]);
        if (0 != data[data.size() - 1]) data[data.size() - 1] = 0;
        if (0 != data[data.size() - 2]) data[data.size() - 2] = 0;
        if (0 != data[data.size() - 3]) data[data.size() - 3] = 0;
        if (0 != data[data.size() - 4]) data[data.size() - 4] = 0;

        const wchar_t* p = (const wchar_t*)data.data();
        while (0 != *p) {
            std::wstring ws = p;
            p += ws.length() + 1;

            std::wstring::size_type pos = ws.find(L"=");
            if (std::wstring::npos == pos) {
                continue;
            }

            std::wstring name = ws.substr(0, pos);
            std::wstring value = ws.substr(pos + 1);
            attributes[name] = value;
        }
    }

    return std::move(attributes);
}

std::wstring nxl_header::load_section_templates(HANDLE h, bool* validated)
{
    std::wstring templates;
    std::vector<unsigned char> data;
    get_section_data(NXL_SECTION_TEMPLATESW, data, h, validated);

    if (data.size() == 0) {
        return templates;
    }

    std::string s = (const char*)data.data();
    templates = NX::utility::conversions::utf8_to_utf16(s);

    return std::move(templates);
}

std::multimap<std::wstring, std::wstring> nxl_header::load_section_tags(HANDLE h, bool* validated)
{
    std::multimap<std::wstring, std::wstring> tags;
    std::vector<unsigned char> data;
    get_section_data(NXL_SECTION_TAGSW, data, h, validated);

    if (data.size() == 0) {
        return tags;
    }

    if (data[0] == '{' && data[1] != 0) {
        assert(0 == data[data.size() - 1]);
        if (0 != data[data.size() - 1]) {
            data[data.size() - 1] = 0;
        }
        std::string s = (char*)data.data();
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        assert(v.is_object());
        const NX::web::json::object& obj = v.as_object();

        std::for_each(obj.begin(), obj.end(), [&](const std::pair<std::wstring, NX::web::json::value>& it) {
            tags.insert(std::pair<std::wstring, std::wstring>(it.first, it.second.as_string()));
        });
    }
    else if (data[0] == 0 && data[1] == 0) {
        ; // empty
    }
    else {
        // wide char*, end with double zero
        assert(0 == data[data.size() - 1]);
        assert(0 == data[data.size() - 2]);
        assert(0 == data[data.size() - 3]);
        assert(0 == data[data.size() - 4]);
        if (0 != data[data.size() - 1]) data[data.size() - 1] = 0;
        if (0 != data[data.size() - 2]) data[data.size() - 2] = 0;
        if (0 != data[data.size() - 3]) data[data.size() - 3] = 0;
        if (0 != data[data.size() - 4]) data[data.size() - 4] = 0;

        const wchar_t* p = (const wchar_t*)data.data();
        while (0 != *p) {
            std::wstring ws = p;
            p += ws.length() + 1;

            std::wstring::size_type pos = ws.find(L"=");
            if (std::wstring::npos == pos) {
                continue;
            }

            std::wstring name = ws.substr(0, pos);
            std::wstring value = ws.substr(pos + 1);
            tags.insert(std::pair<std::wstring, std::wstring>(name, value));
        }
    }

    return std::move(tags);
}