

#include <Windows.h>
#include <assert.h>

#include <vector>

#include <nudf\exception.hpp>
#include <nudf\cert.hpp>
#include <nudf\pe.hpp>


using namespace nudf::win;




//
//  class CPEfile
//

CPEFile::CPEFile()
{
    memset(&_ntHeader, 0, sizeof(_ntHeader));
}

CPEFile::~CPEFile()
{
    Clear();
}

bool CPEFile::load_pe_header(LPCWSTR file)
{
    bool   result = false;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hFileMapping = NULL;
    LPVOID lpFileBase = NULL;

    try {

        PIMAGE_DOS_HEADER dosHeader = NULL;
        PIMAGE_NT_HEADERS ntHeader = NULL;

        hFile = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (hFile == INVALID_HANDLE_VALUE) {
            throw std::exception("fail to open file");
        }

        hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (NULL == hFileMapping) {
            throw std::exception("fail to create file mapping");
        }

        lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
        if (NULL == lpFileBase) {
            throw std::exception("fail to get mapped virtual memory");
        }

        // Parse PE Header
        dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            throw std::exception("invalid dos header magic code");
        }

        // Get NT Header
        ntHeader = (PIMAGE_NT_HEADERS)((PUCHAR)dosHeader + dosHeader->e_lfanew);
        if (0x00004550 != ntHeader->Signature) {
            throw std::exception("invalid NT header magic code");
        }

        result = true;
        memcpy(&_ntHeader, ntHeader, sizeof(_ntHeader));
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    if (lpFileBase != NULL) {
        UnmapViewOfFile(lpFileBase);
        lpFileBase = NULL;
    }
    if (NULL != hFileMapping) {
        CloseHandle(hFileMapping); hFileMapping = NULL;
    }
    if (INVALID_HANDLE_VALUE != hFile) {
        CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE;
    }

    return result;
}

void CPEFile::Load(_In_ LPCWSTR file)
{
    Clear();

    if (!load_pe_header(file)) {
        return;
    }

    // Get Signature
    _cert.Load(file);
}

void CPEFile::Clear() throw()
{    
    _cert.Clear();
    memset(&_ntHeader, 0, sizeof(_ntHeader));
}

CPEFile& CPEFile::operator = (const CPEFile& pe) throw()
{
    if(this != &pe) {
        memcpy(&_ntHeader, pe.GetNtHeaders(), sizeof(IMAGE_NT_HEADERS));
        _cert = pe.GetCert();
    }
    return *this;
}



//
//  class CPECert
//
bool CPECert::Load(_In_ LPCWSTR file) throw()
{
    bool result = false;
    DWORD dwEncoding, dwContentType, dwFormatType;
    HCERTSTORE hStore = NULL;
    HCRYPTMSG hMsg = NULL;
    PCCERT_CONTEXT pCertContext = NULL;


    // Get message handle and store handle from the signed file.
    if(!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                         file,
                         CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                         CERT_QUERY_FORMAT_FLAG_BINARY,
                         0,
                         &dwEncoding,
                         &dwContentType,
                         &dwFormatType,
                         &hStore,
                         &hMsg,
                         NULL))
    {
        return false;
    }

    DWORD dwSignerInfo = 0;
    std::vector<UCHAR> vSignerData;
    PCMSG_SIGNER_INFO pSignerInfo = NULL;
    CERT_INFO CertInfo = {0};

    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &dwSignerInfo)) {
        goto _exit;
    }

    // Allocate memory for signer information.
    vSignerData.resize(dwSignerInfo, 0);
    pSignerInfo = reinterpret_cast<PCMSG_SIGNER_INFO>(vSignerData.data());

    // Get Signer Information.
    if(!CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, (PVOID)pSignerInfo, &dwSignerInfo)) {
        goto _exit;
    }

    memset(&CertInfo, 0, sizeof(CertInfo));
    CertInfo.Issuer = pSignerInfo->Issuer;
    CertInfo.SerialNumber = pSignerInfo->SerialNumber;
    pCertContext = CertFindCertificateInStore(hStore, dwEncoding, 0, CERT_FIND_SUBJECT_CERT,(PVOID)&CertInfo, NULL);
    if(NULL == pCertContext) {
        goto _exit;
    }
    
    nudf::crypto::CCertDecoder::DecodeSubject(pCertContext, _subject);
    nudf::crypto::CCertDecoder::DecodeIssuer(pCertContext, _issuer);
    nudf::crypto::CCertDecoder::DecodeSerial(pCertContext, _serial);
    nudf::crypto::CCertDecoder::DecodeThumbprint(pCertContext, _thumbprint);
    nudf::crypto::CCertDecoder::DecodeAlgorithm(pCertContext, _signatureAlg, _thumbprintAlg);
    nudf::crypto::CCertDecoder::DecodeValidDate(pCertContext, &_validfrom, &_validthrough);

_exit:
    CertFreeCertificateContext(pCertContext);
    CryptMsgClose(hMsg);
    CertCloseStore(hStore, 0);
    return false;
}

void CPECert::Clear() throw()
{
    _thumbprint.clear();
    _subject.clear();
    _issuer.clear();
    _serial.clear();
    _thumbprintAlg.clear();
    _signatureAlg.clear();
    memset(&_validfrom, 0, sizeof(_validfrom));
    memset(&_validthrough, 0, sizeof(_validthrough));
}