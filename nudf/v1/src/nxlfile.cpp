

#include <Windows.h>

#include <fstream>

#include <boost\crc.hpp>

#include <nudf\shared\nxlfmt.h>
#include <nudf\web\json.hpp>
#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\crypto.hpp>
#include <nudf\nxlfile.hpp>


using namespace NX;
using namespace NX::NXL;


static const size_t min_buf_size = 0x100000;    // 1 MB
static const size_t max_buf_size = 0x1000000;   // 16 MB



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

bool nxl_key_id::operator == (const  nxl_key_id& other) const noexcept
{
    if (empty()) {
        return false;
    }
    if (algorithm() != other.algorithm()) {
        return false;
    }
    if (size() != other.size()) {
        return false;
    }
    return (0 == memcmp(id(), other.id(), size()));
}



//
//  class nxl_header
//
nxl_key_pkg::nxl_key_pkg()
{
}

nxl_key_pkg::~nxl_key_pkg()
{
}

nxl_key_pkg::nxl_key_pkg(const nxl_key_id& id, const unsigned char* key, const unsigned long size) : _id(id), _key(std::vector<unsigned char>(key, key+size))
{
}

nxl_key_pkg::nxl_key_pkg(const nxl_key_id& id, const std::vector<unsigned char>& key) : _id(id), _key(key)
{
}

nxl_key_pkg::nxl_key_pkg(const nxl_key_id& id, const NX::secure_mem& key) : _id(id), _key(key)
{
}

nxl_key_pkg& nxl_key_pkg::operator = (const nxl_key_pkg& other) noexcept
{
    if (this != &other) {
        _id = other.id();
        _key = other.key();
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
    clear();
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


static std::vector<std::pair<std::wstring, unsigned long>> build_full_sections(const std::map<std::wstring, unsigned long>& sections)
{
    std::vector<std::pair<std::wstring, unsigned long>> full_sections = { { NXL_SECTION_ATTRIBUTESW , 2048 },{ NXL_SECTION_TEMPLATESW , 4096 },{ NXL_SECTION_TAGSW , 4096 } };
    std::for_each(sections.begin(), sections.end(), [&](const std::pair<std::wstring, unsigned long>& section) {
        if (0 == _wcsicmp(section.first.c_str(), NXL_SECTION_ATTRIBUTESW)) {
            if (section.second < 2048 || 0 != ((section.second - 2048) % 4096)) {
                throw std::exception("invalid attribute section size");
            }
            full_sections[0].second = section.second;
        }
        else  if (0 == _wcsicmp(section.first.c_str(), NXL_SECTION_TEMPLATESW)) {
            if (0 != (section.second % 4096)) {
                throw std::exception("invalid rights template section size");
            }
            full_sections[1].second = section.second;
        }
        else  if (0 == _wcsicmp(section.first.c_str(), NXL_SECTION_TAGSW)) {
            if (0 != (section.second % 4096)) {
                throw std::exception("invalid tag section size");
            }
            full_sections[2].second = section.second;
        }
        else {
            if (0 != (section.second % 4096)) {
                throw std::exception("invalid section size");
            }
            full_sections.push_back(section);
        }
    });
    return full_sections;
}

void nxl_header::create(const std::wstring& message, const nxl_key_pkg& primary_key_pkg, const nxl_key_pkg& recovery_key_pkg, const std::map<std::wstring, unsigned long>& sections)
{
    nudf::crypto::CAesKeyBlob content_key;
    nudf::crypto::CAesKeyBlob key_enc_key;
    std::vector<unsigned char> tmp_key;
    std::vector<std::pair<std::wstring, unsigned long>> full_sections = build_full_sections(sections);

    clear();

    if (full_sections.size() > 72) {
        throw std::exception("too many sections");
    }

    if (primary_key_pkg.empty()) {
        throw std::exception("invalid primary key");
    }

    try {
        // generate content key
        content_key.Generate(256);

        // message
        _message = message;

        // thumbprint
        FILETIME ft = { 0, 0 };
        GetSystemTimeAsFileTime(&ft);
        std::vector<unsigned char> thp;
        nudf::crypto::ToMd5(&ft, sizeof(ft), thp);
        _thumbprint = nudf::string::FromBytes<wchar_t>(thp.data(), (unsigned long)thp.size());

        _version = NXL_VERSION_10;
        _flags = 0;
        _alignment = NXL_PAGE_SIZE;
        _content_offset = sizeof(NXL_HEADER);
        _algorithm = NXL_ALGORITHM_AES256;
        _cbc_size = NXL_CBC_SIZE;

        // set primary key
        _primary_key_id = primary_key_pkg.id();
        _primary_content_key.resize(256, 0);
        memcpy(&_primary_content_key[0], content_key.GetKey(), content_key.GetKeySize());
        tmp_key = primary_key_pkg.plain_key();
        key_enc_key.SetKey(tmp_key.data(), (unsigned long)tmp_key.size());
        tmp_key.clear();
        nudf::crypto::AesEncrypt(key_enc_key, &_primary_content_key[0], 256, 0);

        // set recovery key
        if (!recovery_key_pkg.empty()) {
            _recovery_key_id = recovery_key_pkg.id();
            _recovery_content_key.resize(256, 0);
            memcpy(&_recovery_content_key[0], content_key.GetKey(), content_key.GetKeySize());
            tmp_key = recovery_key_pkg.plain_key();
            key_enc_key.SetKey(tmp_key.data(), (unsigned long)tmp_key.size());
            tmp_key.clear();
            nudf::crypto::AesEncrypt(key_enc_key, &_recovery_content_key[0], 256, 0);
        }

        // set section
        unsigned long section_offset = 0;
        std::vector<unsigned char> zero_data;
        std::for_each(full_sections.begin(), full_sections.end(), [&](const std::pair<std::wstring, unsigned long>& section) {
            unsigned long zero_data_checksum = 0;
            if (zero_data.size() < section.second) {
                zero_data.resize(section.second, 0);
            }
            zero_data_checksum = nudf::crypto::ToCrc32(0, zero_data.data(), (unsigned long)zero_data.size());
            _sections.push_back(nxl_section(section.first, section_offset, section.second, zero_data_checksum));

            _content_offset += section.second;
            section_offset  += section.second;
        });

        unsigned long section_table_checksum = this->calc_sections_checksum();
        _sections_checksum.resize(16);
        memcpy(&_sections_checksum[0], &section_table_checksum, sizeof(section_table_checksum));
        nudf::crypto::CAesKeyBlob aes16_key;
        aes16_key.SetKey(content_key.GetKey(), 16);
        nudf::crypto::AesEncrypt(aes16_key, &_sections_checksum[0], 16, 0);
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
        throw e;
    }
}

std::vector<unsigned char> nxl_header::to_buffer()
{
    std::vector<unsigned char> header_buf;
    header_buf.resize(sizeof(NXL_HEADER), 0);
    NXL_HEADER* header = (NXL_HEADER*)header_buf.data();
    
    // fill header
    memset(&header, 0, sizeof(header));
    //   --> signature
    wcsncpy_s(header->Signature.Message, 67, _message.c_str(), _TRUNCATE);
    header->Signature.Code.HighPart = NXL_SIGNATURE_HIGH;
    header->Signature.Code.LowPart = NXL_SIGNATURE_LOW;
    //   --> basic
    std::vector<unsigned char> thp;
    nudf::string::ToBytes<wchar_t>(_thumbprint, thp);
    memcpy(header->Basic.Thumbprint, thp.data(), 16);
    header->Basic.Version = version();
    header->Basic.Flags   = flags();
    header->Basic.Alignment = alignment();
    header->Basic.PointerOfContent = content_offset();

    // crypto
    header->Crypto.Algorithm        = algorithm();
    header->Crypto.CbcSize          = cbc_size();
    header->Crypto.ContentLength    = 0;
    header->Crypto.AllocateLength   = 0;

    // Primary Key
    header->Crypto.PrimaryKey.KeKeyId.Algorithm = primary_key_id().algorithm();
    header->Crypto.PrimaryKey.KeKeyId.IdSize = primary_key_id().size();
    memcpy(header->Crypto.PrimaryKey.KeKeyId.Id, primary_key_id().id(), primary_key_id().size());
    memcpy(header->Crypto.PrimaryKey.CeKey, primary_content_key().data(), primary_content_key().size());

    // recovery key
    if (!recovery_key_id().empty() && !recovery_content_key().empty()) {
        header->Crypto.PrimaryKey.KeKeyId.Algorithm = recovery_key_id().algorithm();
        header->Crypto.PrimaryKey.KeKeyId.IdSize = recovery_key_id().size();
        memcpy(header->Crypto.PrimaryKey.KeKeyId.Id, recovery_key_id().id(), recovery_key_id().size());
        memcpy(header->Crypto.PrimaryKey.CeKey, recovery_content_key().data(), recovery_content_key().size());
    }

    // sections
    //  --> table
    header->Sections.Count = (unsigned long)sections().size();
    memcpy(header->Sections.Checksum, section_table_checksum().data(), 16);
    //  --> each sections
    int index = 0;
    std::for_each(sections().begin(), sections().end(), [&](const nxl_section& section) {
        memcpy(header->Sections.Sections[index].Name, section.name().c_str(), section.name().length());
        header->Sections.Sections[index].Size = section.size();
        header->Sections.Sections[index].Checksum = section.checksum();
    });

    return header_buf;
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
        CloseHandle(h);
        h = INVALID_HANDLE_VALUE;
    }
    catch (std::exception e) {
        if (INVALID_HANDLE_VALUE != h) {
            CloseHandle(h);
            h = INVALID_HANDLE_VALUE;
        }
        throw e;
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
    _content_size   = header.Crypto.ContentLength;
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

void nxl_header::validate_sections(const std::vector<unsigned char>& key, HANDLE h /*optional*/) const
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

void nxl_header::validate_section(const std::wstring& name, HANDLE h) const
{
    auto pos = std::find_if(_sections.begin(), _sections.end(), [&](const nxl_section& sc) ->bool {
        return (0 == _wcsicmp(sc.name().c_str(), name.c_str()));
    });
    if (pos == _sections.end()) {
        throw std::exception("secction not exists");
    }
    validate_section(*pos, h);
}

void nxl_header::validate_section(const nxl_section& sc, HANDLE h) const
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

void nxl_header::update_sections_checksum(const std::vector<unsigned char>& key)
{
    unsigned long checksum = 0;

    if (key.size() != 16 && key.size() != 32) {
        throw std::exception("invalid key size");
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
    _sections_checksum = std::vector<unsigned char>(cipher_checksum, cipher_checksum + 16);

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


//
//  class nxl_file
//

nxl_file::nxl_file() : _h(INVALID_HANDLE_VALUE), _read_only(false)
{
}

nxl_file::~nxl_file()
{
    close();
}

bool nxl_file::verify(const std::wstring& file) noexcept
{
    bool result = false;
    try {
        nxl_header header;
        header.load(file);
        result = !header.empty();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    return result;
}

void nxl_file::create(const std::wstring& target,
                      const std::wstring& source,
                      const nxl_key_pkg& primary_key,
                      const nxl_key_pkg& recovery_key,
                      const std::wstring& message,
                      const std::map<std::wstring, unsigned long>& sections)
{
    close();

    // only source is specified
    if (target.empty()) {
        if (source.empty()) {
            throw std::exception("target and source are empty at the same time");
        }
        convert(source, primary_key, recovery_key, message, sections);
        return;
    }

    // source and target are same
    if (0 == _wcsicmp(target.c_str(), source.c_str())) {
        convert(source, primary_key, recovery_key, message, sections);
        return;
    }


    //
    //  target doesn't exist
    //

    _header.create(message, primary_key, recovery_key, sections);
    assert(!_header.empty());

    // set file name
    _file_name = target;
    if (!boost::algorithm::iends_with(target, L".nxl")) {
        _file_name += L".nxl";
    }

    try {

        std::vector<unsigned char> content_key_data = _header.decrypt_content_key(primary_key.key().decrypt());
        if (content_key_data.empty()) {
            throw std::exception("fail to decrypt content key");
        }

        nudf::crypto::CAesKeyBlob content_key;
        content_key.SetKey(content_key_data.data(), (unsigned long)content_key_data.size());

        _content_key.encrypt(content_key_data);

        unsigned long bytes_written = 0;

        _h = ::CreateFileW(_file_name.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (_h == INVALID_HANDLE_VALUE) {
            throw std::exception("fail to create new file");
        }

        std::vector<unsigned char> header_buf = _header.to_buffer();
        assert(header_buf.size() == sizeof(NXL_HEADER));

        if (!::WriteFile(_h, header_buf.data(), (unsigned long)header_buf.size(), &bytes_written, NULL)) {
            throw std::exception("fail to write nxl header");
        }
        if ((unsigned long)header_buf.size() != bytes_written) {
            throw std::exception("fail to write nxl header (not all the data is written)");
        }

        if(INVALID_SET_FILE_POINTER == ::SetFilePointer(_h, _header.content_offset(), NULL, FILE_BEGIN)) {
            throw std::exception("fail to move fp to begining of content");
        }
        if (!SetEndOfFile(_h)) {
            throw std::exception("fail to set file init size");
        }

        if (!source.empty()) {

            HANDLE  hs = INVALID_HANDLE_VALUE;
            hs = ::CreateFileW(source.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (INVALID_HANDLE_VALUE == hs) {
                throw std::exception("fail to open source file");
            }

            try {

                unsigned long length_high = 0;
                unsigned long length = GetFileSize(hs, &length_high);

                if (length_high != 0) {
                    throw std::exception("source file is too big");
                }

                if (0 != length) {

                    // prepare trans buffer
                    std::vector<unsigned char> transbuf;
                    if (length <= max_buf_size) {
                        transbuf.resize(round_to_size(length, 4096), 0);
                    }
                    else {
                        transbuf.resize(max_buf_size, 0);
                    }

                    // transfer data
                    unsigned long long data_offset = 0;

                    do {

                        unsigned long bytes_read = 0;
                        unsigned long bytes_written = 0;
                        unsigned long bytes_to_read = 0;
                        unsigned long bytes_to_encrypt = 0;

                        if (length <= max_buf_size) {
                            bytes_to_read = length;
                            bytes_to_encrypt = (unsigned long)round_to_size(length, 4096);
                            assert((unsigned long)transbuf.size() >= bytes_to_encrypt);
                        }
                        else {
                            bytes_to_read = max_buf_size;
                            bytes_to_encrypt = max_buf_size;
                        }

                        if (!::ReadFile(hs, (unsigned char*)transbuf.data(), bytes_to_read, &bytes_read, NULL)) {
                            throw std::exception("fail to read source file");
                        }
                        if (bytes_to_read != bytes_read) {
                            throw std::exception("fail to read source file (not all data is read)");
                        }

                        if (bytes_to_read != bytes_to_encrypt) {
                            // zero data
                            memset((unsigned char*)transbuf.data() + bytes_to_read, 0, bytes_to_encrypt - bytes_to_read);
                        }

                        // encrypt data
                        if (!nudf::crypto::AesEncrypt(content_key, transbuf.data(), bytes_to_encrypt, data_offset)) {
                            throw std::exception("fail to read source file content");
                        }

                        // write to target
                        if (!WriteFile(_h, transbuf.data(), bytes_to_encrypt, &bytes_written, NULL)) {
                            throw std::exception("fail to write encrypted data to target file");
                        }

                        // move to next block
                        data_offset += bytes_to_read;
                        length -= bytes_to_read;

                    } while (length != 0);

                    // update content length
                    _header._content_size = length;
                    if (INVALID_SET_FILE_POINTER == SetFilePointer(_h, FIELD_OFFSET(NXL_HEADER, Crypto.ContentLength), NULL, FILE_BEGIN)) {
                        throw std::exception("fail to move file pointer to NXL_HEADER.Crypto.ContentLength");
                    }
                    if (!::WriteFile(_h, &_header._content_size, sizeof(unsigned long long), &bytes_written, NULL)) {
                        throw std::exception("fail to update file content size");
                    }
                }

                CloseHandle(hs);
                hs = INVALID_HANDLE_VALUE;
            }
            catch (std::exception& e) {
                CloseHandle(hs);
                hs = INVALID_HANDLE_VALUE;
                throw e;
            }
        }
    }
    catch (std::exception& e) {
        close();
        throw e;
    }
}


void nxl_file::convert(const std::wstring& file,
                       const nxl_key_pkg& primary_key,
                       const nxl_key_pkg& recovery_key,
                       const std::wstring& message,
                       const std::map<std::wstring, unsigned long>& sections)
{
}

void nxl_file::open(const std::wstring& source, const nxl_key_pkg& key_package, bool read_only)
{
    std::wstring in_file(source);

    if (!boost::algorithm::iends_with(in_file, L".nxl")) {
        in_file += L".nxl";
    }

    _h = ::CreateFileW(in_file.c_str(), read_only ? GENERIC_READ : (GENERIC_WRITE | GENERIC_READ), 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (_h == INVALID_HANDLE_VALUE) {
        throw std::exception("fail to open source file");
    }


    try {

        _header.load(_h);
        if (key_package.id() == header().primary_key_id()) {
            std::vector<unsigned char> key = header().decrypt_content_key(key_package.key().decrypt());
            if (key.empty()) {
                throw std::exception("fail to decrypt content key");
            }
            _content_key.encrypt(key);
        }
        else if (!header().recovery_key_id().empty() && key_package.id() == header().recovery_key_id()) {
            std::vector<unsigned char> key = header().recovery_content_key(key_package.key().decrypt());
            if (key.empty()) {
                throw std::exception("fail to recovery content key");
            }
            _content_key.encrypt(key);
        }
        else {
            throw std::exception("inavlid key");
        }
    
        _read_only = read_only;
        _file_name = in_file;
    }
    catch (std::exception& e) {
        close();
        throw e;
    }
}

void nxl_file::open(const std::wstring& source, const std::vector<nxl_key_pkg>& key_pkgs, bool read_only)
{
    std::wstring in_file(source);

    if (!boost::algorithm::iends_with(in_file, L".nxl")) {
        in_file += L".nxl";
    }

    _h = ::CreateFileW(source.c_str(), read_only ? GENERIC_READ : (GENERIC_WRITE | GENERIC_READ), 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (_h == INVALID_HANDLE_VALUE) {
        throw std::exception("fail to open source file");
    }


    try {

        _header.load(_h);

        auto pos = std::find_if(key_pkgs.begin(), key_pkgs.end(), [&](const nxl_key_pkg& key_package)->bool {
            return (key_package.id() == header().primary_key_id());
        });

        if (pos != key_pkgs.end()) {
            std::vector<unsigned char> key = header().decrypt_content_key((*pos).key().decrypt());
            if (!key.empty()) {
                _content_key.encrypt(key);
            }
        }
        else {

            pos = std::find_if(key_pkgs.begin(), key_pkgs.end(), [&](const nxl_key_pkg& key_package)->bool {
                return (key_package.id() == header().recovery_key_id());
            });
            // still not find valid key
            if (pos != key_pkgs.end()) {
                // use recovery key
                std::vector<unsigned char> key = header().recovery_content_key((*pos).key().decrypt());
                if (!key.empty()) {
                    _content_key.encrypt(key);
                }
            }
        }
            
        _read_only = read_only;
        _file_name = in_file;
    }
    catch (std::exception& e) {
        close();
        throw e;
    }
}

void nxl_file::close() noexcept
{
    if (_h != INVALID_HANDLE_VALUE) {
        CloseHandle(_h);
        _h = INVALID_HANDLE_VALUE;
    }
    _file_name.clear();
    _content_key.clear();
    _header.clear();
    _read_only = false;
}

void nxl_file::decrypt(const std::wstring& target)
{
    HANDLE oh = INVALID_HANDLE_VALUE;
    std::wstring temp_file;
    bool temp_file_created = false;

    if (!opened()) {
        throw std::exception("file is not opened");
    }
    if (read_only()) {
        throw std::exception("file is opened in read only mode");
    }
    if (boost::algorithm::iends_with(target, L".nxl")) {
        throw std::exception("invalid output file name");
    }

    if (_content_key.empty()) {
        throw std::exception("inavlid content key");
    }
    const std::vector<unsigned char> content_key_data(_content_key.decrypt());
    nudf::crypto::CAesKeyBlob content_key;
    content_key.SetKey(content_key_data.data(), (unsigned long)content_key_data.size());

    try {

        temp_file = nxl_file::get_temp_file_name(target.empty() ? file_name() : target);
        assert(!temp_file.empty());
        temp_file_created = true;
        
        oh = ::CreateFileW(temp_file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
        if (INVALID_HANDLE_VALUE == oh) {
            throw std::exception("fail to create temp file");
        }

        __int64 offset = 0;
        __int64 length = header().content_length();
        std::vector<unsigned char> data;
        data.resize(((size_t)length < max_buf_size) ? (size_t)length : max_buf_size, 0);
        SetFilePointer(_h, header().content_offset(), NULL, FILE_BEGIN);
        while (length != 0) {

            unsigned long bytes_to_decrypt = 0;
            unsigned long bytes_decrypted = 0;
            unsigned long bytes_read = 0;
            unsigned long bytes_written = 0;

            bytes_to_decrypt = (length > (__int64)data.size()) ? ((unsigned long)data.size()) : ((unsigned long)round_to_size((unsigned long)length, header().alignment()));
            if (!ReadFile(_h, &data[0], bytes_to_decrypt, &bytes_read, NULL)) {
                throw std::exception("fail to read cipher data from NXL file");
            }
            if (bytes_to_decrypt != bytes_read) {
                throw std::exception("fail to read all cipher data from NXL file");
            }

            if (!nudf::crypto::AesDecrypt(content_key, &data[0], bytes_to_decrypt, offset)) {
                throw std::exception("fail to decrypt cipher data");
            }

            bytes_decrypted = (bytes_to_decrypt > length) ? ((unsigned long)length) : bytes_to_decrypt;
            length -= bytes_decrypted;
            offset += bytes_decrypted;

            if (!::WriteFile(oh, data.data(), bytes_decrypted, &bytes_written, NULL)) {
                throw std::exception("fail to write plain data to temp file");
            }
        }
        CloseHandle(oh);
        oh = INVALID_HANDLE_VALUE;

        if (!target.empty()) {
            // move
            if (!MoveFileW(temp_file.c_str(), target.c_str())) {
                throw std::exception("fail to rename temp file");
            }
        }
        else {
            // replace current file
            CloseHandle(_h);
            _h = INVALID_HANDLE_VALUE;
            std::wstring out_file = file_name().substr(0, file_name().length() - 4); // remove L".nxl" extension
            if (!::ReplaceFileW(out_file.c_str(), temp_file.c_str(), NULL, REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS, NULL, NULL)) {
                throw std::exception("fail to replace original file");
            }
        }
    }
    catch (std::exception& e) {
        if (INVALID_HANDLE_VALUE != oh) {
            CloseHandle(oh);
            oh = INVALID_HANDLE_VALUE;
            ::DeleteFileW(temp_file.c_str());
        }
        if (temp_file_created) {
            ::DeleteFileW(temp_file.c_str());
        }
        if (INVALID_HANDLE_VALUE == _h) {
            // try to re-open source file, if it is necessary
            _h = ::CreateFileW(file_name().c_str(), read_only() ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (INVALID_HANDLE_VALUE == _h) {
                close();
            }
        }
        throw e;
    }
}

void nxl_file::update_section_table_checksum()
{
    assert(opened());
    assert(!read_only());

    if (_content_key.empty()) {
        throw std::exception("invalid content key");
    }

    _header.update_sections_checksum(_content_key.decrypt());

    const unsigned long checksum_offset = FIELD_OFFSET(NXL_HEADER, Sections.Checksum);

    if (INVALID_SET_FILE_POINTER == ::SetFilePointer(_h, checksum_offset, NULL, FILE_BEGIN)) {
        throw std::exception("fail to move to begining of section table's checksum in file");
    }

    unsigned long bytes_written = 0;
    if (!::WriteFile(_h, header().section_table_checksum().data(), 16, &bytes_written, NULL)) {
        throw std::exception("fail to write secction table's checksum to file");
    }
}

std::vector<unsigned char> nxl_file::create_tags_buffer_legacy(const TagMapType& tags)
{
    // calculate total size
    unsigned long size = 0;
    std::for_each(tags.begin(), tags.end(), [&](const std::pair<std::wstring, std::set<std::wstring>>& item) {
        std::for_each(item.second.begin(), item.second.end(), [&](const std::wstring& value) {
            size += (unsigned long)item.first.length();
            size += 1;  // L'='
            size += (unsigned long)value.length();
            size += 1;  // L'\0'
        });
    });
    size += 1;  // L'\0'

    std::vector<unsigned char> data;
    data.resize(size*sizeof(wchar_t), 0);
    wchar_t* p = (wchar_t*)data.data();

    // copy data
    std::for_each(tags.begin(), tags.end(), [&](const std::pair<std::wstring, std::set<std::wstring>>& item) {
        std::wstring name = item.first;
        std::transform(name.begin(), name.end(), name.begin(), tolower);
        std::for_each(item.second.begin(), item.second.end(), [&](const std::wstring& v) {
            std::wstring value(v);
            std::transform(value.begin(), value.end(), value.begin(), tolower);
            wcsncpy_s(p, size, name.c_str(), _TRUNCATE);
            p += name.length();
            *p = L'=';
            p += 1;
            wcsncpy_s(p, size, value.c_str(), _TRUNCATE);
            p += value.length();
            p += 1;  // L'\0'
        });
    });

    return std::move(data);
}

std::vector<unsigned char> nxl_file::create_tags_buffer_json(const TagMapType& tags)
{
    // calculate total size
    NX::web::json::value jtags = NX::web::json::value::object();

    std::for_each(tags.begin(), tags.end(), [&](const std::pair<std::wstring, std::set<std::wstring>>& item) {
        std::wstring name = item.first;
        std::transform(name.begin(), name.end(), name.begin(), tolower);
        std::vector<NX::web::json::value> value_array;
        std::for_each(item.second.begin(), item.second.end(), [&](const std::wstring& v) {
            std::wstring value(v);
            std::transform(value.begin(), value.end(), value.begin(), tolower);
            value_array.push_back(NX::web::json::value::string(value));
        });
        jtags[name] = NX::web::json::value::array(value_array);
    });

    std::wstring ws = jtags.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> data;
    data.resize(s.length() + 1, 0);
    memcpy(&data[0], s.c_str(), s.length());
    return std::move(data);
}

std::wstring nxl_file::get_temp_file_name(const std::wstring& file_or_dir)
{
    std::wstring temp_file;
    std::wstring dir;
    unsigned long attributes = GetFileAttributesW(file_or_dir.c_str());
    if (attribute_on(attributes, FILE_ATTRIBUTE_DIRECTORY)) {
        dir = file_or_dir;
    }
    else {
        auto pos = file_or_dir.find_last_of(L'\\');
        if (pos == std::wstring::npos) {
            throw std::exception("invalid full file path");
        }
        dir = file_or_dir.substr(0, pos);
    }
    if (0 == GetTempFileNameW(dir.c_str(), L"NXL", 0, nudf::string::tempstr<wchar_t>(temp_file, MAX_PATH))) {
        throw std::exception("fail to generate temp file name");
    }
    return std::move(temp_file);
}


void nxl_file::add_section(const std::vector<std::pair<std::wstring, unsigned long>>& sections)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }
    if (read_only()) {
        throw std::exception("file is opened in read only mode");
    }
}

void nxl_file::remove_section(const std::vector<std::pair<std::wstring, unsigned long>>& sections)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }
    if (read_only()) {
        throw std::exception("file is opened in read only mode");
    }
}

std::vector<unsigned char> nxl_file::get_section_data(const std::wstring& name, bool* validated)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }

    auto pos = std::find_if(header().sections().begin(), header().sections().end(), [&](const NX::NXL::nxl_section& s)->bool {
        return (0 == _wcsicmp(name.c_str(), s.name().c_str()));
    });
    if (pos == header().sections().end()) {
        throw std::exception("section not exist");
    }

    std::vector<unsigned char> data;
    data.resize((*pos).size(), 0);

    if (INVALID_SET_FILE_POINTER == SetFilePointer(_h, (*pos).offset(), NULL, FILE_BEGIN)) {
        throw std::exception("fail to move fp to begining of the section");
    }

    unsigned long bytes_read = 0;
    if (!::ReadFile(_h, &data[0], (unsigned long)data.size(), &bytes_read, NULL)) {
        data.clear();
        throw std::exception("fail to read section data");
    }
    if(bytes_read != (unsigned long)data.size()) {
        data.clear();
        throw std::exception("fail to read all the section data");
    }

    if (NULL != validated) {
        unsigned long real_checksum = nudf::crypto::ToCrc32(0, data.data(), (unsigned long)data.size());
        *validated = ((*pos).checksum() == real_checksum) ? true : false;
    }

    return std::move(data);
}

void nxl_file::set_section_data(const std::wstring& name, const std::vector<unsigned char>& data)
{
    set_section_data(name, data.empty() ? NULL : data.data(), (unsigned long)data.size());
}
void nxl_file::set_section_data(const std::wstring& name, const unsigned char* data, unsigned long size)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }
    if (read_only()) {
        throw std::exception("file is opened in read only mode");
    }
        
    int section_id = 0;
    auto pos = std::find_if(_header.sections().begin(), _header.sections().end(), [&](NX::NXL::nxl_section& s)->bool {

        section_id++;

        if (0 != _wcsicmp(name.c_str(), s.name().c_str())) {
            return false;
        }

        if (size > s.size()) {
            throw std::exception("data size is bigger than section size)");
        }

        std::vector<unsigned char> vdata;
        vdata.resize(s.size(), 0);
        if (NULL != data && 0 != size) {
            memcpy(&vdata[0], data, size);
        }

        if (INVALID_SET_FILE_POINTER == SetFilePointer(_h, s.offset(), NULL, FILE_BEGIN)) {
            throw std::exception("fail to move fp to begining of the section data");
        }

        unsigned long bytes_written = 0;
        if (!::WriteFile(_h, vdata.data(), (unsigned long)vdata.size(), &bytes_written, NULL)) {
            throw std::exception("fail to write section data");
        }
        if (bytes_written != (unsigned long)vdata.size()) {
            throw std::exception("fail to write all the section data");
        }
        unsigned long checksum = nudf::crypto::ToCrc32(0, vdata.data(), (unsigned long)vdata.size());
        s.set_checksum(checksum);

        // update section table
        unsigned long offset = FIELD_OFFSET(NXL_HEADER, Sections.Sections);
        offset += sizeof(NXL_SECTION) * (section_id - 1);
        offset += FIELD_OFFSET(NXL_SECTION, Checksum);
        if (INVALID_SET_FILE_POINTER == SetFilePointer(_h, offset, NULL, FILE_BEGIN)) {
            throw std::exception("fail to move fp to begining of the section checksum");
        }
        if (!::WriteFile(_h, &checksum, (unsigned long)sizeof(unsigned long), &bytes_written, NULL)) {
            throw std::exception("fail to write section checksum");
        }
        update_section_table_checksum();

        // we found it
        return true;
    });
    if (pos == header().sections().end()) {
        throw std::exception("section not exist");
    }
}

nxl_file::AttributeMapType nxl_file::get_nxl_attributes(bool* validated)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }

    AttributeMapType attributes;

    std::vector<unsigned char> data = get_section_data(NXL_SECTION_ATTRIBUTESW, validated);
    if (data.empty()) {
        return attributes;
    }

    if (data[0] == '{' && data[1] != 0) {
        assert(0 == data[data.size() - 1]);
        if (0 != data[data.size() - 1]) {
            data[data.size() - 1] = 0;
        }
        std::string s((const char*)data.data());
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        assert(v.is_object());
        const NX::web::json::object& obj = v.as_object();
        std::for_each(obj.begin(), obj.end(), [&](const std::pair<std::wstring, NX::web::json::value>& it) {
            std::wstring name = it.first;
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            attributes[name] = it.second.as_string();
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
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            attributes[name] = value;
        }
    }

    return attributes;
}

void nxl_file::set_nxl_attributes(const nxl_file::AttributeMapType& attributes)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }
    if (read_only()) {
        throw std::exception("file is opened in read only mode");
    }

    NX::web::json::value v = NX::web::json::value::object();
    std::for_each(attributes.begin(), attributes.end(), [&](const std::pair<std::wstring,std::wstring>& item) {
        std::wstring name = item.first;
        std::wstring value = item.second;
        std::transform(name.begin(), name.end(), name.begin(), tolower);
        std::transform(value.begin(), value.end(), value.begin(), tolower);
        v[name] = NX::web::json::value::string(value);
    });

    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    set_section_data(NXL_SECTION_ATTRIBUTESW, (const unsigned char*)((0 == s.length()) ? NULL : s.c_str()), (unsigned long)(0 == s.length() ? 0 : (s.length()+1)));
}

bool nxl_file::is_remote_eval(bool* validated)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }
    std::vector<unsigned char> data = get_section_data(NXL_SECTION_TEMPLATESW, validated);
    if (data.empty() || 0 == data[0]) {
        return false;
    }
    std::string s((const char*)data.data());
    std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
    NX::web::json::value v = NX::web::json::value::parse(ws);
    if (v.is_object() && v.has_field(L"type") && v[L"type"].is_string()) {
        std::wstring type = v[L"type"].as_string();
        return (0 == _wcsicmp(type.c_str(), L"remote only"));
    }

    return false;
}

void nxl_file::set_remote_eval(bool b)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }
    if (read_only()) {
        throw std::exception("file is opened in read only mode");
    }

    static const std::string remote_eval_json_value("{\"type\": \"remote only\"}");
    set_section_data(NXL_SECTION_TEMPLATESW, (const unsigned char*)(b ? remote_eval_json_value.c_str() : NULL), b ? ((unsigned long)remote_eval_json_value.length()) : 0);
}

nxl_file::TagMapType nxl_file::get_nxl_tags(bool* validated)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }

    if (!opened()) {
        throw std::exception("file is not opened");
    }

    TagMapType tags;

    std::vector<unsigned char> data = get_section_data(NXL_SECTION_TAGSW, validated);
    if (data.empty()) {
        return tags;
    }

    if (data[0] == '{' && data[1] != 0) {
        assert(0 == data[data.size() - 1]);
        if (0 != data[data.size() - 1]) {
            data[data.size() - 1] = 0;
        }
        std::string s((const char*)data.data());
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        assert(v.is_object());
        const NX::web::json::object& obj = v.as_object();
        std::for_each(obj.begin(), obj.end(), [&](const std::pair<std::wstring, NX::web::json::value>& it) {
            const NX::web::json::array& value_array = it.second.as_array();
            std::wstring name = it.first;
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            std::for_each(value_array.begin(), value_array.end(), [&](const NX::web::json::value& va) {
                std::wstring value = va.as_string();
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                tags[name].insert(value);
            });
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
            std::transform(name.begin(), name.end(), name.begin(), tolower);
            std::transform(value.begin(), value.end(), value.begin(), tolower);
            tags[name].insert(value);
        }
    }

    return tags;

    return TagMapType();
}

void nxl_file::set_nxl_tags(const nxl_file::TagMapType& tags)
{
    if (!opened()) {
        throw std::exception("file is not opened");
    }
    if (read_only()) {
        throw std::exception("file is opened in read only mode");
    }

    std::vector<unsigned char> data = create_tags_buffer_legacy(tags);
    set_section_data(NXL_SECTION_TAGSW, data);
}


std::wstring nxl_file::find_nxl_attribute(const AttributeMapType& attributes, const std::wstring& name) noexcept
{
    std::wstring low_name(name);
    std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
    auto pos = attributes.find(low_name);
    if (pos == attributes.end()) {
        return std::wstring();
    }
    return (*pos).second;
}

void nxl_file::set_nxl_attribute(AttributeMapType& attributes, const std::wstring& name, const std::wstring& value) noexcept
{
    std::wstring low_name(name);
    std::wstring low_value(value);
    std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
    std::transform(low_value.begin(), low_value.end(), low_value.begin(), tolower);
    attributes[low_name] = low_value;
}

bool nxl_file::remove_nxl_attribute(AttributeMapType& attributes, const std::wstring& name) noexcept
{
    std::wstring low_name(name);
    std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
    auto pos = attributes.find(low_name);
    if (pos == attributes.end()) {
        return false;
    }
    attributes.erase(pos);
    return true;
}

nxl_file::TagValueMapType nxl_file::find_nxl_tags(const TagMapType& tags, const std::wstring& name) noexcept
{
    std::wstring low_name(name);
    std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
    auto pos = tags.find(low_name);
    if (pos == tags.end()) {
        return TagValueMapType();
    }
    return (*pos).second;
}
void nxl_file::set_nxl_tag(TagMapType& tags, const std::wstring& name, const std::wstring& value) noexcept
{
    std::wstring low_name(name);
    std::wstring low_value(value);
    std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
    std::transform(low_value.begin(), low_value.end(), low_value.begin(), tolower);
    tags[low_name].insert(low_value);
}
bool nxl_file::remove_nxl_tag(TagMapType& tags, const std::wstring& name) noexcept
{
    std::wstring low_name(name);
    std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
    auto pos = tags.find(low_name);
    if (pos == tags.end()) {
        return false;
    }
    tags.erase(pos);
    return true;
}
bool nxl_file::remove_nxl_tag(TagMapType& tags, const std::wstring& name, const TagValueMapType& values) noexcept
{
    bool removed_any = false;

    std::wstring low_name(name);
    std::transform(low_name.begin(), low_name.end(), low_name.begin(), tolower);
    auto pos = tags.find(low_name);
    if (pos == tags.end()) {
        return false;
    }

    TagValueMapType& vs = tags[low_name];

    for (auto it = values.begin(); it != values.end(); ++it) {
        if (vs.empty()) {
            break;
        }

        std::wstring low_value(*it);
        std::transform(low_value.begin(), low_value.end(), low_value.begin(), tolower);
        auto vpos = vs.find(low_value);
        if (vpos == vs.end()) {
            continue;
        }
        vs.erase(vpos);
        removed_any = true;
    }

    return removed_any;
}
