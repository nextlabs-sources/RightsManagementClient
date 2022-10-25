

#ifndef __NXSERV_NXL_FILE_HPP__
#define __NXSERV_NXL_FILE_HPP__


#include <string>
#include <map>
#include <vector>


namespace NX {
namespace NXL {

class nxl_section
{
public:
    nxl_section()
        : _offset(0), _size(0), _checksum(0) {}
    nxl_section(const std::wstring& name, unsigned long offset, unsigned long size, unsigned long checksum)
        : _name(name), _offset(offset), _size(size), _checksum(checksum) {}
    virtual ~nxl_section() {}

    inline const std::wstring& name() const noexcept { return _name; }
    inline unsigned long offset() const noexcept { return _offset; }
    inline unsigned long size() const noexcept { return _size; }
    inline unsigned long checksum() const noexcept { return _checksum; }

    inline void set_checksum(unsigned long checksum) noexcept { _checksum = checksum; }

    inline bool empty() const noexcept { return _name.empty(); }
    inline void clear() noexcept
    {
        _name.clear();
        _offset = 0;
        _size = 0;
        _checksum = 0;
    }

    nxl_section& operator = (const nxl_section& other)
    {
        if (this != &other) {
            _name = other.name();
            _offset = other.offset();
            _size = other.size();
            _checksum = other.checksum();
        }
        return *this;
    }

private:
    std::wstring    _name;
    unsigned long   _offset;
    unsigned long   _size;
    unsigned long   _checksum;
};

class nxl_key_id
{
public:
    nxl_key_id();
    nxl_key_id(unsigned long algorithm, const unsigned char* id, unsigned long size);
    nxl_key_id(unsigned long algorithm, const std::vector<unsigned char>& id);
    virtual ~nxl_key_id();

    inline unsigned long algorithm() const noexcept { return _algorithm; }
    inline const unsigned char* id() const noexcept { return _id.empty() ? NULL : _id.data(); }
    inline unsigned long size() const noexcept { return (unsigned long)_id.size(); }

    inline bool empty() const noexcept { return (0 == size()); }
    inline void clear() noexcept { _algorithm = 0; _id.clear(); }

    nxl_key_id& operator = (const  nxl_key_id& other);

private:
    unsigned long   _algorithm;
    std::vector<unsigned char> _id;
};

class nxl_header
{
public:
    nxl_header();
    virtual ~nxl_header();

    void load(const std::wstring& file);
    void load(HANDLE h);
    void clear() noexcept;

    void validate_sections(const std::vector<unsigned char>& key, HANDLE h = INVALID_HANDLE_VALUE /*optional*/);
    void validate_section(const std::wstring& name, HANDLE h = INVALID_HANDLE_VALUE /*optional*/);
    void validate_section(const nxl_section& sc, HANDLE h);

    inline bool empty() const noexcept { return (0 == _version || _thumbprint.empty()); }

    inline const std::wstring& message() const noexcept { return _message; }
    inline const std::wstring& thumbprint() const noexcept { return _thumbprint; }
    inline unsigned long version() const noexcept { return _version; }
    inline unsigned long flags() const noexcept { return _flags; }
    inline unsigned long alignment() const noexcept { return _alignment; }
    inline unsigned long content_offset() const noexcept { return _content_offset; }
    inline unsigned long algorithm() const noexcept { return _algorithm; }
    inline unsigned long cbc_size() const noexcept { return _cbc_size; }
    inline const nxl_key_id& primary_key_id() const noexcept { return _primary_key_id; }
    inline const nxl_key_id& recovery_key_id() const noexcept { return _recovery_key_id; }

    bool is_content_key_aes128() const noexcept;
    bool is_content_key_aes256() const noexcept;
    bool is_primary_key_aes128() const noexcept;
    bool is_primary_key_aes256() const noexcept;
    inline bool is_primary_key_aes() const noexcept { return (is_primary_key_aes128() || is_primary_key_aes256()); }
    inline bool has_recovery_key() const noexcept { return (!_recovery_key_id.empty()); }
    bool is_recovery_key_aes128() const noexcept;
    bool is_recovery_key_aes256() const noexcept;
    bool is_recovery_key_rsa1024() const noexcept;
    bool is_recovery_key_rsa2048() const noexcept;
    inline bool is_recovery_key_aes() const noexcept { return (is_recovery_key_aes128() || is_recovery_key_aes256()); }
    inline bool is_recovery_key_rsa() const noexcept { return (is_recovery_key_rsa1024() || is_recovery_key_rsa2048()); }

    std::vector<unsigned char> decrypt_content_key(const std::vector<unsigned char>& key) const noexcept;
    std::vector<unsigned char> recovery_content_key(const std::vector<unsigned char>& key) const noexcept;
    unsigned long decrypt_sections_checksum(const std::vector<unsigned char>& key) const noexcept;
    void update_sections_checksum(const std::vector<unsigned char>& key, HANDLE h);
    unsigned long calc_sections_checksum() const noexcept;

    inline const std::vector<nxl_section>& sections() const noexcept { return _sections; }

    // set section data
    void get_section_data(const std::wstring& name, std::vector<unsigned char>& data, HANDLE h, bool* validated = NULL);
    void get_section_data(const nxl_section& sc, std::vector<unsigned char>& data, HANDLE h, bool* validated = NULL);
    void set_section_data(const std::wstring& name, const std::vector<unsigned char>& data, const std::vector<unsigned char>& key, HANDLE h);
    void set_section_data(nxl_section& sc, const std::vector<unsigned char>& data, const std::vector<unsigned char>& key, HANDLE h);

    // get section data
    std::map<std::wstring, std::wstring> load_section_attributes(HANDLE h, bool* validated = NULL);
    std::wstring load_section_templates(HANDLE h, bool* validated = NULL);
    std::multimap<std::wstring, std::wstring> load_section_tags(HANDLE h, bool* validated = NULL);


private:
    std::wstring    _message;
    std::wstring    _thumbprint;
    unsigned long   _version;
    unsigned long   _flags;
    unsigned long   _alignment;
    unsigned long   _content_offset;
    unsigned long   _algorithm;
    unsigned long   _cbc_size;
    nxl_key_id      _primary_key_id;
    std::vector<unsigned char>  _primary_content_key;
    nxl_key_id      _recovery_key_id;
    std::vector<unsigned char>  _recovery_content_key;
    __int64         _content_size;
    __int64         _allocation_size;
    std::vector<nxl_section>    _sections;
    std::vector<unsigned char>  _sections_checksum;
};


}   // namespace NXL
}   // namespace NX



#endif