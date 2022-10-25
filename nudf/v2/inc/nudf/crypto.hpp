

#pragma once
#ifndef __NUDF_CRYPTO_HPP__
#define __NUDF_CRYPTO_HPP__

#include <Bcrypt.h>
#include <Wincrypt.h>

#include <string>
#include <vector>
#include <algorithm>
#include <memory>

namespace NX {

    
namespace crypto {



class data_blob
{
public:
    data_blob() {}
    data_blob(const std::vector<unsigned char>& buf) : _buf(buf) {}
    virtual ~data_blob() {}

    void clear() { _buf.clear(); }
    inline bool empty() const { return _buf.empty(); }
    const std::vector<unsigned char>& buffer() const { return _buf; }
    std::vector<unsigned char>& buffer() { return _buf; }
    data_blob& operator = (const data_blob& other)
    {
        if (this != &other) {
            _buf = other.buffer();
        }
        return *this;
    }

private:
    std::vector<unsigned char> _buf;
};

class handle_object
{
public:
    handle_object() {}
    handle_object(HANDLE h) : _h(h) {}
    virtual ~handle_object() { close(); }

    inline bool empty() const { return (NULL == _h); }
    inline operator HANDLE() const { return _h; }
    inline HANDLE attach(HANDLE h) { _h = h; return _h; }
    inline HANDLE detach() { HANDLE h = _h; _h = NULL; return h; }


protected:
    HANDLE handle() const { return _h; }
    virtual void close() { _h = NULL; }

private:
    HANDLE  _h;
private:
    // No copy allowed
    handle_object& operator = (const handle_object& other) { return *this; }
};

namespace CRYPT32 {
    
class crypt_provider : public handle_object
{
public:
    crypt_provider();
    crypt_provider(HCRYPTPROV h);
    virtual ~crypt_provider();

    inline operator HCRYPTPROV() const { return (HCRYPTPROV)handle(); }
    inline HCRYPTPROV attach(HCRYPTPROV h) { return (HCRYPTPROV)handle_object::attach((HANDLE)h); }
    inline HCRYPTPROV detach() { return (HCRYPTPROV)handle_object::detach(); }

    void acquire(const std::wstring& name, bool enhanced = true, bool always_open = true, bool machine_keyset = true, bool slient = true);
    virtual void close();
};

class crypt_provider_info : public data_blob
{
public:
    crypt_provider_info();
    crypt_provider_info(const std::vector<unsigned char>& buf);
    virtual ~crypt_provider_info();

    inline operator PCRYPT_KEY_PROV_INFO() { return empty() ? NULL : ((PCRYPT_KEY_PROV_INFO)buffer().data()); }
    inline PCRYPT_KEY_PROV_INFO operator ->() { return empty() ? NULL : ((PCRYPT_KEY_PROV_INFO)buffer().data()); }

    crypt_provider_info& operator = (const crypt_provider_info& other);
};

typedef struct _CRYPT32_RSAKEY_HEADER {
    PUBLICKEYSTRUC  pubkey_struc;
    RSAPUBKEY       pubkey;
} CRYPT32_RSAKEY_HEADER, *PCRYPT32_RSAKEY_HEADER;
typedef const CRYPT32_RSAKEY_HEADER* PCCRYPT32_RSAKEY_HEADER;

class public_key_blob : public data_blob
{
public:
    public_key_blob();
    public_key_blob(const std::vector<unsigned char>& buf);
    virtual ~public_key_blob();

    bool valid() const noexcept;
    PCCRYPT32_RSAKEY_HEADER rsa_header() const noexcept;
    const unsigned char* public_exponent() const noexcept;
    unsigned long public_exponent_length() const noexcept;
    const unsigned char* modulus() const noexcept;
    unsigned long modulus_length() const noexcept;

    public_key_blob& operator = (const std::vector<unsigned char>& buf);
};

class private_key_blob : public data_blob
{
public:
    private_key_blob();
    private_key_blob(const std::vector<unsigned char>& buf);
    virtual ~private_key_blob();

    bool valid() const noexcept;
    PCCRYPT32_RSAKEY_HEADER rsa_header() const noexcept;

    const unsigned char* public_exponent() const noexcept;
    unsigned long public_exponent_length() const noexcept;

    const unsigned char* modulus() const noexcept;
    unsigned long modulus_length() const noexcept;

    const unsigned char* prime1() const noexcept;
    unsigned long prime1_length() const noexcept;

    const unsigned char* prime2() const noexcept;
    unsigned long prime2_length() const noexcept;

    const unsigned char* exponent1() const noexcept;
    unsigned long exponent1_length() const noexcept;

    const unsigned char* exponent2() const noexcept;
    unsigned long exponent2_length() const noexcept;

    const unsigned char* coefficient() const noexcept;
    unsigned long coefficient_length() const noexcept;

    const unsigned char* private_exponent() const noexcept;
    unsigned long private_exponent_length() const noexcept;

    public_key_blob get_public_key() const noexcept;
};

class crypt_key : public handle_object
{
public:
    crypt_key();
    crypt_key(HCRYPTKEY h);
    virtual ~crypt_key();

    inline operator HCRYPTKEY() const { return (HCRYPTKEY)handle(); }
    inline HCRYPTKEY attach(HCRYPTKEY h) { return (HCRYPTKEY)handle_object::attach((HANDLE)h); }
    inline HCRYPTKEY detach() { return (HCRYPTKEY)handle_object::detach(); }
    virtual void close();
};

class public_key : public crypt_key
{
public:
    public_key();
    public_key(HCRYPTKEY h);
    virtual ~public_key();

    public_key_blob export_public_key() const;
};

class private_key : public crypt_key
{
public:
    private_key();
    private_key(HCRYPTKEY h);
    virtual ~private_key();

    void generate(const CRYPT32::crypt_provider& prov, unsigned long key_spec = AT_SIGNATURE, bool strong = true, bool exportable = true);

    private_key_blob export_private_key() const;
    public_key_blob export_public_key() const;
};

}   // namespace CRYPT32



namespace BCRYPT {


class crypt_provider : public handle_object
{
public:
    crypt_provider();
    crypt_provider(NCRYPT_PROV_HANDLE h);
    explicit crypt_provider(const std::wstring& alg_name, const std::wstring& impl_name, unsigned long flags);
    virtual ~crypt_provider();

    inline operator NCRYPT_PROV_HANDLE() const { return (NCRYPT_PROV_HANDLE)handle(); }
    inline NCRYPT_PROV_HANDLE attach(BCRYPT_ALG_HANDLE h) { return (NCRYPT_PROV_HANDLE)handle_object::attach((HANDLE)h); }
    inline NCRYPT_PROV_HANDLE detach() { return (NCRYPT_PROV_HANDLE)handle_object::detach(); }

    void open(const std::wstring& alg_name, const std::wstring& impl_name, unsigned long flags = 0);
    virtual void close();
};

class crypt_key : public handle_object
{
public:
    crypt_key();
    crypt_key(BCRYPT_KEY_HANDLE h);
    virtual ~crypt_key();


    inline operator BCRYPT_KEY_HANDLE() const { return (BCRYPT_KEY_HANDLE)handle(); }
    inline BCRYPT_KEY_HANDLE attach(BCRYPT_KEY_HANDLE h) { return (BCRYPT_KEY_HANDLE)handle_object::attach((HANDLE)h); }
    inline BCRYPT_KEY_HANDLE detach() { return (BCRYPT_KEY_HANDLE)handle_object::detach(); }

    virtual void close();
};

class crypt_hash : public handle_object
{
public:
    crypt_hash();
    crypt_hash(BCRYPT_HASH_HANDLE h);
    virtual ~crypt_hash();
    
    inline operator BCRYPT_HASH_HANDLE() const { return (BCRYPT_HASH_HANDLE)handle(); }
    inline BCRYPT_HASH_HANDLE attach(BCRYPT_HASH_HANDLE h) { handle_object::attach((HANDLE)h); }
    inline BCRYPT_HASH_HANDLE detach() { return (BCRYPT_HASH_HANDLE)handle_object::detach(); }

    virtual void close();
};

class crypt_key_aes : public crypt_key
{
public:
    crypt_key_aes();
    virtual ~crypt_key_aes();

    void import(const std::vector<unsigned char>& key);
    void import(const unsigned char* key, unsigned long size);
    std::vector<unsigned char> export_key() const;
    void generate(unsigned long bits_length = 256);
    virtual void close();

    inline unsigned long block_length() const { return _block_length; }

private:
    unsigned long   _block_length;
};

class public_key_blob : public data_blob
{
public:
    public_key_blob();
    public_key_blob(const std::vector<unsigned char>& buf);
    virtual ~public_key_blob();

    bool valid() const noexcept;
    const BCRYPT_RSAKEY_BLOB* rsa_header() const noexcept;
    const unsigned char* public_exponent() const noexcept;
    unsigned long public_exponent_length() const noexcept;
    const unsigned char* modulus() const noexcept;
    unsigned long modulus_length() const noexcept;
};

class private_key_blob : public data_blob
{
public:
    private_key_blob();
    private_key_blob(const std::vector<unsigned char>& buf);
    virtual ~private_key_blob();

    bool valid() const noexcept;
    bool full_private_key() const;

    const BCRYPT_RSAKEY_BLOB* rsa_header() const noexcept;

    const unsigned char* public_exponent() const noexcept;
    unsigned long public_exponent_length() const noexcept;

    const unsigned char* modulus() const noexcept;
    unsigned long modulus_length() const noexcept;

    const unsigned char* prime1() const noexcept;
    unsigned long prime1_length() const noexcept;

    const unsigned char* prime2() const noexcept;
    unsigned long prime2_length() const noexcept;

    const unsigned char* exponent1() const noexcept;
    unsigned long exponent1_length() const noexcept;

    const unsigned char* exponent2() const noexcept;
    unsigned long exponent2_length() const noexcept;

    const unsigned char* coefficient() const noexcept;
    unsigned long coefficient_length() const noexcept;

    const unsigned char* private_exponent() const noexcept;
    unsigned long private_exponent_length() const noexcept;

    BCRYPT::public_key_blob get_public_key() const noexcept;
};

class public_key : public crypt_key
{
public:
    public_key();
    public_key(BCRYPT_KEY_HANDLE h);
    virtual ~public_key();

    public_key_blob export_public_key() const;
};

class private_key : public crypt_key
{
public:
    private_key();
    private_key(BCRYPT_KEY_HANDLE h);
    virtual ~private_key();

    void generate(bool strong = true);

    private_key_blob export_private_key() const;
    public_key_blob export_public_key() const;
};


std::vector<unsigned char> aes_encrypt(const crypt_key_aes& h, const void* data, unsigned long size, unsigned __int64 ivec = 0, const unsigned long cbc_size = 512);
std::vector<unsigned char> aes_decrypt(const crypt_key_aes& h, const void* data, unsigned long size, unsigned __int64 ivec = 0, const unsigned long cbc_size = 512);

std::vector<unsigned char> md5_hash(const unsigned char* data, unsigned long size);
std::vector<unsigned char> sha1_hash(const unsigned char* data, unsigned long size);
std::vector<unsigned char> sha2_hash(const unsigned char* data, unsigned long size);

}   // namespace BCRYPT


}   // namespace crypto


namespace certificate {

class name_blob : public CERT_NAME_BLOB
{
public:
    name_blob();
    name_blob(const std::wstring& s);
    virtual ~name_blob();

    inline bool empty() { return (0 == this->cbData); }
    inline const std::vector<unsigned char>& buffer() const { return _buf; }

    void clear();
    void set_name(const std::wstring& s);

    name_blob& operator = (const name_blob& other);


private:
    std::vector<unsigned char> _buf;
};


class properties
{
public:
    properties();
    virtual ~properties();

    properties& operator = (const properties& other);
    void clear();

    inline bool empty() const { return _subject_name.empty(); }
    inline bool self_signed() const { return (_subject_name == _issuer_name); }

    inline const std::wstring& get_subject_name() const { return _subject_name; }
    inline const std::wstring& get_issuer_name() const { return _issuer_name; }
    inline const std::wstring& get_friendly_name() const { return _friendly_name; }
    inline const std::wstring& get_description() const { return _description; }
    inline const std::wstring& get_serial() const { return _serial; }
    inline const std::wstring& get_sign_algorithm() const { return _sign_algorithm; }
    inline const std::wstring& get_hash_algorithm() const { return _hash_algorithm; }
    inline const std::vector<unsigned char>& get_thumbprint() const { return _thumbprint; }
    inline const std::vector<unsigned char>& get_signature_hash() const { return _signature_hash; }
    inline const std::vector<unsigned char>& get_public_key() const { return _public_key; }
    inline const std::vector<unsigned char>& get_private_key() const { return _private_key; }
    inline const FILETIME& get_valid_from_time() const { return _valid_from; }
    inline const FILETIME& get_valid_through_time() const { return _valid_through; }

    inline void set_subject_name(const std::wstring& v) { _subject_name = v; }
    inline void set_issuer_name(const std::wstring& v) { _issuer_name = v; }
    inline void set_friendly_name(const std::wstring& v) { _friendly_name = v; }
    inline void set_description(const std::wstring& v) { _description = v; }
    inline void set_serial(const std::wstring& v) { _serial = v; }
    inline void set_sign_algorithm(const std::wstring& v) { _sign_algorithm = v; }
    inline void set_hash_algorithm(const std::wstring& v) { _hash_algorithm = v; }
    inline void set_thumbprint(const std::vector<unsigned char>& v) { _thumbprint = v; }
    inline void set_signature_hash(const std::vector<unsigned char>& v) { _signature_hash = v; }
    inline void set_public_key(const std::vector<unsigned char>& v) { _public_key = v; }
    inline void set_private_key(const std::vector<unsigned char>& v) { _private_key = v; }
    inline void set_valid_from(const FILETIME& v) { _valid_from = v; }
    inline void set_valid_through(const FILETIME& v) { _valid_through = v; }


private:
    std::wstring    _subject_name;
    std::wstring    _issuer_name;
    std::wstring    _friendly_name;
    std::wstring    _description;
    std::wstring    _serial;
    std::wstring    _sign_algorithm;
    std::wstring    _hash_algorithm;
    std::vector<unsigned char> _thumbprint;
    std::vector<unsigned char> _signature_hash;
    std::vector<unsigned char> _public_key;     // Key Blob
    std::vector<unsigned char> _private_key;    // Key Blob

    FILETIME        _valid_from;
    FILETIME        _valid_through;
};

class context
{
public:
    context();
    context(PCCERT_CONTEXT p);
    virtual ~context();


    inline operator PCCERT_CONTEXT() { return _p; }
    inline PCCERT_CONTEXT attach(PCCERT_CONTEXT p) { _p = p;  return _p; }
    inline PCCERT_CONTEXT detach() { PCCERT_CONTEXT p = _p;  _p = nullptr;  return p; }
    inline bool empty() const { return (nullptr != _p); }

    static std::shared_ptr<context> create(const std::wstring& subject, unsigned long key_spec, bool strong, context* issuer, unsigned short valid_years = 20);

    virtual void open(const unsigned char pb, unsigned long cb);
    virtual void open(const std::wstring& file);
    void clear();

    NX::certificate::properties get_all_properties();

    //
    // certificate properties
    //

    // --> provider/key/algorithm
    NX::crypto::CRYPT32::crypt_provider get_key_provider() const;
    NX::crypto::CRYPT32::crypt_provider_info get_key_provider_info() const;
    unsigned long get_key_spec() const;
    unsigned long get_public_key_length() const;
    std::vector<unsigned char> get_public_key_blob() const;
    NX::crypto::CRYPT32::public_key_blob get_public_key() const;
    std::vector<unsigned char> get_private_key_blob() const;
    NX::crypto::CRYPT32::private_key_blob get_private_key() const;
    bool has_private_key() const;
    bool is_self_signed() const;

    // --> cert information
    std::vector<unsigned char> get_serial_number() const;
    std::vector<unsigned char> get_thumb_print() const;
    std::wstring get_issuer_name() const;
    std::wstring get_subject_name() const;
    FILETIME get_date_stamp() const;
    std::wstring get_friendly_name() const;
    FILETIME get_start_time() const;
    FILETIME get_expire_time() const;
    std::wstring get_signature_algorithm() const;
    std::wstring get_hash_algorithm() const;
    std::vector<unsigned char> get_signature_hash() const;

protected:
    void set_property(unsigned long id, const void* pb, unsigned long cb);
    std::vector<unsigned char> get_property(unsigned long id) const;
    unsigned long get_property_size(unsigned long id) const;
    std::vector<unsigned char> decode_object(unsigned long encoding_type, const char* struc_type, const unsigned char* data, unsigned long data_size, unsigned long flags) const;

private:
    // No copy allowed
    context& operator = (const context& other) { return *this; }

private:
    PCCERT_CONTEXT  _p;
};

class store
{
public:
    virtual ~store();

    inline operator HCERTSTORE() { return _h; }
    inline bool opened() const { return (NULL != _h); }
    inline HCERTSTORE attach(HCERTSTORE h) { _h = h; return _h; }
    inline HCERTSTORE detach() { HCERTSTORE dh = _h; _h = NULL; return dh; }

    virtual void close();

protected:
    store();
    store(HCERTSTORE h);

private:
    // No copy is allowed
    store& operator = (const store& other) { return *this; }

private:
    HCERTSTORE  _h;
};

class system_store : public store
{
public:
    system_store();
    system_store(const std::wstring& location);
    virtual ~system_store();

    virtual void open(const std::wstring& location);

    static const std::wstring personal_folder;
    static const std::wstring trust_root_ca_folder;
    static const std::wstring intermediate_ca_folder;
    static const std::wstring trusted_people_folder;
    static const std::wstring trusted_publishers_folder;
    static const std::wstring untrusted_folder;
    static const std::wstring third_party_root_ca_folder;
};

class user_store : public system_store
{
public:
    user_store();
    user_store(const std::wstring& location);
    virtual ~user_store();
    virtual void open(const std::wstring& location);
};

class memory_store : public store
{
public:
    memory_store();
    virtual ~memory_store();
};

class file_store : public store
{
public:
    file_store();
    virtual ~file_store();

    virtual void open(const std::wstring& location, const std::wstring& password = std::wstring());
};

class pcks12_file_store : public file_store
{
public:
    pcks12_file_store();
    virtual ~pcks12_file_store();

    virtual void open(const std::wstring& location, const std::wstring& password);
};

}   // namespace cert

}


#endif