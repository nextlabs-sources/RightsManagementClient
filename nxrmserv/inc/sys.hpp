

#ifndef __NX_SYSTEM_HPP__
#define __NX_SYSTEM_HPP__

#include <Windows.h>

#include <string>
#include <vector>



namespace NX {


typedef enum _CPUARCH {
    arch_unknown = 0,
    arch_i386,
    arch_amd64
} CPUARCH;


namespace sys {

void trim_working_set();

namespace os
{
bool is_server() noexcept;
bool is_workstation() noexcept;
bool is_32bits() noexcept;
bool is_64bits() noexcept;

ULONG os_type() noexcept;
LCID locale_id() noexcept;
LANGID language_id() noexcept;
LANGID ui_language_id() noexcept;

const std::wstring& os_name() noexcept;
const std::wstring& os_type_name() noexcept;
const std::wstring& os_locale_name() noexcept;

bool is_win_2000(bool or_later=false) noexcept;
bool is_win_xp(bool or_later=false) noexcept;
bool is_win_xp_64pro(bool or_later=false) noexcept;
bool is_win_home_server(bool or_later=false) noexcept;
bool is_win_2003(bool or_later=false) noexcept;
bool is_win_2003r2(bool or_later=false) noexcept;
bool is_win_vista(bool or_later=false) noexcept;
bool is_win_2008(bool or_later=false) noexcept;
bool is_win_7(bool or_later=false) noexcept;
bool is_win_2008r2(bool or_later=false) noexcept;
bool is_win_8(bool or_later=false) noexcept;
bool is_win_2012(bool or_later=false) noexcept;
bool is_win_81(bool or_later=false) noexcept;
bool is_win_2012r2(bool or_later=false) noexcept;
bool is_win_10(bool or_later=false) noexcept;
bool is_win_2016(bool or_later=false) noexcept;
}   // namespace os

namespace hardware
{

class net_adapter
{
public:
    net_adapter() : _if_type(0), _ipv4_enabled(0), _ipv6_enabled(0), _oper_status(0), _trans_link_speed(0), _recv_link_speed(0) {}
    net_adapter(_In_ void* address);
    ~net_adapter() {}

    net_adapter& operator = (const net_adapter& other)
    {
        if (this != &other) {
            _name = other.name();
            _friendly_name = other.friendly_name();
            _description = other.description();
            _phyaddr = other.physical_address();
            _if_type = other.if_type();
            _if_type_name = other.if_type_name();
            _ipv4_enabled = other.ipv4_enabled();
            _ipv6_enabled = other.ipv6_enabled();
            _oper_status = other.oper_status();
            _oper_status_name = other.oper_status_name();
            _trans_link_speed = other.transmit_link_speed();
            _recv_link_speed = other.receive_link_speed();
            _ipv4_addresses = other.ipv4_addresses();
            _ipv6_addresses = other.ipv6_addresses();
        }
        return *this;
    }

    inline const std::wstring& name() const noexcept { return _name; }
    inline const std::wstring& friendly_name() const noexcept { return _friendly_name; }
    inline const std::wstring& description() const noexcept { return _description; }
    inline const std::wstring& physical_address() const noexcept { return _phyaddr; }
    inline unsigned long if_type() const noexcept { return _if_type; }
    inline const std::wstring& if_type_name() const noexcept { return _if_type_name; }
    inline unsigned long ipv4_enabled() const noexcept { return _ipv4_enabled; }
    inline unsigned long ipv6_enabled() const noexcept { return _ipv6_enabled; }
    inline unsigned long oper_status() const noexcept { return _oper_status; }
    inline const std::wstring& oper_status_name() const noexcept { return _oper_status_name; }
    inline unsigned __int64 transmit_link_speed() const noexcept { return _trans_link_speed; }
    inline unsigned __int64 receive_link_speed() const noexcept { return _recv_link_speed; }
    inline const std::vector<std::wstring>& ipv4_addresses() const noexcept { return _ipv4_addresses; }
    inline const std::vector<std::wstring>& ipv6_addresses() const noexcept { return _ipv6_addresses; }

    bool is_connected() const noexcept;

private:
    std::wstring        _name;
    std::wstring        _friendly_name;
    std::wstring        _description;
    std::wstring        _phyaddr;
    unsigned long       _if_type;
    std::wstring        _if_type_name;
    unsigned long       _ipv4_enabled;
    unsigned long       _ipv6_enabled;
    unsigned long       _oper_status;
    std::wstring        _oper_status_name;
    unsigned __int64    _trans_link_speed;
    unsigned __int64    _recv_link_speed;
    std::vector<std::wstring>   _ipv4_addresses;
    std::vector<std::wstring>   _ipv6_addresses;
};

class disk
{
public:
    disk() : _type(0), _total(0), _available_free(0), _total_free(0)
    {
    }
    disk(const std::wstring& name, unsigned int type, unsigned __int64 total, unsigned __int64 available_free, unsigned __int64 total_free) : 
        _name(name), _type(type), _total(total), _available_free(available_free), _total_free(total_free)
    {
    }
    ~disk()
    {
    }

    disk& operator = (const disk& other) noexcept
    {
        if (this != &other) {
            _name = other.name();
            _type = other.type();
            _total = other.total();
            _available_free = other.available_free();
            _total_free = other.total_free();
        }
        return *this;
    }

    inline const std::wstring& name() const noexcept { return _name; }
    inline unsigned int type() const noexcept { return _type; }
    inline unsigned __int64 total() const noexcept { return _total; }
    inline unsigned __int64 available_free() const noexcept { return _available_free; }
    inline unsigned __int64 total_free() const noexcept { return _total_free; }

    std::wstring type_string() const noexcept;
    void clear() noexcept;
    bool empty() const noexcept;
    bool is_fixed() const noexcept;
    bool is_removable() const noexcept;
    bool is_remote() const noexcept;
    bool is_cdrom() const noexcept;
    bool is_ramdiak() const noexcept;
    bool load(const wchar_t drive) noexcept;

private:
    std::wstring        _name;
    unsigned int        _type;
    unsigned __int64    _total;
    unsigned __int64    _available_free;
    unsigned __int64    _total_free;
};

const std::wstring& cpu_brand() noexcept;
unsigned long memory_in_mb() noexcept;
unsigned __int64 memory_in_bytes() noexcept;
void get_net_adapters(std::vector<net_adapter>& adapters);
std::vector<std::wstring> get_active_ipv4();
void get_disks(std::vector<disk>& disks);
}   // namespace hardware

}   // namespace sys

class version
{
public:
    version();
    version(const std::wstring& v);
    version(unsigned long major, unsigned long minor, unsigned long build);
    virtual ~version();

    version& operator = (const version& v);
    bool operator == (const version& v);
    bool operator == (const std::wstring& v);
    
    void clear() noexcept;

    inline const std::wstring& version_str() const noexcept {return _str;}
    inline unsigned long version_major() const noexcept {return _major;}
    inline unsigned long version_minor() const noexcept {return _minor;}
    inline unsigned long version_build() const noexcept {return _build;}

    inline bool empty() const noexcept {return _str.empty();}

private:
    void load_string(const std::wstring& v) noexcept;
    bool vdalidate_string(const std::wstring& v) noexcept;

private:
    std::wstring    _str;
    unsigned long   _major;
    unsigned long   _minor;
    unsigned long   _build;
};

class module
{
public:
    module();
    module(const std::wstring& module_path, bool lowercase=false);
    module(HMODULE mod, bool lowercase=false);
    virtual ~module();

    module& operator = (const module& m);
    module& operator = (const std::wstring& module_path);

    void clear() noexcept;
    void to_lower() noexcept;
    void to_upper() noexcept;

    inline const std::wstring& full_path() const noexcept {return _full_path;}
    inline const std::wstring& parent_dir() const noexcept {return _parent_dir;}
    inline const std::wstring& image_name() const noexcept {return _image_name;}

    inline bool empty() const noexcept {return _full_path.empty();}
    inline bool has_path() const noexcept {return !_parent_dir.empty();}

    static void parse_filepath(const std::wstring& fullpath, std::wstring& parent_dir, std::wstring& final_part);
    static void parse_filename(const std::wstring& filename, std::wstring& first_part, std::wstring& suffix);

private:
    std::wstring    _full_path;
    std::wstring    _parent_dir;
    std::wstring    _image_name;
};

class language
{
public:
    language();
    language(unsigned long id);
    ~language();

    language& operator = (const language& lg);
    language& operator = (unsigned long id);

    void clear();

    static std::wstring id_to_name(unsigned long id);
    static const unsigned long default_lang_id;
    static const std::wstring default_lang_name;

    inline bool empty() const noexcept {return (_name.empty() && 0==_id);}
    inline unsigned long id() const noexcept {return _id;}
    inline const std::wstring& name() const noexcept {return _name;}

private:

private:
    unsigned long   _id;
    std::wstring    _name;
};

class product
{
public:
    product();
    product(const std::wstring& name, NX::CPUARCH arch);
    virtual ~product();

    product& operator = (const product& p);

    void clear();
    bool load(const std::wstring& guid, NX::CPUARCH arch) noexcept;

    inline bool empty() const noexcept {return _guid.empty();}
    inline bool has_name() const noexcept {return !_name.empty();}
    inline bool installed() const noexcept {return !_install_root.empty();}

    inline const std::wstring& guid() const noexcept {return _guid;}
    inline const std::wstring& name() const noexcept {return _name;}
    inline const NX::version& version() const noexcept {return _version;}
    inline const std::wstring& publisher() const noexcept {return _publisher;}
    inline const NX::language& language() const noexcept {return _language;}
    inline const std::wstring& install_root() const noexcept {return _install_root;}
    inline NX::CPUARCH architecture() const noexcept {return _arch;}


protected:
    bool find(const std::wstring& name, NX::CPUARCH arch) noexcept;

private:
    std::wstring    _guid;
    std::wstring    _name;
    NX::version     _version;
    std::wstring    _publisher;
    NX::language    _language;
    std::wstring    _install_root;
    CPUARCH         _arch;
};


class host
{
public:
    host();
    ~host(){}

    inline host& operator = (const host& other) noexcept
    {
        if(this != &other) {
            _name = other.name();
            _fqdn = other.fully_qualified_domain_name();
        }
        return *this;
    }

    inline const std::wstring& name() const noexcept {return _name;}
    inline const std::wstring& fully_qualified_domain_name() const noexcept {return _fqdn;}

    inline bool empty() const noexcept {return _name.empty();}
    inline bool in_domain() const noexcept {return !_fqdn.empty();}
    inline void clear() noexcept {_name.clear();_fqdn.clear();}

private:
    std::wstring    _name;
    std::wstring    _fqdn;
};


class logged_on_users
{
public:
    logged_on_users();
    ~logged_on_users() {}

    inline const std::vector<std::pair<std::wstring, std::wstring>>& users() const noexcept { return _users; }

private:
    std::vector<std::pair<std::wstring, std::wstring>> _users;
};

class pe_cert
{
public:
    pe_cert() {}
    pe_cert(const std::wstring& file) { load(file); }
    ~pe_cert() {}

    inline bool is_valid() const throw() { return (!_subject.empty()); }
    inline const std::wstring& subject() const throw() { return _subject; }
    inline const std::wstring& issuer() const throw() { return _issuer; }
    inline const std::wstring& serial() const throw() { return _serial; }
    inline const std::vector<UCHAR>& thumbprint() const throw() { return _thumbprint; }
    inline const std::wstring& thumbprint_alg() const throw() { return _thumbprint_alg; }
    inline const std::wstring& signature_alg() const throw() { return _signature_alg; }
    inline const SYSTEMTIME& valid_from() const throw() { return _validfrom; }
    inline const SYSTEMTIME& valid_thru() const throw() { return _validthru; }

    pe_cert& operator = (const pe_cert& cert) throw()
    {
        if (this != &cert) {
            _subject = cert.subject();
            _issuer = cert.issuer();
            _serial = cert.serial();
            _thumbprint = cert.thumbprint();
            _thumbprint_alg = cert.thumbprint_alg();
            _signature_alg = cert.signature_alg();
            memcpy(&_validfrom, &cert.valid_from(), sizeof(_validfrom));
            memcpy(&_validthru, &cert.valid_thru(), sizeof(_validthru));
        }
        return *this;
    }

    bool load(const std::wstring& file) noexcept;
    void clear() throw();


private:
    std::wstring        _subject;
    std::wstring        _issuer;
    std::wstring        _serial;
    std::vector<unsigned char>  _thumbprint;
    std::wstring        _thumbprint_alg;
    std::wstring        _signature_alg;
    SYSTEMTIME          _validfrom;
    SYSTEMTIME          _validthru;
};

class pe_file
{
public:
    pe_file();
    pe_file(const std::wstring& file);
    virtual ~pe_file();

    void load(const std::wstring& file) noexcept;
    void clear() noexcept;
    pe_file& operator = (const pe_file& pe) noexcept;

    inline bool IsValid() const noexcept { return (0x00004550 == _nt_header.Signature); }
    inline const IMAGE_NT_HEADERS* nt_headers() const noexcept { return (&_nt_header); }
    inline const IMAGE_FILE_HEADER* file_header() const noexcept { return (&_nt_header.FileHeader); }
    inline const IMAGE_OPTIONAL_HEADER* optional_header() const noexcept { return (&_nt_header.OptionalHeader); }
    inline const pe_cert& cert() const noexcept { return _cert; }

    inline bool is_x86() const noexcept { return (IsValid() && _nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_I386); }
    inline bool ix_x64() const noexcept { return (IsValid() && _nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64); }
    inline bool is_ia64() const noexcept { return (IsValid() && _nt_header.FileHeader.Machine == IMAGE_FILE_MACHINE_IA64); }

    inline bool is_exe() const noexcept { return (IsValid() && (0 != (_nt_header.FileHeader.Characteristics == IMAGE_FILE_EXECUTABLE_IMAGE))); }
    inline bool is_dll() const noexcept { return (IsValid() && (0 != (_nt_header.FileHeader.Characteristics == IMAGE_FILE_DLL))); }
    inline bool is_sys() const noexcept { return (IsValid() && (0 != (_nt_header.FileHeader.Characteristics == IMAGE_FILE_SYSTEM))); }

    inline unsigned long checksum() const noexcept { return _nt_header.OptionalHeader.CheckSum; }

private:
    IMAGE_NT_HEADERS    _nt_header;
    pe_cert             _cert;
};

class process_info
{
public:
    process_info() : _session_id(-1) {}
    process_info(unsigned long session_id, const std::wstring& image, const std::wstring& publisher) : _session_id(session_id), _image(image), _publisher(publisher) {}
    process_info(unsigned long process_id);
    process_info(unsigned long session_id, const std::wstring& image);
    virtual ~process_info() {}

    inline bool is_session_id_valid() const noexcept {return ((unsigned long)-1 != _session_id); }
    inline unsigned long session_id() const noexcept { return _session_id; }
    inline const std::wstring& image() const noexcept { return _image; }
    inline const std::wstring& publisher() const noexcept { return _publisher; }
    inline process_info& operator = (const process_info& other)
    {
        if (this != &other) {
            _image = other.image();
            _publisher = other.publisher();
        }
        return *this;
    }

    void clear() noexcept
    {
        _image.clear();
        _publisher.clear();
    }

    bool empty() const noexcept
    {
        return _image.empty();
    }

protected:
    std::wstring get_publisher(const std::wstring& image);

private:
    unsigned long   _session_id;
    std::wstring    _image;
    std::wstring    _publisher;
};


}   // namespace NX



#endif