

#include <Windows.h>
#include <stdio.h>



#include <nudf\exception.hpp>
#include <nudf\cert.hpp>
#include <nudf\shared\keydef.h>
#include <nudf\string.hpp>
#include <nudf\time.hpp>
#include <nudf\convert.hpp>
#include <nudf\encoding.hpp>
#include <nudf\xmlparser.hpp>


bool GetAgentInformation(_In_ const std::wstring& profile, _Out_ std::wstring& agentId, _Out_ nudf::crypto::CRsaPubKeyBlob& agentKey);
bool GenerateSessionKey(_In_ const std::wstring& agentId, _In_ nudf::crypto::CRsaPubKeyBlob& agentKey, _Out_ nudf::crypto::CAesKeyBlob& sessionKey);
bool DecryptKeyBundle(_In_ const std::wstring& keybundle, _In_ const std::wstring& keybundlexml, _In_ nudf::crypto::CAesKeyBlob& sessionKey);

void usage(const wchar_t* app)
{
    printf("KeyDecrypt Usage:\n");
    printf("   %s <KEYFILE>\n", app);
}

int wmain(int argc, wchar_t** argv)
{
    if(argc != 2) {
        usage(argv[0]);
        return -1;
    }

    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(argv[1])) {
        printf("Cannot file target key bundle file.\n");
        return ERROR_NOT_FOUND;
    }

    std::wstring wsKeyBundle = argv[1];
    std::wstring wsAgentProfile;
    std::wstring wsKeyBundleXml;

    WCHAR wzDir[MAX_PATH] = {0};
    wcsncpy_s(wzDir, MAX_PATH, argv[1], _TRUNCATE);
    WCHAR* pos = wcsrchr(wzDir, L'\\');
    if(NULL != pos) {
        *(pos+1) = L'\0';
        wsAgentProfile = wzDir;
        wsKeyBundleXml = wzDir;
    }
    wsAgentProfile += L"profile.xml";
    wsKeyBundleXml += L"keybundle.xml";

    std::wstring agentId;
    nudf::crypto::CRsaPubKeyBlob agentKey;
    nudf::crypto::CAesKeyBlob sessionKey;

    nudf::crypto::Initialize();

    if(!GetAgentInformation(wsAgentProfile, agentId, agentKey)) {
        return -1;
    }
    if(!GenerateSessionKey(agentId, agentKey, sessionKey)) {
        return -1;
    }
    if(!DecryptKeyBundle(wsKeyBundle, wsKeyBundleXml, sessionKey)) {
        return -1;
    }

    return 0;
}

bool GetAgentInformation(_In_ const std::wstring& profile, _Out_ std::wstring& agentId, _Out_ nudf::crypto::CRsaPubKeyBlob& agentKey)
{
    bool bRet = false;

    try {

        nudf::util::CXmlDocument doc;

        try {
            doc.LoadFromFile(profile.c_str());
        }
        catch(const nudf::CException& e) {
            printf("Fail to load profile.\n");
            throw e;
        }

        CComPtr<IXMLDOMElement> spBundle;
        CComPtr<IXMLDOMNode> spProfile;
        CComPtr<IXMLDOMNode> spAgentId;
        CComPtr<IXMLDOMNode> spCert;
        std::wstring wsRoot;
        std::wstring wsAgentId;
        std::wstring wsAgentCertficate;

        if(!doc.GetDocRoot(&spBundle)) {
            printf("Fail to find BUNDLE element.\n");
            throw WIN32ERROR();
        }
        wsRoot = doc.GetNodeName(spBundle);
        if(0 != _wcsicmp(wsRoot.c_str(), L"Bundle")) {
            printf("Fail to find BUNDLE element.\n");
            throw WIN32ERROR();
        }
        if(!doc.FindChildElement(spBundle, L"Profile", &spProfile)) {
            printf("Fail to find PROFILE element.\n");
            throw WIN32ERROR();
        }
        if(!doc.FindChildElement(spProfile, L"AgentId", &spAgentId)) {
            printf("Fail to find AgentId element.\n");
            throw WIN32ERROR();
        }
        wsAgentId = doc.GetNodeText(spAgentId);
        if(wsAgentId.empty()) {
            printf("AgentId is empty.\n");
            throw WIN32ERROR();
        }
        if(!doc.FindChildElement(spProfile, L"AgentCertificate", &spCert)) {
            printf("Fail to find AgentCertificate element.\n");
            throw WIN32ERROR();
        }
        wsAgentCertficate = doc.GetNodeText(spCert);
        if(wsAgentCertficate.empty()) {
            printf("Agent Certficate is empty.\n");
            throw WIN32ERROR();
        }
        
        nudf::crypto::CX509CertContext context;
        HRESULT hr = context.Create(wsAgentCertficate.c_str());
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }
        hr = context.GetPublicKeyBlob(agentKey);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        agentId = wsAgentId;
        bRet = true;
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        bRet = false;
    }

    return bRet;
}

bool GenerateSessionKey(_In_ const std::wstring& agentId, _In_ nudf::crypto::CRsaPubKeyBlob& agentKey, _Out_ nudf::crypto::CAesKeyBlob& sessionKey)
{
    HRESULT hr = S_OK;
    std::vector<UCHAR>  signature;
    std::vector<UCHAR>  key;

    if(agentId.empty()) {
        printf("Invalid agent Id\n");
        return false;
    }
    if(NULL == agentKey.GetBlob()) {
        printf("Invalid agent certificate public key\n");
        return false;
    }
        
    if(!nudf::crypto::ToSha256(agentKey.GetBlob(), agentKey.GetBlobSize(), key) || key.size()!=32) {
        printf("Fail to get SHA1 hash code\n");
        return false;
    }

    sessionKey.Clear();
    sessionKey.SetKey(&key[0], 32);
    return true;
}

bool DecryptKeyBundle(_In_ const std::wstring& keybundle, _In_ const std::wstring& keybundlexml, _In_ nudf::crypto::CAesKeyBlob& sessionKey)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwVDL  = 0;
    DWORD dwRead = 0;
    char  szMagic[32] = {0};
    std::vector<UCHAR> magic;
    bool bRet = false;
    
    static const char CONF_MAGIC[32] = {'N', 'e', 'x', 't', 'L', 'a', 'b', 's', ' ', 'C', 'o', 'n', 'f', 'i', 'g', 'u',
                                        'r', 'e', ' ', 'F', 'i', 'l', 'e',   0,   0,   0,   0,   0,   0,   0,   0,   0};

    hFile = ::CreateFileW(keybundle.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(INVALID_HANDLE_VALUE == hFile) {
        printf("Fail to open key bundle file (%d)\n", GetLastError());
        return false;
    }

    DWORD dwSize = GetFileSize(hFile, NULL);
    if(dwSize < sizeof(ULONG)) {
        printf("Invalid key bundle file size (%d)\n", dwSize);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    // Check Magic
    if(!::ReadFile(hFile, szMagic, 32, &dwRead, NULL)) {
        printf("Fail to read magic header (%d)\n", GetLastError());
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return false;
    }
    if(!nudf::crypto::AesDecrypt(sessionKey, szMagic, 32, 0, magic)) {
        printf("Fail to decrypt magic header\n");
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    if(0 != memcmp(&magic[0], CONF_MAGIC, 32)) {
        printf("Invalid magic header\n");
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    // Get VDL
    if(!::ReadFile(hFile, &dwVDL, (ULONG)sizeof(ULONG), &dwRead, NULL)) {
        printf("Fail to read VDL (%d)\n", GetLastError());
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        return false;
    }

    if(0 != dwVDL) {

        std::vector<UCHAR> cipher;
        std::vector<UCHAR> data;

        cipher.resize(dwSize-36, 0);
        if(!::ReadFile(hFile, &cipher[0], dwSize-36, &dwRead, NULL)) {
            printf("Fail to read cipher content (%d)\n", GetLastError());
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
            return false;
        }
        if(!nudf::crypto::AesDecrypt(sessionKey, &cipher[0], dwSize-36, 0, data)) {
            printf("Fail to decrypt cipher content (%d)\n", GetLastError());
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
            return false;
        }

        const NXRM_KEY_BLOB* pk = (const NXRM_KEY_BLOB*)&data[0];
        int count = (int)(dwVDL / sizeof(NXRM_KEY_BLOB));

        try {
            nudf::util::CXmlDocument doc;
            CComPtr<IXMLDOMNode> spBundle;

            doc.Create();
            doc.AppendChildElement(L"KeyBundle", &spBundle);

            for(int i=0; i<count; i++) {
                CComPtr<IXMLDOMNode> spKey;
                CComPtr<IXMLDOMNode> spKeyRing;
                CComPtr<IXMLDOMNode> spKeyHash;
                CComPtr<IXMLDOMNode> spKeyTime;
                CComPtr<IXMLDOMNode> spKeyData;
                char szKeyRing[9] = {0};
                memcpy(szKeyRing, pk[i].KeKeyId.Id, 8); szKeyRing[8] = '\0';
                std::string sKeyRing = szKeyRing;
                std::wstring wsKeyRing(sKeyRing.begin(), sKeyRing.end());

                std::wstring wsHash = nudf::string::FromBytes<wchar_t>(&pk[i].KeKeyId.Id[8], 32);
                std::wstring wsKeyData = nudf::string::FromBytes<wchar_t>(pk[i].Key, (NXRM_ALGORITHM_AES256 == pk[i].KeKeyId.Algorithm) ? 32 : 16);
                long lTimestamp = 0;
                memcpy(&lTimestamp, &pk[i].KeKeyId.Id[40], 4);

                WCHAR wzTimestamp[128] = {0};
                nudf::time::CTime tm;
                SYSTEMTIME st = {0};
                tm.FromSecondsSince1970Jan1st(lTimestamp);
                tm.ToSystemTime(&st);
                swprintf_s(wzTimestamp, L"%08X (%04d-%02d-%02dT%02d:%02d:%02d.%03dZ)", lTimestamp, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

                doc.AppendChildElement(spBundle, L"KEY", &spKey);
                doc.AppendChildElement(spKey, L"KeyRing", &spKeyRing);
                doc.SetNodeText(spKeyRing, wsKeyRing);
                doc.AppendChildElement(spKey, L"Hash", &spKeyHash);
                doc.SetNodeText(spKeyHash, wsHash);
                doc.AppendChildElement(spKey, L"Timestamp", &spKeyTime);
                doc.SetNodeText(spKeyTime, wzTimestamp);
                doc.AppendChildElement(spKey, L"KeyData", &spKeyData);
                doc.SetNodeText(spKeyData, wsKeyData);
            }

            ::DeleteFileW(keybundlexml.c_str());
            doc.SaveToFile(keybundlexml.c_str());
            bRet = true;
        }
        catch(const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e);
            printf("Fail to save keys to XML file\n");
            bRet = false;
        }
    }

    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;
    return bRet;
}