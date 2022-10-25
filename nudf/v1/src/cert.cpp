
#include <Windows.h>
#include <assert.h>
#include <Wincrypt.h>

#include <vector>
#include <list>

#include <nudf\encoding.hpp>
#include <nudf\convert.hpp>
#include <nudf\cert.hpp>


using namespace nudf::crypto;
using namespace nudf::util::convert;


//
//  class CCertDecoder
//


bool CCertDecoder::DecodeSubject(_In_ PCCERT_CONTEXT context, _Out_ std::wstring& subject)
{
    subject = DecodeNameInfo(context->pCertInfo->Subject.pbData, context->pCertInfo->Subject.cbData, szOID_COMMON_NAME);
    return (!subject.empty());
}

bool CCertDecoder::DecodeIssuer(PCCERT_CONTEXT context, _Out_ std::wstring& issuer)
{
    issuer = DecodeNameInfo(context->pCertInfo->Issuer.pbData, context->pCertInfo->Issuer.cbData, szOID_COMMON_NAME);
    return (!issuer.empty());
}

std::wstring CCertDecoder::DecodeNameInfo(_In_ unsigned char* pb, _In_ unsigned long cb, _In_ const char* szOID)
{
    PCERT_NAME_INFO ni = NULL;
    std::vector<unsigned char> nibuf;
    unsigned long   nisize = 0;
    std::wstring    wsName;


    // decode public key
    if (!CryptDecodeObject(X509_ASN_ENCODING, X509_NAME, pb, cb, 0, NULL, &nisize) || 0 == nisize) {
        return wsName;
    }

    nibuf.resize(nisize, 0);
    if (!CryptDecodeObject(X509_ASN_ENCODING, X509_NAME, pb, cb, 0, &nibuf[0], &nisize)) {
        return wsName;
    }

    ni = (PCERT_NAME_INFO)&nibuf[0];    
    for (int i = 0; i < (int)ni->cRDN; i++) {
        for (int j = 0; j < (int)ni->rgRDN[i].cRDNAttr; j++) {
            // we are looking for common name
            if (0 == strcmp(szOID, ni->rgRDN[i].rgRDNAttr[j].pszObjId)) {
                wsName = DecodeRdnAttribute(&ni->rgRDN[i].rgRDNAttr[j]);
                break;
            }
        }
    }

    return wsName;
}

bool CCertDecoder::DecodeSerial(PCCERT_CONTEXT context, _Out_ std::wstring& serial)
{
    return true;
}

bool CCertDecoder::DecodeThumbprint(PCCERT_CONTEXT context, _Out_ std::vector<unsigned char>& thumbprint)
{
    unsigned long  shasize = 0;

    if (!CertGetCertificateContextProperty(context, CERT_SHA1_HASH_PROP_ID, NULL, &shasize) || 0 == shasize) {
        return false;
    }

    thumbprint.resize(shasize, 0);
    if (!CertGetCertificateContextProperty(context, CERT_SHA1_HASH_PROP_ID, &thumbprint[0], &shasize)) {
        return false;
    }

    return true;
}

bool CCertDecoder::DecodeAlgorithm(PCCERT_CONTEXT context, _Out_ std::wstring& sign_alg, _Out_ std::wstring& thumbprint_alg)
{
    wchar_t* algs = NULL;
    unsigned long  size = 0;
    std::vector<unsigned char> data;
    
    if (!CertGetCertificateContextProperty(context, CERT_SIGN_HASH_CNG_ALG_PROP_ID, NULL, &size) || 0 == size) {
        return false;
    }
    
    data.resize(size + 2);
    size = size + 2;
    if (!CertGetCertificateContextProperty(context, CERT_SIGN_HASH_CNG_ALG_PROP_ID, &data[0], &size)) {
        return false;
    }

    sign_alg = (wchar_t*)(&data[0]);
    std::wstring::size_type pos = sign_alg.find_last_of(L'/');
    if(pos != std::wstring::npos) {
        thumbprint_alg = sign_alg.substr(pos+1);
        sign_alg = sign_alg.substr(0, pos);
    }

    return TRUE;
}

bool CCertDecoder::DecodeValidDate(PCCERT_CONTEXT context, _Out_ SYSTEMTIME* validfrom, _Out_ SYSTEMTIME* validthrough)
{
    FileTimeToSystemTime(&context->pCertInfo->NotBefore, validfrom);
    FileTimeToSystemTime(&context->pCertInfo->NotAfter, validthrough);
    return true;
}

bool CCertDecoder::DecodeKey(PCCERT_CONTEXT context, _Out_ std::vector<unsigned char>& key)
{
    unsigned long  blobsize = 0;

    // decode public key
    if (!CryptDecodeObject(X509_ASN_ENCODING,
                           RSA_CSP_PUBLICKEYBLOB,
                           context->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                           context->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                           0,
                           NULL,
                           &blobsize) || 0 == blobsize) {
        return false;
    }

    key.resize(blobsize, 0);
    if (!CryptDecodeObject(X509_ASN_ENCODING,
                           RSA_CSP_PUBLICKEYBLOB,
                           context->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                           context->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                           0,
                           &key[0],
                           &blobsize)) {
        return false;
    }

    return true;
}

std::wstring CCertDecoder::DecodeRdnAttribute(_In_ PCERT_RDN_ATTR attr)
{
    std::wstring wstr;
    unsigned long size = 0;

    size = CertRDNValueToStrW(attr->dwValueType, &attr->Value, NULL, 0);
    if (0 == size) {
        return wstr;
    }

    wstr.resize(size+1, 0);
    CertRDNValueToStrW(attr->dwValueType, &attr->Value, &wstr[0], size+1);
    return std::wstring(wstr.c_str());
}


//
//  class CCertStore
//
CCertStore::CCertStore() : _store(NULL)
{
}

CCertStore::CCertStore(_In_ LPCWSTR location) : _store(NULL), _location(location?location:L"")
{
}

CCertStore::~CCertStore()
{
    Close();
}

HCERTSTORE CCertStore::Attach(_In_ HCERTSTORE store) throw()
{
    assert(NULL == _store);
    assert(NULL != store);
    _store = store;
    return _store;
}

HCERTSTORE CCertStore::Detach() throw()
{
    HCERTSTORE store = _store;
    _store = NULL;
    return store;
}

void CCertStore::Close() throw()
{
    if(NULL != _store) {
        (VOID)CertCloseStore(_store, 0);
        _store = NULL;
    }
}

HRESULT CCertStore::ExportToPkcs12Package(_In_ LPCWSTR file, _In_ LPCWSTR password, _In_ bool with_privatekey) throw()
{
    HRESULT hr = S_OK;
    CRYPT_DATA_BLOB pfx;
    std::vector<UCHAR> data;

    pfx.cbData = 0;
    pfx.pbData = NULL;
    PFXExportCertStore(_store, &pfx, password, (with_privatekey ? (EXPORT_PRIVATE_KEYS|REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY) : 0));
    if(0 == pfx.cbData) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    data.resize(pfx.cbData, 0);
    pfx.pbData = &data[0];
    if(!PFXExportCertStore(_store, &pfx, password, (with_privatekey ? (EXPORT_PRIVATE_KEYS|REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY) : 0))) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwWritten = 0;

    hFile = ::CreateFileW(file, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr =  ::WriteFile(hFile, pfx.pbData, pfx.cbData, &dwWritten, NULL) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE;
    if(S_OK != hr) {
        ::DeleteFileW(file);
    }
    return hr;
}

HRESULT CCertStore::ExportToX509Package(_In_ LPCWSTR file) throw()
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwWritten = 0;

    hFile = ::CreateFileW(file, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = CertSaveStore(_store, X509_ASN_ENCODING, CERT_STORE_SAVE_AS_PKCS7, CERT_STORE_SAVE_TO_FILE, (void*)hFile, 0) ? S_OK : HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
    CloseHandle(hFile);
    return hr;
}

HRESULT CCertStore::AddCert(_In_ PCCERT_CONTEXT context)
{
    return (CertAddCertificateContextToStore(*this, context, CERT_STORE_ADD_REPLACE_EXISTING, NULL)) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT CCertStore::AddCert(_In_ PCCERT_CONTEXT context, _Out_ CCertContext& new_context)
{
    PCCERT_CONTEXT store_context = NULL;

    if (!CertAddCertificateContextToStore(*this, context, CERT_STORE_ADD_REPLACE_EXISTING, &store_context)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    new_context.Attach(store_context);
    return S_OK;
}

//
//  class CSysCertStore
//
CSysCertStore::CSysCertStore() : CCertStore()
{
}

CSysCertStore::CSysCertStore(_In_ LPCWSTR location) : CCertStore(location)
{
}

CSysCertStore::~CSysCertStore()
{
}

HRESULT CSysCertStore::Open() throw()
{
    return (NULL != Attach(CertOpenStore(CERT_STORE_PROV_SYSTEM,
		                                 X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		                                 NULL,
		                                 CERT_SYSTEM_STORE_LOCAL_MACHINE|CERT_STORE_NO_CRYPT_RELEASE_FLAG|CERT_STORE_OPEN_EXISTING_FLAG,
		                                 GetLocation().c_str()))) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}


//
//  class CMemCertStore
//

CMemCertStore::CMemCertStore() : CCertStore(L"memory")
{
}

CMemCertStore::~CMemCertStore()
{
}

HRESULT CMemCertStore::Open() throw()
{
    return (NULL != Attach(CertOpenStore(CERT_STORE_PROV_MEMORY, 0, NULL, 0, NULL))) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}


//
//  class CFileCertStore
//
CFileCertStore::CFileCertStore() : CCertStore()
{
}

CFileCertStore::CFileCertStore(_In_ LPCWSTR location) : CCertStore(location)
{
}

CFileCertStore::~CFileCertStore()
{
}

HRESULT CFileCertStore::Open() throw()
{
    HCERTSTORE store = NULL;

    store = CertOpenStore(CERT_STORE_PROV_FILENAME_W, X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, NULL, CERT_STORE_OPEN_EXISTING_FLAG, (const void *)GetLocation().c_str());
    if(NULL == store) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    Attach(store);
    return S_OK;
}

HRESULT CFileCertStore::Create() throw()
{
    HCERTSTORE store = NULL;

    store = CertOpenStore(CERT_STORE_PROV_FILENAME_W, X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, NULL, CERT_STORE_CREATE_NEW_FLAG, (const void *)GetLocation().c_str());
    if(NULL == store) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    Attach(store);
    return S_OK;
}


//
//  class CFileCertStore
//
CPkcs12CertStore::CPkcs12CertStore() : CCertStore()
{
}

CPkcs12CertStore::CPkcs12CertStore(_In_ LPCWSTR file) : CCertStore(file)
{
}

CPkcs12CertStore::~CPkcs12CertStore()
{
}

HRESULT CPkcs12CertStore::Open() throw()
{
    return Open(NULL);
}

HRESULT CPkcs12CertStore::Open(_In_opt_ LPCWSTR password) throw()
{
    std::vector<UCHAR> data;
    HANDLE h = INVALID_HANDLE_VALUE;

    h = ::CreateFileW(GetLocation().c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == h) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD size = GetFileSize(h, NULL);
    if(INVALID_FILE_SIZE == size) {
        CloseHandle(h);  h = INVALID_HANDLE_VALUE;
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if(0 == size) {
        CloseHandle(h);  h = INVALID_HANDLE_VALUE;
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    data.resize(size, 0);
    if(!::ReadFile(h, &data[0], size, &size, NULL)) {
        CloseHandle(h);  h = INVALID_HANDLE_VALUE;
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    // Close file
    CloseHandle(h);
    h = INVALID_HANDLE_VALUE;

    // Open store
    return Open(&data[0], size, password);
}

HRESULT CPkcs12CertStore::Open(_In_ const UCHAR* pb, _In_ ULONG cb, _In_opt_ LPCWSTR password) throw()
{
    CRYPT_DATA_BLOB blob;
    blob.cbData = cb;
    blob.pbData = (BYTE*)pb;
    return (NULL != Attach(PFXImportCertStore(&blob, password, CRYPT_EXPORTABLE))) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}



//
//  class CCertContext
//

CCertContext::CCertContext() : _context(NULL)
{
}

CCertContext::~CCertContext()
{
    Close();
}

PCCERT_CONTEXT CCertContext::Attach(_In_ PCCERT_CONTEXT context) throw()
{
    Close();
    _context = context;
    return _context;
}

PCCERT_CONTEXT CCertContext::Detach() throw()
{
    PCCERT_CONTEXT context = _context;
    _context = NULL;
    return context;
}

void CCertContext::Close() throw()
{
    if(NULL != _context) {
        CertFreeCertificateContext(_context);
        _context = NULL;
    }
}

HRESULT CCertContext::Create(_In_ LPCWSTR subject, _In_ bool signature, _In_ bool strong, _In_ int valid_years)
{
    return Create(subject, signature, strong, NULL, valid_years);
}

HRESULT CCertContext::Create(_In_ LPCWSTR subject, _In_ bool signature, _In_ bool strong, _In_opt_ PCCERT_CONTEXT root, _In_ int valid_years)
{
    HRESULT hr = S_OK;
    DWORD dwSize = 0;
    std::vector<UCHAR> data;
    DWORD dwKeyFlag = strong ? 0x08000000/*2048 Bits Key*/ : RSA1024BIT_KEY;

    assert(NULL == _context);

	if (!CertStrToNameW(X509_ASN_ENCODING, subject, CERT_OID_NAME_STR, NULL, NULL, &dwSize, NULL)) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

    assert(0 != dwSize);
    data.resize(dwSize, 0);
	if (!CertStrToNameW(X509_ASN_ENCODING, subject, CERT_OID_NAME_STR, NULL, &data[0], &dwSize, NULL)) {
		return HRESULT_FROM_WIN32(GetLastError());
	}

    CERT_NAME_BLOB SubjectIssuerBlob;
    memset(&SubjectIssuerBlob, 0, sizeof(SubjectIssuerBlob));
    SubjectIssuerBlob.cbData = dwSize;
    SubjectIssuerBlob.pbData = &data[0];

    const wchar_t* wzContainerName = L"NextLabsKeyContainer";
    // Prepare key provider structure for self-signed certificate    
    CRYPT_KEY_PROV_INFO KeyProvInfo;
    memset(&KeyProvInfo, 0, sizeof(KeyProvInfo));
    KeyProvInfo.pwszContainerName = (LPWSTR)wzContainerName;
    KeyProvInfo.pwszProvName      = MS_ENHANCED_PROV_W;
    KeyProvInfo.dwProvType        = PROV_RSA_FULL;
    KeyProvInfo.dwFlags           = CRYPT_SILENT|CRYPT_MACHINE_KEYSET;
    KeyProvInfo.cProvParam        = 0;
    KeyProvInfo.rgProvParam       = NULL;
    KeyProvInfo.dwKeySpec         = signature ? AT_SIGNATURE : AT_KEYEXCHANGE;

    CRYPT_KEY_PROV_INFO* pKeyProvInfo = NULL;
    std::vector<unsigned char> vKeyProvInfo;

    if(NULL != root) {
        if(!(CertGetCertificateContextProperty(root, CERT_KEY_PROV_INFO_PROP_ID, NULL, &dwSize)))  {
            return HRESULT_FROM_WIN32(GetLastError());
        } 
        vKeyProvInfo.resize(dwSize, 0);
        pKeyProvInfo = (CRYPT_KEY_PROV_INFO*)vKeyProvInfo.data();
        if(!(CertGetCertificateContextProperty(root, CERT_KEY_PROV_INFO_PROP_ID, pKeyProvInfo, &dwSize)))  {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    HCRYPTPROV hProv = NULL;
    if (!CryptAcquireContextW(&hProv, wzContainerName, MS_ENHANCED_PROV_W, PROV_RSA_FULL, CRYPT_SILENT|CRYPT_MACHINE_KEYSET)) {
        // Key set not exist
        if (!CryptAcquireContextW(&hProv, wzContainerName, MS_ENHANCED_PROV_W, PROV_RSA_FULL, CRYPT_SILENT|CRYPT_MACHINE_KEYSET|CRYPT_NEWKEYSET)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            return hr;
        }
    }

    HCRYPTKEY hKey = NULL;
    if (!CryptGenKey(hProv, signature ? AT_SIGNATURE : AT_KEYEXCHANGE, dwKeyFlag | CRYPT_EXPORTABLE, &hKey)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
    }


    // Prepare algorithm structure for self-signed certificate
    CRYPT_ALGORITHM_IDENTIFIER SignatureAlgorithm;
    memset(&SignatureAlgorithm, 0, sizeof(SignatureAlgorithm));
    SignatureAlgorithm.pszObjId = szOID_RSA_SHA1RSA;

    // Prepare Expiration date for self-signed certificate
    SYSTEMTIME EndTime;
    GetSystemTime(&EndTime);
    EndTime.wYear += valid_years;

    // Create self-signed certificate    
    hr = (NULL != Attach(CertCreateSelfSignCertificate(NULL, &SubjectIssuerBlob, 0, (NULL !=pKeyProvInfo) ? pKeyProvInfo : (&KeyProvInfo), &SignatureAlgorithm, NULL, &EndTime, 0))) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

    CryptDestroyKey(hKey);
    return hr;
}

HRESULT CCertContext::GetProperty(_In_ DWORD id, _Out_ std::vector<UCHAR>& data) const throw()
{
    DWORD size = 0;

    if(NULL == _context) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
    }

    if(!CertGetCertificateContextProperty(_context, id, NULL, &size)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    assert(0 != size);
    data.resize(size, 0);
    if(!CertGetCertificateContextProperty(_context, id, &data[0], &size)) {
        data.clear();
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT CCertContext::GetPropAccessState(_Out_ PDWORD state) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> data;

    hr = GetProperty(CERT_ACCESS_STATE_PROP_ID, data);
    if(S_OK != hr) {
        return hr;
    }

    assert(data.size() == sizeof(DWORD));
    *state = *((PDWORD)(&data[0]));
    return S_OK;
}

HRESULT CCertContext::GetPropCrossCertDistPoints(_Out_ std::vector<UCHAR>& dist_points) const throw()
{
    return GetProperty(CERT_CROSS_CERT_DIST_POINTS_PROP_ID, dist_points);
}

HRESULT CCertContext::GetPropCtlUsage(_Out_ std::vector<UCHAR>& usage) const throw()
{
    return GetProperty(CERT_CTL_USAGE_PROP_ID, usage);
}

HRESULT CCertContext::GetPropDateStamp(_Out_ FILETIME* date) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> data;

    hr = GetProperty(CERT_DATE_STAMP_PROP_ID, data);
    if(S_OK != hr) {
        return hr;
    }

    assert(data.size() == sizeof(FILETIME));
    *date = *((FILETIME*)(&data[0]));
    return S_OK;
}

HRESULT CCertContext::GetPropDescription(_Out_ std::vector<UCHAR>& desc) const throw()
{
    return GetProperty(CERT_DESCRIPTION_PROP_ID, desc);
}

HRESULT CCertContext::GetPropEnhKeyUsage(_Out_ std::vector<UCHAR>& usage) const throw()
{
    return GetProperty(CERT_ENHKEY_USAGE_PROP_ID, usage);
}

HRESULT CCertContext::GetPropFriendlyName(_Out_ std::wstring& name) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> data;

    hr = GetProperty(CERT_FRIENDLY_NAME_PROP_ID, data);
    if(S_OK != hr) {
        return hr;
    }

    name = (LPCWSTR)(&data[0]);
    return S_OK;
}

HRESULT CCertContext::GetPropHash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetPropIssuerPubKeyMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_ISSUER_PUBLIC_KEY_MD5_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetPropIssuerSnMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_ISSUER_SERIAL_NUMBER_MD5_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetPropKeyProvHandle(_Out_ HCRYPTPROV* handle) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> data;

    hr = GetProperty(CERT_KEY_PROV_HANDLE_PROP_ID, data);
    if(S_OK != hr) {
        return hr;
    }

    assert(data.size() == sizeof(HCRYPTPROV));
    *handle = *((HCRYPTPROV*)(&data[0]));
    return S_OK;
}

HRESULT CCertContext::GetPropKeyProvInfo(_Out_ std::vector<UCHAR>& info) const throw()
{
    return GetProperty(CERT_KEY_PROV_INFO_PROP_ID, info);
}

HRESULT CCertContext::GetPropKeySpec(_Out_ PDWORD spec) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> data;

    hr = GetProperty(CERT_KEY_SPEC_PROP_ID, data);
    if(S_OK != hr) {
        return hr;
    }

    assert(data.size() == sizeof(DWORD));
    *spec = *((PDWORD)(&data[0]));
    return S_OK;
}

HRESULT CCertContext::GetPropMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_MD5_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetPropSha1Hash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_SHA1_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetPropSignHashCngAlgorithm(_Out_ std::wstring& sign_alg, _Out_ std::wstring& hash_alg) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> data;
    LPWSTR alg_pair = NULL;
    LPWSTR pos = NULL;

    hr = GetProperty(CERT_SIGN_HASH_CNG_ALG_PROP_ID, data);
    if(S_OK != hr) {
        return hr;
    }

    alg_pair = (LPWSTR)(&data[0]);
    pos = wcschr(alg_pair, '/');
    assert(NULL != pos);
    if(NULL == pos) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    *pos = L'\0'; ++pos;
    sign_alg = alg_pair;
    hash_alg = pos;
    return S_OK;
}

HRESULT CCertContext::GetPropSignatureHash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_SIGNATURE_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetPropSubjectInfoAccess(_Out_ std::vector<UCHAR>& access) const throw()
{
    return GetProperty(CERT_SUBJECT_INFO_ACCESS_PROP_ID, access);
}

HRESULT CCertContext::GetPropSubjectNameMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_SUBJECT_NAME_MD5_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetPropSubjectPubKeyMd5Hash(_Out_ std::vector<UCHAR>& hash) const throw()
{
    return GetProperty(CERT_SUBJECT_PUBLIC_KEY_MD5_HASH_PROP_ID, hash);
}

HRESULT CCertContext::GetSubjectName(_Out_ std::wstring& name) const throw()
{
    WCHAR wzName[256] = {0};
    if(!CertGetNameStringW(_context, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, wzName, 256)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    name = wzName;
    return S_OK;
}

HRESULT CCertContext::GetIssuerName(_Out_ std::wstring& name) const throw()
{
    WCHAR wzName[256] = {0};
    if(!CertGetNameStringW(_context, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL, wzName, 256)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    name = wzName;
    return S_OK;
}

HRESULT CCertContext::GetValidFromDate(_Out_ PSYSTEMTIME date) const throw()
{
    return FileTimeToSystemTime(&_context->pCertInfo->NotBefore, date) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT CCertContext::GetValidThruDate(_Out_ PSYSTEMTIME date) const throw()
{
    return FileTimeToSystemTime(&_context->pCertInfo->NotAfter, date) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

bool CCertContext::operator == (_In_ const CCertContext& context) const throw()
{
    if(!IsValid() || !context.IsValid()) {
        return false;
    }
    return CertCompareCertificate(X509_ASN_ENCODING, _context->pCertInfo,  ((PCCERT_CONTEXT)context)->pCertInfo) ? true : false;
}

bool CCertContext::IsSelfSign() const throw()
{
    if(!IsValid()) {
        return false;
    }
    return CertCompareCertificateName(X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, &(_context->pCertInfo->Issuer), &(_context->pCertInfo->Subject)) ? true : false;
}

bool CCertContext::EqualPublicKey(_In_ const CCertContext& context) const throw()
{
    if(!IsValid() || !context.IsValid()) {
        return false;
    }
    return CertComparePublicKeyInfo(X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, &(_context->pCertInfo->SubjectPublicKeyInfo),  &(((PCCERT_CONTEXT)context)->pCertInfo->SubjectPublicKeyInfo)) ? true : false;
}

bool CCertContext::HasPrivateKey() const throw()
{
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE hProv = NULL;
    DWORD   dwKeySpec = 0;
    BOOL    bCallerFree = FALSE;
    bool    result = false;

    // searching for cert with private key
    result = CryptAcquireCertificatePrivateKey(_context, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &hProv, &dwKeySpec, &bCallerFree) ? true : false;
    if(bCallerFree) {
        CryptReleaseContext(hProv, 0);
        hProv = NULL;
    }

    return result;
}

DWORD CCertContext::GetPublicKeyLength() const throw()
{
    return CertGetPublicKeyLength(X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, &(_context->pCertInfo->SubjectPublicKeyInfo));
}

HRESULT CCertContext::GetPublicKeyBlob(_Out_ std::vector<UCHAR>& keyblob) const throw()
{
    ULONG size = 0;

    // decode public key
    if (!CryptDecodeObject(X509_ASN_ENCODING,
                           RSA_CSP_PUBLICKEYBLOB,
                           _context->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                           _context->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                           0,
                           NULL,
                           &size)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if(0 == size) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    keyblob.resize(size, 0);
    if (!CryptDecodeObject(X509_ASN_ENCODING,
                           RSA_CSP_PUBLICKEYBLOB,
                           _context->pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
                           _context->pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData,
                           0,
                           &keyblob[0],
                           &size)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT CCertContext::GetPublicKeyBlob(_Out_ CRsaPubKeyBlob& keyblob) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> blob;
    hr = GetPublicKeyBlob(blob);
    if(S_OK != hr) {
        return hr;
    }
    return keyblob.SetBlob(&blob[0], (ULONG)blob.size());
}

HRESULT CCertContext::GetPublicKeyBlob(_Out_ CLegacyRsaPubKeyBlob& keyblob) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> blob;
    hr = GetPublicKeyBlob(blob);
    if(S_OK != hr) {
        return hr;
    }
    return keyblob.SetBlob(&blob[0], (ULONG)blob.size());
}

HRESULT CCertContext::GetPrivateKeyBlob(_Out_ std::vector<UCHAR>& keyblob) const throw()
{
    HRESULT     hr = S_OK;
    HCRYPTKEY   hKey = NULL;
    HCRYPTPROV  hProv = NULL;
    DWORD       dwKeySpec = 0;
    BOOL        bCallerFree = FALSE;
    DWORD       dwSize = 0;
    
    // searching for cert with private key
    if(!CryptAcquireCertificatePrivateKey(_context, CRYPT_ACQUIRE_COMPARE_KEY_FLAG, NULL, &hProv, &dwKeySpec, &bCallerFree)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto _exit;
    }

    // Get private key
    if (!CryptGetUserKey(hProv, dwKeySpec, &hKey)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto _exit;
    }

    // export key
    if (!CryptExportKey(hKey, NULL, PRIVATEKEYBLOB, 0, NULL, &dwSize) && ERROR_MORE_DATA != GetLastError()) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto _exit;
    }

    assert(0 != dwSize);
    if(0 == dwSize) {
        hr = NTE_NO_KEY;
        goto _exit;
    }

    keyblob.resize(dwSize, 0);

    if (!CryptExportKey(hKey, NULL, PRIVATEKEYBLOB, 0, &keyblob[0], &dwSize)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto _exit;
    }


_exit:
    if(NULL != hKey) {
        CryptDestroyKey(hKey);
        hKey = NULL;
    }
    if(bCallerFree && NULL!=hProv) {
        CryptReleaseContext(hProv, 0);
        hProv = NULL;
    }
    return hr;
}

HRESULT CCertContext::GetPrivateKeyBlob(_Out_ CLegacyRsaPriKeyBlob& keyblob) const throw()
{
    HRESULT hr = S_OK;
    std::vector<UCHAR> blob;
    hr = GetPrivateKeyBlob(blob);
    if(S_OK != hr) {
        return hr;
    }
    return keyblob.SetBlob(&blob[0], (ULONG)blob.size());
}


//
//  class CX509CertContext
//

CX509CertContext::CX509CertContext() : CCertContext()
{
}

CX509CertContext::~CX509CertContext()
{
}

HRESULT CX509CertContext::Create(_In_ const UCHAR* pb, _In_ ULONG cb)
{
    return (NULL != Attach(CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, pb, cb))) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT CX509CertContext::Create(_In_ LPCWSTR base64)
{
    std::vector<UCHAR> certdata;
    if(!nudf::util::encoding::Base64Decode<wchar_t>(base64, certdata)) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    return Create(&certdata[0], (DWORD)certdata.size());
}

HRESULT CX509CertContext::Create(_In_ LPCSTR base64)
{
    std::vector<UCHAR> certdata;
    if(!nudf::util::encoding::Base64Decode<char>(base64, certdata)) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    return Create(&certdata[0], (DWORD)certdata.size());
}

HRESULT CX509CertContext::CreateFromFile(_In_ LPCWSTR file)
{
    HANDLE hFile  = INVALID_HANDLE_VALUE;
    DWORD  dwSize = 0;

    hFile = ::CreateFileW(file, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    dwSize = GetFileSize(hFile, NULL);
    if(INVALID_FILE_SIZE == dwSize) {
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if(0 == dwSize || dwSize > 0x1000000 /*16MB*/) {
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    std::vector<char> content;
    content.resize(dwSize+1, 0);
    if(!ReadFile(hFile, &content[0], dwSize, &dwSize, NULL)) {
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;

    std::vector<UCHAR> certdata;
    const UCHAR* pb = (const UCHAR*)&content[0];
    ULONG cb = dwSize;
    if(nudf::util::encoding::Base64Decode<char>(&content[0], certdata)) {
        pb = (const UCHAR*)&certdata[0];
        cb = (ULONG)certdata.size();
    }

    return Create(pb, cb);
}


//
//  class CPkcs12CertContext
//

CPkcs12CertContext::CPkcs12CertContext() : CCertContext()
{
}

CPkcs12CertContext::~CPkcs12CertContext()
{
}

HRESULT CPkcs12CertContext::Create(_In_ const UCHAR* pb, _In_ ULONG cb, _In_ LPCWSTR password)
{
    HRESULT hr = S_OK;
    CPkcs12CertStore store;
    CCertContext context;

    hr = store.Open(pb, cb, password);
    if(S_OK != hr) {
        return hr;
    }

    if(!store.FindCert(context, [] (const CCertContext& c) -> bool {return c.HasPrivateKey();}) || !context.IsValid()) {
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    Attach(context.Detach());
    return S_OK;
}

HRESULT CPkcs12CertContext::Create(_In_ LPCWSTR base64, _In_ LPCWSTR password)
{
    std::vector<UCHAR> certdata;
    if(!nudf::util::encoding::Base64Decode<wchar_t>(base64, certdata)) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    return Create(&certdata[0], (DWORD)certdata.size(), password);
}

HRESULT CPkcs12CertContext::Create(_In_ LPCSTR base64, _In_ LPCWSTR password)
{
    std::vector<UCHAR> certdata;
    if(!nudf::util::encoding::Base64Decode<char>(base64, certdata)) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }
    return Create(&certdata[0], (DWORD)certdata.size(), password);
}

HRESULT CPkcs12CertContext::CreateFromFile(_In_ LPCWSTR file, _In_ LPCWSTR password)
{
    HRESULT hr = S_OK;
    CPkcs12CertStore store(file);
    CCertContext context;

    hr = store.Open(password);
    if(S_OK != hr) {
        return hr;
    }

    if(!store.FindCert(context, [] (const CCertContext& c) -> bool {return c.HasPrivateKey();}) || !context.IsValid()) {
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    Attach(context.Detach());
    return S_OK;
}
