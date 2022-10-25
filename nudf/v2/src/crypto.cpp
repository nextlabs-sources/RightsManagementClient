
#include <Windows.h>
#include <assert.h>

#include <fstream>

#include <nudf\eh.hpp>
#include <nudf\debug.hpp>
#include <nudf\string.hpp>
#include <nudf\handyutil.hpp>
#include <nudf\crypto.hpp>


using namespace NX;
using namespace NX::crypto;
using namespace NX::certificate;

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL 0xC0000023
#endif

//
//  namespace NX::crypto::CRYPT32
//

CRYPT32::crypt_provider::crypt_provider() : handle_object()
{
}

CRYPT32::crypt_provider::crypt_provider(HCRYPTPROV h) : handle_object((HANDLE)h)
{
}

CRYPT32::crypt_provider::~crypt_provider()
{
}

void CRYPT32::crypt_provider::acquire(const std::wstring& name, bool enhanced, bool always_open, bool machine_keyset, bool slient)
{
    const unsigned long flags = ((machine_keyset ? CRYPT_MACHINE_KEYSET : 0) | (slient ? CRYPT_SILENT : 0));

    HCRYPTPROV h = NULL;

    close();

    if (!CryptAcquireContextW(&h, name.empty() ? NULL : name.c_str(), enhanced ? MS_ENHANCED_PROV_W : NULL, PROV_RSA_FULL, flags)) {

        if (GetLastError() != NTE_BAD_KEYSET) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }

        if (!always_open) {
            throw NX::exception(WIN32_ERROR_MSG2(NTE_BAD_KEYSET));
        }

        if (!CryptAcquireContextW(&h, name.empty() ? NULL : name.c_str(), enhanced ? MS_ENHANCED_PROV_W : NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET | flags)) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }
    }

    if (NULL != h) {
        attach(h);
    }
}

void CRYPT32::crypt_provider::close()
{
    if (!empty()) {
        CryptReleaseContext(detach(), 0);
    }
}

CRYPT32::crypt_provider_info::crypt_provider_info() : data_blob()
{
}

CRYPT32::crypt_provider_info::crypt_provider_info(const std::vector<unsigned char>& buf) : data_blob(buf)
{
}

CRYPT32::crypt_provider_info::~crypt_provider_info()
{
}

CRYPT32::crypt_provider_info& CRYPT32::crypt_provider_info::operator = (const CRYPT32::crypt_provider_info& other)
{
    if (this != &other) {
        data_blob::operator= (other.buffer());
    }
    return *this;
}

CRYPT32::public_key_blob::public_key_blob() : data_blob()
{
}

CRYPT32::public_key_blob::public_key_blob(const std::vector<unsigned char>& buf) : data_blob(buf)
{
    if (!valid()) {
        clear();
    }
}

CRYPT32::public_key_blob::~public_key_blob()
{
}

bool CRYPT32::public_key_blob::valid() const noexcept
{
    if (buffer().size() <= sizeof(CRYPT32_RSAKEY_HEADER)) {
        return false;
    }
    PCCRYPT32_RSAKEY_HEADER header = (PCCRYPT32_RSAKEY_HEADER)buffer().data();
    const unsigned long blob_size = sizeof(CRYPT32_RSAKEY_HEADER) + (header->pubkey.bitlen + 7) / 8;
    if (buffer().size() < blob_size) {
        return false;
    }
    if (header->pubkey_struc.bType != PUBLICKEYBLOB || header->pubkey.magic != 0x31415352/*'RSA1'*/) {
        return false;
    }
    return true;
}

CRYPT32::PCCRYPT32_RSAKEY_HEADER CRYPT32::public_key_blob::rsa_header() const noexcept
{
    return empty() ? nullptr : ((PCCRYPT32_RSAKEY_HEADER)buffer().data());
}

CRYPT32::public_key_blob& CRYPT32::public_key_blob::operator = (const std::vector<unsigned char>& buf)
{
    if (!valid()) {
        throw NX::exception(WIN32_ERROR_MSG2(ERROR_INVALID_DATA));
    }
    data_blob::operator=(buf);
    return *this;
}

const unsigned char* CRYPT32::public_key_blob::public_exponent() const noexcept
{
    return empty() ? nullptr : ((const unsigned char*)(&(rsa_header()->pubkey.pubexp)));
}

unsigned long CRYPT32::public_key_blob::public_exponent_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.pubexp & 0xFF000000) ? 4 :
                          (rsa_header()->pubkey.pubexp & 0x00FF0000) ? 3 :
                          (rsa_header()->pubkey.pubexp & 0x0000FF00) ? 2 : 1);
}

const unsigned char* CRYPT32::public_key_blob::modulus() const noexcept
{
    return empty() ? nullptr : ((const unsigned char*)(rsa_header() + 1));
}

unsigned long CRYPT32::public_key_blob::modulus_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 7) / 8);
}


CRYPT32::private_key_blob::private_key_blob()
{
}

CRYPT32::private_key_blob::private_key_blob(const std::vector<unsigned char>& buf) : data_blob(buf)
{
    if (!valid()) {
        clear();
    }
}

CRYPT32::private_key_blob::~private_key_blob()
{
}

bool CRYPT32::private_key_blob::valid() const noexcept
{
    if (buffer().size() <= sizeof(CRYPT32_RSAKEY_HEADER)) {
        return false;
    }

    const unsigned long blob_size = sizeof(CRYPT32_RSAKEY_HEADER)
                                    + modulus_length()              // Modulus
                                    + prime1_length()               // Prime1
                                    + prime2_length()               // Prime2
                                    + prime1_length()               // Exponent1
                                    + prime2_length()               // Exponent2
                                    + prime1_length()               // Coefficient
                                    + private_exponent_length();    // PrivateExponent
    if (buffer().size() < blob_size) {
        return false;
    }
    if (rsa_header()->pubkey_struc.bType != PRIVATEKEYBLOB) {
        return false;
    }
    if (rsa_header()->pubkey.magic != 0x31415352/*'RSA1'*/) {
        return false;
    }
    return true;
}

CRYPT32::PCCRYPT32_RSAKEY_HEADER CRYPT32::private_key_blob::rsa_header() const noexcept
{
    return empty() ? nullptr : ((PCCRYPT32_RSAKEY_HEADER)buffer().data());
}

const unsigned char* CRYPT32::private_key_blob::public_exponent() const noexcept
{
    return empty() ? nullptr : ((const unsigned char*)(&(rsa_header()->pubkey.pubexp)));
}

unsigned long CRYPT32::private_key_blob::public_exponent_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.pubexp & 0xFF000000) ? 4 :
                          (rsa_header()->pubkey.pubexp & 0x00FF0000) ? 3 :
                          (rsa_header()->pubkey.pubexp & 0x0000FF00) ? 2 : 1);
}

const unsigned char* CRYPT32::private_key_blob::modulus() const noexcept
{
    return empty() ? nullptr : ((const unsigned char*)(rsa_header() + 1));
}

unsigned long CRYPT32::private_key_blob::modulus_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 7) / 8);
}

const unsigned char* CRYPT32::private_key_blob::prime1() const noexcept
{
    return empty() ? nullptr : (modulus() + modulus_length());
}

unsigned long CRYPT32::private_key_blob::prime1_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 15) / 16);
}

const unsigned char* CRYPT32::private_key_blob::prime2() const noexcept
{
    return empty() ? nullptr : (prime1() + prime1_length());
}

unsigned long CRYPT32::private_key_blob::prime2_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 15) / 16);
}

const unsigned char* CRYPT32::private_key_blob::exponent1() const noexcept
{
    return empty() ? nullptr : (prime2() + prime2_length());
}

unsigned long CRYPT32::private_key_blob::exponent1_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 15) / 16);
}

const unsigned char* CRYPT32::private_key_blob::exponent2() const noexcept
{
    return empty() ? nullptr : (exponent1() + exponent1_length());
}

unsigned long CRYPT32::private_key_blob::exponent2_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 15) / 16);
}

const unsigned char* CRYPT32::private_key_blob::coefficient() const noexcept
{
    return empty() ? nullptr : (exponent2() + exponent2_length());
}

unsigned long CRYPT32::private_key_blob::coefficient_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 15) / 16);
}

const unsigned char* CRYPT32::private_key_blob::private_exponent() const noexcept
{
    return empty() ? nullptr : (coefficient() + coefficient_length());
}

unsigned long CRYPT32::private_key_blob::private_exponent_length() const noexcept
{
    return empty() ? 0 : ((rsa_header()->pubkey.bitlen + 7) / 8);
}

CRYPT32::public_key_blob CRYPT32::private_key_blob::get_public_key() const noexcept
{
    std::vector<unsigned char> buf;
    if (!empty()) {

        buf.resize(sizeof(CRYPT32::CRYPT32_RSAKEY_HEADER) + modulus_length());
        CRYPT32::PCRYPT32_RSAKEY_HEADER header = (CRYPT32::PCRYPT32_RSAKEY_HEADER)buf.data();
        header->pubkey_struc.bType = PUBLICKEYBLOB;
        header->pubkey_struc.bVersion = rsa_header()->pubkey_struc.bVersion;
        header->pubkey_struc.reserved = rsa_header()->pubkey_struc.reserved;
        header->pubkey_struc.aiKeyAlg = rsa_header()->pubkey_struc.aiKeyAlg;
        header->pubkey.magic  = 0x31415352/*'RSA1'*/;
        header->pubkey.bitlen = rsa_header()->pubkey.bitlen;
        header->pubkey.pubexp = rsa_header()->pubkey.pubexp;
        memcpy(header + 1, modulus(), modulus_length());
    }
    return CRYPT32::public_key_blob(buf);
}



CRYPT32::crypt_key::crypt_key() : handle_object()
{
}

CRYPT32::crypt_key::crypt_key(HCRYPTKEY h) : handle_object((HANDLE)h)
{
}

CRYPT32::crypt_key::~crypt_key()
{
}

void CRYPT32::crypt_key::close()
{
    if (!empty()) {
        CryptDestroyKey(detach());
    }
}

CRYPT32::public_key::public_key() : CRYPT32::crypt_key()
{
}

CRYPT32::public_key::public_key(HCRYPTKEY h) : CRYPT32::crypt_key(h)
{
}

CRYPT32::public_key::~public_key()
{
}

CRYPT32::public_key_blob CRYPT32::public_key::export_public_key() const
{
    unsigned long size = 0;
    std::vector<unsigned char> buf;

    // export key
    if (!CryptExportKey((HCRYPTKEY)handle(), NULL, PUBLICKEYBLOB, 0, NULL, &size) && ERROR_MORE_DATA != GetLastError()) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    buf.resize(size, 0);
    if (!CryptExportKey((HCRYPTKEY)handle(), NULL, PUBLICKEYBLOB, 0, buf.data(), &size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    return CRYPT32::public_key_blob(buf);
}

CRYPT32::private_key::private_key() : CRYPT32::crypt_key()
{
}

CRYPT32::private_key::private_key(HCRYPTKEY h) : CRYPT32::crypt_key(h)
{
}

CRYPT32::private_key::~private_key()
{
}

void CRYPT32::private_key::generate(const CRYPT32::crypt_provider& prov, unsigned long key_spec, bool strong, bool exportable)
{
    const unsigned long flags = (strong ? 0x08000000/*2048 Bits Key*/ : RSA1024BIT_KEY) | (exportable ? CRYPT_EXPORTABLE : 0);
    HCRYPTKEY h = NULL;

    close();

    if (!CryptGenKey(prov, key_spec, flags, &h)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    attach(h);
}

CRYPT32::private_key_blob CRYPT32::private_key::export_private_key() const
{
    unsigned long size = 0;
    std::vector<unsigned char> buf;

    // export key
    if (!CryptExportKey((HCRYPTKEY)handle(), NULL, PRIVATEKEYBLOB, 0, NULL, &size) && ERROR_MORE_DATA != GetLastError()) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    buf.resize(size, 0);
    if (!CryptExportKey((HCRYPTKEY)handle(), NULL, PRIVATEKEYBLOB, 0, buf.data(), &size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    return CRYPT32::private_key_blob(buf);
}

CRYPT32::public_key_blob CRYPT32::private_key::export_public_key() const
{
    unsigned long size = 0;
    std::vector<unsigned char> buf;

    // export key
    if (!CryptExportKey((HCRYPTKEY)handle(), NULL, PUBLICKEYBLOB, 0, NULL, &size) && ERROR_MORE_DATA != GetLastError()) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    buf.resize(size, 0);
    if (!CryptExportKey((HCRYPTKEY)handle(), NULL, PUBLICKEYBLOB, 0, buf.data(), &size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    return CRYPT32::public_key_blob(buf);
}



//
//  namespace NX::crypto::BCRYPT
//

namespace bcrypt_detail {

class crypt_provider_aes : public BCRYPT::crypt_provider
{
public:
    crypt_provider_aes() : crypt_provider(BCRYPT_AES_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0)
    {
        if (0 != BCryptSetProperty(handle(), BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0)) {
            close();
        }
    }
    virtual ~crypt_provider_aes() {}
};
crypt_provider_aes& bcrypt_aes_provider()
{
    static crypt_provider_aes prov;
    return prov;
}

class crypt_provider_rsa : public BCRYPT::crypt_provider
{
public:
    crypt_provider_rsa() : crypt_provider(BCRYPT_RSA_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0) {}
    virtual ~crypt_provider_rsa() {}
};
crypt_provider_rsa& bcrypt_rsa_provider()
{
    static crypt_provider_rsa prov;
    return prov;
}

class crypt_provider_rc4 : public BCRYPT::crypt_provider
{
public:
    crypt_provider_rc4() : crypt_provider(BCRYPT_RC4_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0) {}
    virtual ~crypt_provider_rc4() {}
};
crypt_provider_rc4& bcrypt_rc4_provider()
{
    static crypt_provider_rc4 prov;
    return prov;
}

class crypt_provider_md5 : public BCRYPT::crypt_provider
{
public:
    crypt_provider_md5() : crypt_provider(BCRYPT_MD5_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0), _hash_length(0), _object_length(0)
    {
        unsigned long ret = 0;
        if (0 != BCryptGetProperty(handle(), BCRYPT_HASH_LENGTH, (PUCHAR)&_hash_length, sizeof(unsigned long), &ret, 0)) {
            close();
        }
        if (0 != BCryptGetProperty(handle(), BCRYPT_OBJECT_LENGTH, (PUCHAR)&_object_length, sizeof(unsigned long), &ret, 0)) {
            close();
        }
    }
    virtual ~crypt_provider_md5() {}
    virtual void close() { BCRYPT::crypt_provider::close(); _hash_length = 0; _object_length = 0; }
    inline unsigned long hash_length() const { return _hash_length; }
    inline unsigned long object_length() const { return _object_length; }
private:
    unsigned long   _hash_length;
    unsigned long   _object_length;
};
crypt_provider_md5& bcrypt_md5_provider()
{
    static crypt_provider_md5 prov;
    return prov;
}

class crypt_provider_sha1 : public BCRYPT::crypt_provider
{
public:
    crypt_provider_sha1() : crypt_provider(BCRYPT_SHA1_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0), _hash_length(0), _object_length(0)
    {
        unsigned long ret = 0;
        if (0 != BCryptGetProperty(handle(), BCRYPT_HASH_LENGTH, (PUCHAR)&_hash_length, sizeof(unsigned long), &ret, 0)) {
            close();
        }
        if (0 != BCryptGetProperty(handle(), BCRYPT_OBJECT_LENGTH, (PUCHAR)&_object_length, sizeof(unsigned long), &ret, 0)) {
            close();
        }
    }
    virtual ~crypt_provider_sha1() {}
    virtual void close() { BCRYPT::crypt_provider::close(); _hash_length = 0; _object_length = 0; }
    inline unsigned long hash_length() const { return _hash_length; }
    inline unsigned long object_length() const { return _object_length; }
private:
    unsigned long   _hash_length;
    unsigned long   _object_length;
};
crypt_provider_sha1& bcrypt_sha1_provider()
{
    static crypt_provider_sha1 prov;
    return prov;
}

class crypt_provider_sha2 : public BCRYPT::crypt_provider
{
public:
    crypt_provider_sha2() : crypt_provider(BCRYPT_SHA256_ALGORITHM, MS_PRIMITIVE_PROVIDER, 0), _hash_length(0), _object_length(0)
    {
        unsigned long ret = 0;
        if (0 != BCryptGetProperty(handle(), BCRYPT_HASH_LENGTH, (PUCHAR)&_hash_length, sizeof(unsigned long), &ret, 0)) {
            close();
        }
        if (0 != BCryptGetProperty(handle(), BCRYPT_OBJECT_LENGTH, (PUCHAR)&_object_length, sizeof(unsigned long), &ret, 0)) {
            close();
        }
    }
    virtual ~crypt_provider_sha2() {}
    virtual void close() { BCRYPT::crypt_provider::close(); _hash_length = 0; _object_length = 0; }
    inline unsigned long hash_length() const { return _hash_length; }
    inline unsigned long object_length() const { return _object_length; }
private:
    unsigned long   _hash_length;
    unsigned long   _object_length;
};
crypt_provider_sha2& bcrypt_sha2_provider()
{
    static crypt_provider_sha2 prov;
    return prov;
}


} // namespace bcrypt_detail


BCRYPT::crypt_provider::crypt_provider() : handle_object()
{
}

BCRYPT::crypt_provider::crypt_provider(NCRYPT_PROV_HANDLE h) : handle_object((HANDLE)h)
{
}

BCRYPT::crypt_provider::crypt_provider(const std::wstring& alg_name, const std::wstring& impl_name, unsigned long flags) : handle_object()
{
    try {

        open(alg_name, impl_name, flags);

    }
    catch (const NX::exception& e) {
        UNREFERENCED_PARAMETER(e);
        close();
    }
}

BCRYPT::crypt_provider::~crypt_provider()
{
}

void BCRYPT::crypt_provider::open(const std::wstring& alg_name, const std::wstring& impl_name, unsigned long flags)
{
    NTSTATUS Status;
    ULONG    cbResult = 0;

    BCRYPT_ALG_HANDLE h = NULL;

    Status = BCryptOpenAlgorithmProvider(&h, alg_name.c_str(), impl_name.c_str(), 0);
    if (0 != Status) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    attach(h);
}

void BCRYPT::crypt_provider::close()
{
    if (!empty()) {
        CryptReleaseContext(detach(), 0);
    }
}


BCRYPT::crypt_key::crypt_key() : handle_object()
{
}

BCRYPT::crypt_key::crypt_key(BCRYPT_KEY_HANDLE h) : handle_object((HANDLE)h)
{
}

BCRYPT::crypt_key::~crypt_key()
{
}

void BCRYPT::crypt_key::close()
{
    if (!empty()) {
        BCryptDestroyKey(detach());
    }
}


BCRYPT::crypt_hash::crypt_hash() : handle_object()
{
}

BCRYPT::crypt_hash::crypt_hash(BCRYPT_HASH_HANDLE h) : handle_object(h)
{
}

BCRYPT::crypt_hash::~crypt_hash()
{
}

void BCRYPT::crypt_hash::close()
{
    if (!empty()) {
        BCryptDestroyHash(detach());
    }
}


BCRYPT::crypt_key_aes::crypt_key_aes() : BCRYPT::crypt_key(), _block_length(0)
{
}

BCRYPT::crypt_key_aes::~crypt_key_aes()
{
}

void BCRYPT::crypt_key_aes::import(const std::vector<unsigned char>& key)
{
    import(key.data(), (unsigned long)key.size());
}

void BCRYPT::crypt_key_aes::import(const unsigned char* key, unsigned long size)
{
    std::vector<unsigned char>  aes_blob;
    BCRYPT_KEY_DATA_BLOB_HEADER* p;

    if (bcrypt_detail::bcrypt_aes_provider().empty()) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_HANDLE));
    }

    close();

    if (key == nullptr) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_PARAMETER));
    }
    if (size != 16 && size != 32) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_PARAMETER));
    }

    aes_blob.resize(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + 32, 0);
    p = (BCRYPT_KEY_DATA_BLOB_HEADER*)aes_blob.data();
    p->cbKeyData = size;
    p->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
    p->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
    memcpy(aes_blob.data() + sizeof(BCRYPT_KEY_DATA_BLOB_HEADER), key, size);
    
    BCRYPT_KEY_HANDLE h = NULL;
    NTSTATUS Status = BCryptImportKey(bcrypt_detail::bcrypt_aes_provider(), NULL, BCRYPT_KEY_DATA_BLOB, &h, NULL, 0, aes_blob.data(), (unsigned long)aes_blob.size(), 0);
    if (0 != Status) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    attach(h);

    unsigned long result_size = 0;
    Status = BCryptGetProperty(h, BCRYPT_BLOCK_LENGTH, (unsigned char*)&_block_length, sizeof(unsigned long), &result_size, 0);
    if (0 != Status) {
        close();
        throw NX::exception(NT_STATUS_MSG2(Status));
    }
}

void BCRYPT::crypt_key_aes::close()
{
    BCRYPT::crypt_key::close();
    _block_length = 0;
}

std::vector<unsigned char> BCRYPT::crypt_key_aes::export_key() const
{
    std::vector<unsigned char> aes_blob;
    return std::move(aes_blob);
}

void BCRYPT::crypt_key_aes::generate(unsigned long bits_length)
{
    unsigned char key[32] = { 0 };

    if (128 != bits_length && 256 != bits_length) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_PARAMETER));
    }

    NTSTATUS Status = BCryptGenRandom(NULL, key, 32, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (0 != Status) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    import(key, bits_length / 8);
}


BCRYPT::public_key_blob::public_key_blob()
{
}

BCRYPT::public_key_blob::public_key_blob(const std::vector<unsigned char>& buf)
{
    if (!valid()) {
        clear();
    }
}

BCRYPT::public_key_blob::~public_key_blob()
{
}

bool BCRYPT::public_key_blob::valid() const noexcept
{
    if (buffer().size() <= sizeof(BCRYPT_RSAKEY_BLOB)) {
        return false;
    }

    const unsigned long blob_size = sizeof(BCRYPT_RSAKEY_BLOB)
                                    + public_exponent_length()
                                    + modulus_length();
    
    if (buffer().size() < blob_size) {
        return false;
    }

    if (rsa_header()->Magic != BCRYPT_RSAPUBLIC_MAGIC) {
        return false;
    }

    return true;
}

const BCRYPT_RSAKEY_BLOB* BCRYPT::public_key_blob::rsa_header() const noexcept
{
    return empty() ? nullptr : ((const BCRYPT_RSAKEY_BLOB*)buffer().data());
}

const unsigned char* BCRYPT::public_key_blob::public_exponent() const noexcept
{
    return empty() ? nullptr : ((const unsigned char*)(rsa_header() + 1));
}

unsigned long BCRYPT::public_key_blob::public_exponent_length() const noexcept
{
    return empty() ? 0 : rsa_header()->cbPublicExp;
}

const unsigned char* BCRYPT::public_key_blob::modulus() const noexcept
{
    return empty() ? nullptr : (public_exponent() + public_exponent_length());
}

unsigned long BCRYPT::public_key_blob::modulus_length() const noexcept
{
    return empty() ? 0 : rsa_header()->cbModulus;
}

BCRYPT::private_key_blob::private_key_blob() : data_blob()
{
}

BCRYPT::private_key_blob::private_key_blob(const std::vector<unsigned char>& buf) : data_blob(buf)
{
    if (!valid()) {
        clear();
    }
}

BCRYPT::private_key_blob::~private_key_blob()
{
}

bool BCRYPT::private_key_blob::valid() const noexcept
{
    if (buffer().size() <= sizeof(BCRYPT_RSAKEY_BLOB)) {
        return false;
    }

    if (rsa_header()->Magic == BCRYPT_RSAPRIVATE_MAGIC) {

        const unsigned long blob_size = sizeof(BCRYPT_RSAKEY_BLOB)
                                        + rsa_header()->cbPublicExp     // public exponent
                                        + rsa_header()->cbModulus       // modulus
                                        + rsa_header()->cbPrime1        // prime1
                                        + rsa_header()->cbPrime2;       // prime2

        if (buffer().size() <= blob_size) {
            return false;
        }
    }
    else if (rsa_header()->Magic == BCRYPT_RSAFULLPRIVATE_MAGIC) {

        const unsigned long blob_size = sizeof(BCRYPT_RSAKEY_BLOB)
                                        + rsa_header()->cbPublicExp     // public exponent
                                        + rsa_header()->cbModulus       // modulus
                                        + rsa_header()->cbPrime1        // prime1
                                        + rsa_header()->cbPrime2        // prime2
                                        + rsa_header()->cbPrime1        // exponent1
                                        + rsa_header()->cbPrime2        // exponent2
                                        + rsa_header()->cbPrime1        // coefficient
                                        + rsa_header()->cbModulus;      // private exponent

        if (buffer().size() <= blob_size) {
            return false;
        }
    }
    else {
        return false;
    }
    return true;
}

bool BCRYPT::private_key_blob::full_private_key() const
{
    return (buffer().size() > sizeof(BCRYPT_RSAKEY_BLOB) && rsa_header()->Magic == BCRYPT_RSAFULLPRIVATE_MAGIC);
}

const BCRYPT_RSAKEY_BLOB* BCRYPT::private_key_blob::rsa_header() const noexcept
{
    return empty() ? nullptr : ((const BCRYPT_RSAKEY_BLOB*)buffer().data());
}

const unsigned char* BCRYPT::private_key_blob::public_exponent() const noexcept
{
    return empty() ? nullptr : ((const unsigned char*)(rsa_header() + 1));
}

unsigned long BCRYPT::private_key_blob::public_exponent_length() const noexcept
{
    return empty() ? 0 : rsa_header()->cbPublicExp;
}

const unsigned char* BCRYPT::private_key_blob::modulus() const noexcept
{
    return empty() ? nullptr : (public_exponent() + public_exponent_length());
}

unsigned long BCRYPT::private_key_blob::modulus_length() const noexcept
{
    return empty() ? 0 : rsa_header()->cbModulus;
}

const unsigned char* BCRYPT::private_key_blob::prime1() const noexcept
{
    return empty() ? nullptr : (modulus() + modulus_length());
}

unsigned long BCRYPT::private_key_blob::prime1_length() const noexcept
{
    return empty() ? 0 : rsa_header()->cbPrime1;
}

const unsigned char* BCRYPT::private_key_blob::prime2() const noexcept
{
    return empty() ? nullptr : (prime1() + prime1_length());
}

unsigned long BCRYPT::private_key_blob::prime2_length() const noexcept
{
    return empty() ? 0 : rsa_header()->cbPrime2;
}

const unsigned char* BCRYPT::private_key_blob::exponent1() const noexcept
{
    return empty() ? nullptr : (full_private_key() ? (prime2() + prime2_length()) : nullptr);
}

unsigned long BCRYPT::private_key_blob::exponent1_length() const noexcept
{
    return empty() ? 0 : (full_private_key() ? rsa_header()->cbPrime1 : 0);
}

const unsigned char* BCRYPT::private_key_blob::exponent2() const noexcept
{
    return empty() ? nullptr : (full_private_key() ? (exponent1() + exponent1_length()) : nullptr);
}

unsigned long BCRYPT::private_key_blob::exponent2_length() const noexcept
{
    return empty() ? 0 : (full_private_key() ? rsa_header()->cbPrime2 : 0);
}

const unsigned char* BCRYPT::private_key_blob::coefficient() const noexcept
{
    return empty() ? nullptr : (full_private_key() ? (exponent2() + exponent2_length()) : nullptr);
}

unsigned long BCRYPT::private_key_blob::coefficient_length() const noexcept
{
    return empty() ? 0 : (full_private_key() ? rsa_header()->cbPrime1 : 0);
}

const unsigned char* BCRYPT::private_key_blob::private_exponent() const noexcept
{
    return empty() ? nullptr : (full_private_key() ? (coefficient() + coefficient_length()) : nullptr);
}

unsigned long BCRYPT::private_key_blob::private_exponent_length() const noexcept
{
    return empty() ? 0 : (full_private_key() ? rsa_header()->cbModulus : 0);
}

BCRYPT::public_key_blob BCRYPT::private_key_blob::get_public_key() const noexcept
{
    std::vector<unsigned char> buf;
    if (!empty()) {
        buf.resize(sizeof(BCRYPT_RSAKEY_BLOB) + public_exponent_length() + modulus_length());
        BCRYPT_RSAKEY_BLOB* header = (BCRYPT_RSAKEY_BLOB*)buf.data();
        header->Magic = BCRYPT_RSAPUBLIC_MAGIC;
        header->BitLength = rsa_header()->BitLength;
        header->cbPublicExp = rsa_header()->cbPublicExp;
        header->cbModulus = rsa_header()->cbModulus;
        header->cbPrime1 = 0;
        header->cbPrime2 = 0;
        memcpy(buf.data() + sizeof(BCRYPT_RSAKEY_BLOB), public_exponent(), public_exponent_length());
        memcpy(buf.data() + sizeof(BCRYPT_RSAKEY_BLOB) + public_exponent_length(), modulus(), modulus_length());
    }
    return BCRYPT::public_key_blob(buf);
}

BCRYPT::public_key::public_key() : BCRYPT::crypt_key()
{
}

BCRYPT::public_key::public_key(BCRYPT_KEY_HANDLE h) : BCRYPT::crypt_key(h)
{
}

BCRYPT::public_key::~public_key()
{
}

BCRYPT::public_key_blob BCRYPT::public_key::export_public_key() const
{
    NTSTATUS status = 0;
    unsigned long size = 0;
    std::vector<unsigned char> buf;

    // export key
    status = BCryptExportKey(handle(), NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, 0, &size, 0);
    if (STATUS_BUFFER_TOO_SMALL != status) {
        throw NX::exception(NT_STATUS_MSG2(status));

    }
    if(0 == size) {
        throw NX::exception(NT_STATUS_MSG2(status));
    }

    buf.resize(size, 0);
    status = BCryptExportKey(handle(), NULL, BCRYPT_RSAPUBLIC_BLOB, buf.data(), size, &size, 0);
    if (0 != status) {
        throw NX::exception(NT_STATUS_MSG2(status));
    }

    return BCRYPT::public_key_blob(buf);
}


BCRYPT::private_key::private_key() : BCRYPT::crypt_key()
{
}

BCRYPT::private_key::private_key(BCRYPT_KEY_HANDLE h) : BCRYPT::crypt_key(h)
{
}

BCRYPT::private_key::~private_key()
{
}

void BCRYPT::private_key::generate(bool strong)
{
    NTSTATUS            status = 0;
    BCRYPT_KEY_HANDLE   h = NULL;
    ULONG               size = 0;

    if (bcrypt_detail::bcrypt_rsa_provider().empty()) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_HANDLE));
    }


    status = BCryptGenerateKeyPair(bcrypt_detail::bcrypt_rsa_provider(), &h, strong ? 2048 : 1024, 0);
    if (0 != status) {
        throw NX::exception(NT_STATUS_MSG2(status));
    }

    attach(h);
}

BCRYPT::private_key_blob BCRYPT::private_key::export_private_key() const
{
    NTSTATUS status = 0;
    unsigned long size = 0;
    std::vector<unsigned char> buf;

    // export key
    status = BCryptExportKey(handle(), NULL, BCRYPT_RSAFULLPRIVATE_BLOB, NULL, 0, &size, 0);
    if (STATUS_BUFFER_TOO_SMALL != status) {
        throw NX::exception(NT_STATUS_MSG2(status));

    }
    if (0 == size) {
        throw NX::exception(NT_STATUS_MSG2(status));
    }

    buf.resize(size, 0);
    status = BCryptExportKey(handle(), NULL, BCRYPT_RSAFULLPRIVATE_BLOB, buf.data(), size, &size, 0);
    if (0 != status) {
        throw NX::exception(NT_STATUS_MSG2(status));
    }

    return BCRYPT::private_key_blob(buf);
}

BCRYPT::public_key_blob BCRYPT::private_key::export_public_key() const
{
    NTSTATUS status = 0;
    unsigned long size = 0;
    std::vector<unsigned char> buf;

    // export key
    status = BCryptExportKey(handle(), NULL, BCRYPT_RSAPUBLIC_BLOB, NULL, 0, &size, 0);
    if (STATUS_BUFFER_TOO_SMALL != status) {
        throw NX::exception(NT_STATUS_MSG2(status));

    }
    if (0 == size) {
        throw NX::exception(NT_STATUS_MSG2(status));
    }

    buf.resize(size, 0);
    status = BCryptExportKey(handle(), NULL, BCRYPT_RSAPUBLIC_BLOB, buf.data(), size, &size, 0);
    if (0 != status) {
        throw NX::exception(NT_STATUS_MSG2(status));
    }

    return BCRYPT::public_key_blob(buf);
}



std::vector<unsigned char> BCRYPT::aes_encrypt(const crypt_key_aes& h, const void* data, unsigned long size, unsigned __int64 ivec, const unsigned long cbc_size)
{
    NTSTATUS status = 0;
    std::vector<unsigned char> cipher_data;


    if (!IsAligned32(cbc_size, h.block_length())) {
        throw NX::exception(NT_STATUS_MSG2(ERROR_INVALID_PARAMETER));
    }
    if (!IsAligned64(ivec, cbc_size)) {
        throw NX::exception(NT_STATUS_MSG2(ERROR_INVALID_PARAMETER));
    }

    cipher_data.resize(RoundToSize32(size, h.block_length()), 0);
    unsigned char* output = cipher_data.data();
    unsigned long  output_size = (unsigned long)cipher_data.size();
    
    while (size != 0) {

        const unsigned bytes_to_encrypt = (size < cbc_size) ? size : cbc_size;
        unsigned long bytes_encrypted = 0;
        
        std::vector<unsigned char> ivec_buf = BCRYPT::md5_hash((const unsigned char*)&ivec, sizeof(unsigned __int64));

        NTSTATUS status = BCryptEncrypt(h, (unsigned char*)data, bytes_to_encrypt, NULL, ivec_buf.data(), 16, output, output_size, &bytes_encrypted, 0);
        if (0 != status) {
            throw NX::exception(NT_STATUS_MSG2(status));
        }

        assert(bytes_to_encrypt == bytes_encrypted);
        size -= bytes_to_encrypt;
        ivec += bytes_to_encrypt;
        data = (const void*)(((const unsigned char*)data) + bytes_to_encrypt);
        output += bytes_encrypted;
        output_size -= bytes_encrypted;
    }

    return std::move(cipher_data);
}

std::vector<unsigned char> BCRYPT::aes_decrypt(const crypt_key_aes& h, const void* data, unsigned long size, unsigned __int64 ivec, unsigned long cbc_size)
{
    NTSTATUS status = 0;
    std::vector<unsigned char> plain_data;


    if (!IsAligned32(cbc_size, h.block_length())) {
        throw NX::exception(NT_STATUS_MSG2(ERROR_INVALID_PARAMETER));
    }
    if (!IsAligned64(ivec, cbc_size)) {
        throw NX::exception(NT_STATUS_MSG2(ERROR_INVALID_PARAMETER));
    }
    if (!IsAligned32(size, h.block_length())) {
        throw NX::exception(NT_STATUS_MSG2(ERROR_INVALID_PARAMETER));
    }

    plain_data.resize(size, 0);
    unsigned char* output = plain_data.data();
    unsigned long  output_size = (unsigned long)plain_data.size();

    while (size != 0) {

        const unsigned bytes_to_decrypt = (size < cbc_size) ? size : cbc_size;
        unsigned long bytes_decrypted = 0;
        
        std::vector<unsigned char> ivec_buf = BCRYPT::md5_hash((const unsigned char*)&ivec, sizeof(unsigned __int64));

        status = BCryptDecrypt(h, (unsigned char*)data, bytes_to_decrypt, NULL, ivec_buf.data(), 16, output, output_size, &bytes_decrypted, 0);
        if (0 != status) {
            throw NX::exception(NT_STATUS_MSG2(status));
        }

        assert(bytes_to_decrypt == bytes_decrypted);
        size -= bytes_to_decrypt;
        ivec += bytes_to_decrypt;
        data = (const void*)(((const unsigned char*)data) + bytes_to_decrypt);
        output += bytes_decrypted;
        output_size -= bytes_decrypted;
    }

    return std::move(plain_data);
}

std::vector<unsigned char> BCRYPT::md5_hash(const unsigned char* data, unsigned long size)
{
    NTSTATUS Status = 0;
    BCRYPT_HASH_HANDLE h;
    std::vector<unsigned char> hash_object;
    std::vector<unsigned char> hash_result;

    if (bcrypt_detail::bcrypt_md5_provider().empty()) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_HANDLE));
    }

    hash_object.resize(bcrypt_detail::bcrypt_md5_provider().object_length(), 0);
    Status = BCryptCreateHash(bcrypt_detail::bcrypt_md5_provider(), &h, hash_object.data(), (ULONG)hash_object.size(), NULL, 0, 0);
    if (Status < 0) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    BCRYPT::crypt_hash ch(h);

    Status = BCryptHashData(ch, (PUCHAR)data, size, 0);
    if (Status < 0) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    hash_result.resize(bcrypt_detail::bcrypt_md5_provider().hash_length(), 0);
    Status = BCryptFinishHash(ch, hash_result.data(), (ULONG)hash_result.size(), 0);
    if (Status < 0) {
        hash_result.clear();
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    return std::move(hash_result);
}

std::vector<unsigned char> BCRYPT::sha1_hash(const unsigned char* data, unsigned long size)
{
    NTSTATUS Status = 0;
    BCRYPT_HASH_HANDLE h;
    std::vector<unsigned char> hash_object;
    std::vector<unsigned char> hash_result;

    if (bcrypt_detail::bcrypt_sha1_provider().empty()) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_HANDLE));
    }

    hash_object.resize(bcrypt_detail::bcrypt_sha1_provider().object_length(), 0);
    Status = BCryptCreateHash(bcrypt_detail::bcrypt_sha1_provider(), &h, hash_object.data(), (ULONG)hash_object.size(), NULL, 0, 0);
    if (Status < 0) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    BCRYPT::crypt_hash ch(h);

    Status = BCryptHashData(ch, (PUCHAR)data, size, 0);
    if (Status < 0) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    hash_result.resize(bcrypt_detail::bcrypt_sha1_provider().hash_length(), 0);
    Status = BCryptFinishHash(ch, hash_result.data(), (ULONG)hash_result.size(), 0);
    if (Status < 0) {
        hash_result.clear();
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    return std::move(hash_result);
}

std::vector<unsigned char> BCRYPT::sha2_hash(const unsigned char* data, unsigned long size)
{
    NTSTATUS Status = 0;
    BCRYPT_HASH_HANDLE h;
    std::vector<unsigned char> hash_object;
    std::vector<unsigned char> hash_result;

    if (bcrypt_detail::bcrypt_sha2_provider().empty()) {
        throw NX::exception(NT_STATUS_MSG2(STATUS_INVALID_HANDLE));
    }

    hash_object.resize(bcrypt_detail::bcrypt_sha2_provider().object_length(), 0);
    Status = BCryptCreateHash(bcrypt_detail::bcrypt_sha2_provider(), &h, hash_object.data(), (ULONG)hash_object.size(), NULL, 0, 0);
    if (Status < 0) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    BCRYPT::crypt_hash ch(h);

    Status = BCryptHashData(ch, (PUCHAR)data, size, 0);
    if (Status < 0) {
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    hash_result.resize(bcrypt_detail::bcrypt_sha2_provider().hash_length(), 0);
    Status = BCryptFinishHash(ch, hash_result.data(), (ULONG)hash_result.size(), 0);
    if (Status < 0) {
        hash_result.clear();
        throw NX::exception(NT_STATUS_MSG2(Status));
    }

    return std::move(hash_result);
}








//
//  class name_blob
//
name_blob::name_blob() : CERT_NAME_BLOB({0, NULL})
{
}

name_blob::name_blob(const std::wstring& s) : CERT_NAME_BLOB({0, NULL})
{
}

name_blob::~name_blob()
{
}

void name_blob::clear()
{
    this->cbData = 0;
    this->pbData = NULL;
    _buf.clear();
}

void name_blob::set_name(const std::wstring& s)
{
    clear();

    if (s.empty()) {
        return;
    }

    unsigned long size = 0;

    if (!CertStrToNameW(X509_ASN_ENCODING, s.c_str(), CERT_OID_NAME_STR, NULL, NULL, &size, NULL)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    _buf.resize(size, 0);
    if (!CertStrToNameW(X509_ASN_ENCODING, s.c_str(), CERT_OID_NAME_STR, NULL, _buf.data(), &size, NULL)) {
        _buf.clear();
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
    if (size != (unsigned long)_buf.size()) {
        _buf.resize(size);
    }

    this->cbData = (unsigned long)_buf.size();
    this->pbData = _buf.empty() ? NULL : _buf.data();
}

name_blob& name_blob::operator = (const name_blob& other)
{
    if (this != &other) {
        _buf = other.buffer();
        this->cbData = (unsigned long)_buf.size();
        this->pbData = _buf.empty() ? NULL : _buf.data();
    }
    return *this;
}




//
//  class certificate properties
//
properties::properties()
{
    memset(&_valid_from, 0, sizeof(FILETIME));
    memset(&_valid_through, 0, sizeof(FILETIME));
}

properties::~properties()
{
}

properties& properties::operator = (const properties& other)
{
    if (this != &other) {
        _subject_name = other.get_subject_name();
        _issuer_name = other.get_issuer_name();
        _friendly_name = other.get_friendly_name();
        _description = other.get_description();
        _serial = other.get_serial();
        _sign_algorithm = other.get_sign_algorithm();
        _hash_algorithm = other.get_hash_algorithm();
        _thumbprint = other.get_thumbprint();
        _signature_hash = other.get_signature_hash();
        _public_key = other.get_public_key();
        _private_key = other.get_private_key();
        _valid_from = other.get_valid_from_time();
        _valid_through = other.get_valid_through_time();
    }
    return *this;
}

void properties::clear()
{
    _subject_name.clear();
    _issuer_name.clear();
    _friendly_name.clear();
    _description.clear();
    _serial.clear();
    _thumbprint.clear();
    _sign_algorithm.clear();
    _hash_algorithm.clear();
    _signature_hash.clear();
    memset(&_valid_from, 0, sizeof(FILETIME));
    memset(&_valid_through, 0, sizeof(FILETIME));
}


//
//  class certificate
//
context::context()
{
}

context::context(PCCERT_CONTEXT p) :_p(p)
{
}

context::~context()
{
    clear();
}

void context::open(const unsigned char pb, unsigned long cb)
{
}

void context::open(const std::wstring& file)
{
}

void context::clear()
{
    if (NULL != _p) {
        CertFreeCertificateContext(_p);
        _p = NULL;
    }
}

NX::certificate::properties context::get_all_properties()
{
    NX::certificate::properties cert_props;

    cert_props.set_issuer_name(get_issuer_name());

    return cert_props;
}

std::shared_ptr<context> context::create(const std::wstring& subject, unsigned long key_spec, bool strong, context* issuer, unsigned short valid_years)
{
    name_blob       subject_name;
    CRYPT32::crypt_provider_info   kpi;
    CRYPT32::crypt_provider  cp;
    CRYPT32::private_key     ck;

    if (NULL != issuer) {
        kpi = issuer->get_key_provider_info();
        if (kpi.empty()) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }
        if (kpi->dwKeySpec != AT_SIGNATURE) {
            throw NX::exception(WIN32_ERROR_MSG2(NTE_BAD_KEY_STATE));
        }
    }
    else {
    }


    subject_name.set_name(subject);
    cp.acquire(L"");
    ck.generate(cp, key_spec, strong, true);


    // Prepare algorithm structure
    CRYPT_ALGORITHM_IDENTIFIER sign_alg;
    memset(&sign_alg, 0, sizeof(sign_alg));
    sign_alg.pszObjId = szOID_RSA_SHA256RSA;

    // Prepare Expiration date for self-signed certificate
    SYSTEMTIME expire_time;
    GetSystemTime(&expire_time);
    expire_time.wYear += valid_years;

    // Create self-signed certificate
    return std::shared_ptr<context>(new context(CertCreateSelfSignCertificate(NULL, &subject_name, 0, kpi, &sign_alg, NULL, &expire_time, 0)));
}


// issuer information

// provider/key/algorithm
CRYPT32::crypt_provider context::get_key_provider() const
{
    const std::vector<unsigned char>& buf = get_property(CERT_KEY_PROV_HANDLE_PROP_ID);
    assert(buf.size() == sizeof(HCRYPTPROV));
    return CRYPT32::crypt_provider(*((HCRYPTPROV*)buf.data()));
}

CRYPT32::crypt_provider_info context::get_key_provider_info() const
{
    return CRYPT32::crypt_provider_info(get_property(CERT_KEY_PROV_INFO_PROP_ID));
}

unsigned long context::get_key_spec() const
{
    const std::vector<unsigned char>& buf = get_property(CERT_KEY_SPEC_PROP_ID);
    assert(buf.size() == sizeof(unsigned long));
    return *((unsigned long*)buf.data());
}

unsigned long context::get_public_key_length() const
{
    return CertGetPublicKeyLength(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &(_p->pCertInfo->SubjectPublicKeyInfo));
}

std::vector<unsigned char> context::get_public_key_blob() const
{
    return decode_object(X509_ASN_ENCODING,
                         RSA_CSP_PUBLICKEYBLOB,
                         (const unsigned char*)_p->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                         _p->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                         0);
}

NX::crypto::CRYPT32::public_key_blob context::get_public_key() const
{
    return NX::crypto::CRYPT32::public_key_blob(get_public_key_blob());
}

std::vector<unsigned char> context::get_private_key_blob() const
{
    HRESULT     hr = S_OK;
    HCRYPTKEY   hk = NULL;
    HCRYPTPROV  hp = NULL;
    DWORD       key_spec = 0;
    BOOL        caller_free = FALSE;
    DWORD       key_size = 0;
    std::vector<unsigned char> key_buf;

    try {

        // searching for cert with private key
        if (!CryptAcquireCertificatePrivateKey(_p, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &hp, &key_spec, &caller_free)) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }

        // get private key
        if (!CryptGetUserKey(hp, key_spec, &hk)) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }

        // export key
        if (!CryptExportKey(hk, NULL, PRIVATEKEYBLOB, 0, NULL, &key_size) && ERROR_MORE_DATA != GetLastError()) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }

        if (0 == key_size) {
            throw NX::exception(WIN32_ERROR_MSG2(NTE_NO_KEY));
        }

        key_buf.resize(key_size, 0);
        if (!CryptExportKey(hk, NULL, PRIVATEKEYBLOB, 0, key_buf.data(), &key_size)) {
            throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
        }

        CryptDestroyKey(hk);
        hk = NULL;
        if (caller_free) { CryptReleaseContext(hp, 0); }
        hp = NULL;

    }
    catch (const NX::exception& e) {

        if (NULL != hk) {
            CryptDestroyKey(hk);
            hk = NULL;
        }
        if (NULL != hp && caller_free) {
            CryptReleaseContext(hp, 0);
            hp = NULL;
        }

        throw e;
    }

    return std::move(key_buf);
}

NX::crypto::CRYPT32::private_key_blob context::get_private_key() const
{
    return NX::crypto::CRYPT32::private_key_blob(get_private_key_blob());
}

bool context::has_private_key() const
{
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE h = NULL;
    unsigned long   key_spec = 0;
    BOOL    caller_free = FALSE;
    bool    result = false;

    // searching for cert with private key
    if (CryptAcquireCertificatePrivateKey(_p, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &h, &key_spec, &caller_free)) {
        result = true;
        if (caller_free) {
            CryptReleaseContext(h, 0);
            h = NULL;
        }
    }

    return result;
}

bool context::is_self_signed() const
{
    return CertCompareCertificateName(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &(_p->pCertInfo->Issuer), &(_p->pCertInfo->Subject)) ? true : false;
}

// cert information
std::vector<unsigned char> context::get_serial_number() const
{
    std::vector<unsigned char> serial_number;
    const unsigned long size = _p->pCertInfo->SerialNumber.cbData;
    if (0 != size) {
        for (int i = 0; i < (int)size; i++) {
            serial_number.push_back(_p->pCertInfo->SerialNumber.pbData[size - i - 1]);
        }
    }
    return std::move(serial_number);
}

std::vector<unsigned char> context::get_thumb_print() const
{
    return std::move(get_property(CERT_SHA1_HASH_PROP_ID));
}

std::wstring context::get_issuer_name() const
{
    std::wstring name;
    if (!CertGetNameStringW(_p, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL, NX::string_buffer<wchar_t>(name, MAX_PATH), MAX_PATH)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
    return std::move(name);
}

std::wstring context::get_subject_name() const
{
    std::wstring name;
    if (!CertGetNameStringW(_p, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, NX::string_buffer<wchar_t>(name, MAX_PATH), MAX_PATH)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
    return std::move(name);
}

FILETIME context::get_date_stamp() const
{
    FILETIME ft = { 0, 0 };
    const std::vector<unsigned char>& buf = get_property(CERT_DATE_STAMP_PROP_ID);
    if (buf.size() >= sizeof(FILETIME)) {
        memcpy(&ft, buf.data(), sizeof(FILETIME));
    }
    return ft;
}

std::wstring context::get_friendly_name() const
{
    const std::vector<unsigned char>& buf = get_property(CERT_FRIENDLY_NAME_PROP_ID);
    return std::move(std::wstring(buf.empty() ? L"" : ((const wchar_t*)buf.data())));
}

FILETIME context::get_start_time() const
{
    return _p->pCertInfo->NotBefore;
}

FILETIME context::get_expire_time() const
{
    return _p->pCertInfo->NotAfter;
}

std::wstring context::get_signature_algorithm() const
{
    const std::vector<unsigned char>& buf = get_property(CERT_SIGN_HASH_CNG_ALG_PROP_ID);
    if (buf.empty()) {
        return std::wstring();
    }
    const wchar_t* start_pos = (const wchar_t*)buf.data();
    const wchar_t* end_pos = wcschr(start_pos, L'/');
    if (NULL == end_pos) {
        return std::wstring();
    }
    return std::move(std::wstring(start_pos, end_pos));
}

std::wstring context::get_hash_algorithm() const
{
    const std::vector<unsigned char>& buf = get_property(CERT_SIGN_HASH_CNG_ALG_PROP_ID);
    const wchar_t* start_pos = buf.empty() ? NULL : wcschr((const wchar_t*)buf.data(), L'/');
    return std::move(std::wstring((NULL == start_pos) ? L"" : start_pos));
}

std::vector<unsigned char> context::get_signature_hash() const
{
    // signature hash size depends on the algorithm ==> get_hash_algorithm()
    return std::move(get_property(CERT_SIGNATURE_HASH_PROP_ID));
}




// private
unsigned long context::get_property_size(unsigned long id) const
{
    unsigned long size = 0;
    if (nullptr == _p) {
        throw NX::exception(WIN32_ERROR_MSG2(ERROR_INVALID_HANDLE));
    }
    if (!CertGetCertificateContextProperty(_p, id, NULL, &size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
    return size;
}

std::vector<unsigned char> context::get_property(unsigned long id) const
{
    std::vector<unsigned char>  prop;
    unsigned long size = get_property_size(id);
    
    assert(0 != size);
    prop.resize(size, 0);
    if (!CertGetCertificateContextProperty(_p, id, prop.data(), &size)) {
        prop.clear();
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
    return std::move(prop);
}

void context::set_property(unsigned long id, const void* pb, unsigned long cb)
{
}

std::vector<unsigned char> context::decode_object(unsigned long encoding_type, const char* struc_type, const unsigned char* data, unsigned long data_size, unsigned long flags) const
{
    std::vector<unsigned char> output_buf;
    unsigned long output_size = 0;

    // decode public key
    if (!CryptDecodeObject(encoding_type, struc_type, data, data_size, flags, NULL, &output_size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    if (0 == output_size) {
        return output_buf;
    }

    output_buf.resize(output_size, 0);
    if (!CryptDecodeObject(encoding_type, struc_type, data, data_size, flags, output_buf.data(), &output_size)) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }

    return std::move(output_buf);
}


//
//  class certificate store
//
store::store() : _h(NULL)
{
}

store::store(HCERTSTORE h) : _h(h)
{
}

store::~store()
{
    close();
}

void store::close()
{
    if (NULL != _h) {
        (VOID)CertCloseStore(_h, 0);
        _h = NULL;
    }
}


//
//  class certificate system store
//

const std::wstring system_store::personal_folder(L"My");
const std::wstring system_store::trust_root_ca_folder(L"Root");
const std::wstring system_store::intermediate_ca_folder(L"CertificateAuthority");
const std::wstring system_store::trusted_people_folder(L"TrustedPeople");
const std::wstring system_store::trusted_publishers_folder(L"TrustedPublisher");
const std::wstring system_store::untrusted_folder(L"Disallowed");
const std::wstring system_store::third_party_root_ca_folder(L"AuthRoot");

system_store::system_store() : store()
{
}

system_store::system_store(const std::wstring& location)
    : store(CertOpenStore(CERT_STORE_PROV_SYSTEM,
                          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                          NULL,
                          CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_NO_CRYPT_RELEASE_FLAG | CERT_STORE_OPEN_EXISTING_FLAG,
                          location.c_str()))
{
}

system_store::~system_store()
{
}

void system_store::open(const std::wstring& location)
{
    if (NULL == attach(CertOpenStore(CERT_STORE_PROV_SYSTEM,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        NULL,
        CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_NO_CRYPT_RELEASE_FLAG | CERT_STORE_OPEN_EXISTING_FLAG,
        location.c_str()))) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
}

//
//  class certificate current user store
//
user_store::user_store() : system_store()
{
}

user_store::user_store(const std::wstring& location)
{
}

user_store::~user_store()
{
}

void user_store::open(const std::wstring& location)
{
    if (NULL == attach(CertOpenStore(CERT_STORE_PROV_SYSTEM,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        NULL,
        CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_NO_CRYPT_RELEASE_FLAG | CERT_STORE_OPEN_EXISTING_FLAG,
        location.c_str()))) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
}


//
//  class certificate memory store
//
memory_store::memory_store() : store(CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, 0, NULL))
{
}

memory_store::~memory_store()
{
}

//
//  class certificate file store
//
file_store::file_store() : store()
{
}

file_store::~file_store()
{
}

void file_store::open(const std::wstring& location, const std::wstring& password)
{
    UNREFERENCED_PARAMETER(password);
    if (NULL == attach(CertOpenStore(CERT_STORE_PROV_FILENAME_W,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        NULL,
        CERT_STORE_OPEN_EXISTING_FLAG,
        location.c_str()))) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
}

//void file_store::create(const std::wstring& location)
//{
//    if (NULL == attach(CertOpenStore(CERT_STORE_PROV_FILENAME_W,
//        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
//        NULL,
//        CERT_STORE_CREATE_NEW_FLAG,
//        location.c_str()))) {
//        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
//    }
//}


//
//  class pcks12_file_store file store
//
pcks12_file_store::pcks12_file_store() : file_store()
{
}

pcks12_file_store::~pcks12_file_store()
{
}

void pcks12_file_store::open(const std::wstring& location, const std::wstring& password)
{
    std::ifstream      fs;
    std::vector<UCHAR> buf;

    if (password.empty()) {
        SetLastError(NTE_INCORRECT_PASSWORD);
        throw NX::exception(WIN32_ERROR_MSG2(NTE_INCORRECT_PASSWORD));
    }

    fs.open(location, std::ifstream::binary);
    if (!fs.is_open() || !fs.good()) {
        throw NX::exception(WIN32_ERROR_MSG(GetLastError(), "fail to open pcks12 file"));
    }

    // get length of file:
    fs.seekg(0, fs.end);
    size_t length = (size_t)fs.tellg();
    fs.seekg(0, fs.beg);

    if (0 == length) {
        SetLastError(ERROR_INVALID_DATA);
        throw NX::exception(WIN32_ERROR_MSG(ERROR_INVALID_DATA, "empty pcks12 file"));
    }

    buf.resize(length, 0);
    fs.read((char*)buf.data(), length);
    if (fs.eof() || fs.bad() || fs.fail()) {
        throw NX::exception(WIN32_ERROR_MSG(GetLastError(), "fail to read pcks12 file"));
    }

    fs.close();

    // open it
    CRYPT_DATA_BLOB blob = { (unsigned long)buf.size(), buf.data() };
    if (NULL == attach(PFXImportCertStore(&blob, password.c_str(), CRYPT_EXPORTABLE))) {
        throw NX::exception(WIN32_ERROR_MSG2(GetLastError()));
    }
}