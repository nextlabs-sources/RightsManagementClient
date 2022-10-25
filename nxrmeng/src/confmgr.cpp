

#include <Windows.h>
#include <assert.h>

#include <string>
#include <list>
#include <vector>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\encoding.hpp>

#include "nxrmeng.h"
#include "engine.hpp"
#include "confmgr.hpp"


using namespace nxrm::engine;



static const char CONF_MAGIC[32] = {'N', 'e', 'x', 't', 'L', 'a', 'b', 's', ' ', 'C', 'o', 'n', 'f', 'i', 'g', 'u',
                                    'r', 'e', ' ', 'F', 'i', 'l', 'e',   0,   0,   0,   0,   0,   0,   0,   0,   0};

CSecureFile::CSecureFile()
{
}

CSecureFile::~CSecureFile()
{
}

void CSecureFile::Encrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& source_file, _In_ const std::wstring& target_file)
{
    HANDLE hSource = INVALID_HANDLE_VALUE;

    try {

        hSource = ::CreateFileW(source_file.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hSource) {
            throw WIN32ERROR();
        }

        DWORD dwSourceSize = GetFileSize(hSource, NULL);

        if(0 == dwSourceSize) {
            Encrypt(key, NULL, 0, target_file);
        }
        else {
            const DWORD bufsize = (dwSourceSize + 31) & ~(31);
            std::vector<UCHAR> buf;
            DWORD dwRead = 0;

            buf.resize(bufsize, 0);
            if(!::ReadFile(hSource, &buf[0], dwSourceSize, &dwRead, NULL)) {
                throw WIN32ERROR();
            }
            Encrypt(key, &buf[0], bufsize, target_file);
        }
        
        CloseHandle(hSource);
        hSource = INVALID_HANDLE_VALUE;
    }
    catch(const nudf::CException& e) {

        if(INVALID_HANDLE_VALUE != hSource) {
            CloseHandle(hSource);
            hSource = INVALID_HANDLE_VALUE;
        }
        throw e;
    }
}

void CSecureFile::Encrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_opt_ const void* pb, _In_ const ULONG cb, _In_ const std::wstring& target_file)
{
    HANDLE hTarget = INVALID_HANDLE_VALUE;



    try {

        DWORD dwWritten = 0;
        std::vector<UCHAR> magic;

        if(!nudf::crypto::AesEncrypt(key, CONF_MAGIC, 32, 0, magic)) {
            throw WIN32ERROR();
        }

#ifdef _DEBUG
        std::wstring wsTmp = nudf::string::FromBytes<wchar_t>(key.GetKey(), key.GetKeySize());
        LOGSYSINF(L"CSecureFile::Encrypt with Key: %s", wsTmp.c_str());
        wsTmp = nudf::string::FromBytes<wchar_t>(&magic[0], (ULONG)magic.size());
        LOGSYSINF(L"CSecureFile::Encrypt magic \"%S\": %s", CONF_MAGIC, wsTmp.c_str());
#endif


        hTarget = ::CreateFileW(target_file.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hTarget) {
            throw WIN32ERROR();
        }
        
        // Write Magic
        if(!::WriteFile(hTarget, &magic[0], 32, &dwWritten, NULL)) {
            throw WIN32ERROR();
        }
        // Write VDL
        if(!::WriteFile(hTarget, &cb, (ULONG)sizeof(ULONG), &dwWritten, NULL)) {
            throw WIN32ERROR();
        }
        // Write File Content
        if(0 != cb && NULL != pb) {

            std::vector<UCHAR> buf;
            std::vector<UCHAR> cipher;
            const DWORD bufsize = (cb + 31) & ~(31);
            DWORD dwRead = 0;

            if(cb != bufsize) {
                buf.resize(bufsize, 0);
                memcpy(&buf[0], pb, cb);
                pb = &buf[0];
            }
            
            if(!nudf::crypto::AesEncrypt(key, pb, bufsize, 0, cipher)) {
                throw WIN32ERROR();
            }
            if(!::WriteFile(hTarget, &cipher[0], (ULONG)cipher.size(), &dwWritten, NULL)) {
                throw WIN32ERROR();
            }
        }
        
        CloseHandle(hTarget);
        hTarget = INVALID_HANDLE_VALUE;
    }
    catch(const nudf::CException& e) {

        if(INVALID_HANDLE_VALUE != hTarget) {
            CloseHandle(hTarget);
            hTarget = INVALID_HANDLE_VALUE;
            ::DeleteFileW(target_file.c_str());
        }
        throw e;
    }
}

void CSecureFile::Decrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& source_file, _In_ const std::wstring& target_file)
{
    HANDLE hTarget = INVALID_HANDLE_VALUE;

    try {

        DWORD dwWritten = 0;
        std::vector<UCHAR> target;

        Decrypt(key, source_file, target);

        hTarget = ::CreateFileW(target_file.c_str(), GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hTarget) {
            throw WIN32ERROR();
        }
        
        // Write File Content
        if(0 != target.size()) {

            if(!::WriteFile(hTarget, &target[0], (ULONG)target.size(), &dwWritten, NULL)) {
                throw WIN32ERROR();
            }
        }
        
        CloseHandle(hTarget);
        hTarget = INVALID_HANDLE_VALUE;
    }
    catch(const nudf::CException& e) {

        if(INVALID_HANDLE_VALUE != hTarget) {
            CloseHandle(hTarget);
            hTarget = INVALID_HANDLE_VALUE;
            ::DeleteFileW(target_file.c_str());
        }
        throw e;
    }
}

void CSecureFile::Decrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& source_file, _Out_ std::vector<UCHAR>& target)
{
    HANDLE hSource = INVALID_HANDLE_VALUE;

    try {

        DWORD dwVDL  = 0;
        DWORD dwRead = 0;
        char  szMagic[32] = {0};
        std::vector<UCHAR> magic;


        hSource = ::CreateFileW(source_file.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hSource) {
            throw WIN32ERROR();
        }

        DWORD dwSourceSize = GetFileSize(hSource, NULL);
        if(dwSourceSize < 36 || 0 != ((dwSourceSize-36)%32)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Check Magic
        if(!::ReadFile(hSource, szMagic, 32, &dwRead, NULL)) {
            throw WIN32ERROR();
        }
        if(!nudf::crypto::AesDecrypt(key, szMagic, 32, 0, magic)) {
            throw WIN32ERROR();
        }

#ifdef _DEBUG
        std::wstring wsTmp = nudf::string::FromBytes<wchar_t>(key.GetKey(), key.GetKeySize());
        LOGSYSINF(L"CSecureFile::Decrypt with Key: %s", wsTmp.c_str());
        wsTmp = nudf::string::FromBytes<wchar_t>((const UCHAR*)szMagic, 32);
        LOGSYSINF(L"CSecureFile::Decrypt magic \"%s\": %S", wsTmp.c_str(), (const char*)(&magic[0]));
#endif

        assert(magic.size() == 32);
        if(0 != memcmp(&magic[0], CONF_MAGIC, 32)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Get VDL
        if(!::ReadFile(hSource, &dwVDL, (ULONG)sizeof(ULONG), &dwRead, NULL)) {
            throw WIN32ERROR();
        }

        if(0 != dwVDL) {

            std::vector<UCHAR> cipher;
            std::vector<UCHAR> data;

            cipher.resize(dwSourceSize-36, 0);
            if(!::ReadFile(hSource, &cipher[0], dwSourceSize-36, &dwRead, NULL)) {
                throw WIN32ERROR();
            }
            if(!nudf::crypto::AesDecrypt(key, &cipher[0], dwSourceSize-36, 0, data)) {
                throw WIN32ERROR();
            }
            target.resize(dwVDL, 0);
            memcpy(&target[0], &data[0], dwVDL);
        }
        
        CloseHandle(hSource);
        hSource = INVALID_HANDLE_VALUE;
    }
    catch(const nudf::CException& e) {

        if(INVALID_HANDLE_VALUE != hSource) {
            CloseHandle(hSource);
            hSource = INVALID_HANDLE_VALUE;
        }
        throw e;
    }
}


CConfFile::CConfFile() : _h(INVALID_HANDLE_VALUE)
{
}

CConfFile::~CConfFile()
{
    FreeTempFile();
}

void CConfFile::GenerateTempFile(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& tempdir)
{
    WCHAR wzTempFile[MAX_PATH] = {0};

    if(0 == GetTempFileNameW(tempdir.c_str(), L"N", 0, wzTempFile)) {
        throw WIN32ERROR();
    }

    HANDLE hTemp = INVALID_HANDLE_VALUE;

    try {

        Decrypt(key, _file, wzTempFile);

        hTemp = ::CreateFileW(wzTempFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hTemp) {
            throw WIN32ERROR();
        }
        
        FreeTempFile();
        _h = hTemp;
        _tempfile = wzTempFile;
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CConfFile::FreeTempFile()
{
    if(_h != INVALID_HANDLE_VALUE) {
        CloseHandle(_h);
        _h = INVALID_HANDLE_VALUE;
    }
    ::DeleteFileW(_tempfile.c_str());
    _tempfile.clear();
}



//
//  class CKeyConf
//

CKeyBundle& CKeyBundle::operator = (const CKeyBundle& bundle) throw()
{
    if(this != &bundle) {
        _hash = bundle.GetHash();
        _blob = bundle.GetData();
    }
    return *this;
}

void CKeyBundle::Load(_In_ const nudf::crypto::CAesKeyBlob& key)
{
    if(key.GetKeyBitsLength() != 256) {
        throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
    }

    std::vector<UCHAR> data;
    size_t blobsize = 0;

    Decrypt(key, _nxPaths.GetKeyConf(), data);
    if(data.size() < sizeof(NXRM_KEY_BLOB) || 0 != (data.size() % sizeof(NXRM_KEY_BLOB))) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    blobsize = data.size();
    if(0 == blobsize) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    _blob = data;
    _hash.resize(16, 0);
    nudf::crypto::ToMd5(&_blob[0], (ULONG)_blob.size(), _hash);
}

void CKeyBundle::Load(_In_ IXMLDOMNode* pNode)
{
    HRESULT hr = S_OK;
    std::wstring name;
    std::list<std::pair<ULONGLONG,std::vector<UCHAR>>> keys;

    name = nudf::util::XmlUtil::GetNodeName(pNode);
    if(0 != _wcsicmp(L"keyRings", name.c_str())) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    CComPtr<IXMLDOMNodeList>    spKeyRingList;
    long                        nChild = 0;
    hr = pNode->get_childNodes(&spKeyRingList);
    if(FAILED(hr)) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    hr = spKeyRingList->get_length(&nChild);
    if(FAILED(hr) || 0 == nChild) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    for(int i=0; i<nChild; i++) {

        CComPtr<IXMLDOMNode> spKeyRing;
        std::wstring wsKeyRingName;

        hr = spKeyRingList->get_item(i, &spKeyRing);
        if(FAILED(hr)) {
            break;
        }
        name = nudf::util::XmlUtil::GetNodeName(spKeyRing);
        if(0 != _wcsicmp(L"keyRing", name.c_str())) {
            continue;
        }

        if(!nudf::util::XmlUtil::GetNodeAttribute(spKeyRing, L"KeyRingName", wsKeyRingName)) {
            continue;
        }

        CComPtr<IXMLDOMNodeList>    spKeyList;
        long                        nKeyChild = 0;
        hr = spKeyRing->get_childNodes(&spKeyList);
        if(FAILED(hr)) {
            break;
        }
        hr = spKeyList->get_length(&nKeyChild);
        if(FAILED(hr) || 0 == nKeyChild) {
            break;
        }

        for(int j=0; j<nKeyChild; j++) {
            
            CComPtr<IXMLDOMNode> spKey;

            hr = spKeyList->get_item(j, &spKey);
            if(FAILED(hr)) {
                break;
            }
            name = nudf::util::XmlUtil::GetNodeName(spKey);
            if(0 != _wcsicmp(L"key", name.c_str())) {
                continue;
            }

            // We have a key element now
            CComPtr<IXMLDOMNode> spKeyId;
            CComPtr<IXMLDOMNode> spKeyData;
            CComPtr<IXMLDOMNode> spTimestamp;

            if(!nudf::util::XmlUtil::FindChildElement(spKey, L"KeyId", &spKeyId)) {
                continue;
            }
            if(!nudf::util::XmlUtil::FindChildElement(spKey, L"KeyData", &spKeyData)) {
                continue;
            }
            if(!nudf::util::XmlUtil::FindChildElement(spKey, L"TimeStamp", &spTimestamp)) {
                continue;
            }

            std::wstring wsKeyId = nudf::util::XmlUtil::GetNodeText(spKeyId);
            std::wstring wsKeyData = nudf::util::XmlUtil::GetNodeText(spKeyData);
            std::wstring wsTimeStamp = nudf::util::XmlUtil::GetNodeText(spTimestamp);
            if(wsKeyData.empty() || wsTimeStamp.empty()) {
                continue;
            }

            std::vector<UCHAR> kId;
            std::vector<UCHAR> kData;
            FILETIME           fTime = {0, 0};

            if(!nudf::util::encoding::Base64Decode<wchar_t>(wsKeyId, kId) || kId.size()!=32) {
                continue;
            }
            if(!nudf::util::encoding::Base64Decode<wchar_t>(wsKeyData, kData)) {
                continue;
            }
            // Only support AES 256 in this release
            if(32 != kData.size()) {
                continue;
            }
            if(!nudf::string::ToSystemTime<wchar_t>(wsTimeStamp, &fTime, NULL)) {
                continue;
            }

            nudf::time::CTime timestamp(&fTime);
            LONG dosTime = (LONG)timestamp.ToSecondsSince1970Jan1st();

            std::string sKeyRingName(wsKeyRingName.begin(), wsKeyRingName.end());
            std::vector<UCHAR> blob;
            blob.resize(sizeof(NXRM_KEY_BLOB), 0);
            NXRM_KEY_BLOB* keyblob = (NXRM_KEY_BLOB*)&blob[0];
            keyblob->KeKeyId.Algorithm = (32 == kData.size()) ? NXRM_ALGORITHM_AES256 : NXRM_ALGORITHM_AES128;
            keyblob->KeKeyId.IdSize = (ULONG)(8 + kId.size() + 4);
            memcpy(&keyblob->KeKeyId.Id[0], sKeyRingName.c_str(), min(8, sKeyRingName.length()));
            memcpy(&keyblob->KeKeyId.Id[8], &kId[0], kId.size());
            memcpy(&keyblob->KeKeyId.Id[8+kId.size()], &dosTime, sizeof(LONG));
            memcpy(keyblob->Key, &kData[0], kData.size());

            keys.push_back(std::pair<ULONGLONG, std::vector<UCHAR>>(timestamp.ToUll(), blob));
        }
    }

    if(keys.empty()) {
        return;
    }

    // latest key in front
    keys.sort([] (const std::pair<ULONGLONG, std::vector<UCHAR>>& it1, const std::pair<ULONGLONG, std::vector<UCHAR>>& it2) -> bool {return (it1.first > it2.first);});

    // Set _blob
    _blob.resize(keys.size() * sizeof(NXRM_KEY_BLOB), 0);
    NXRM_KEY_BLOB* pBlob = (NXRM_KEY_BLOB*)(&_blob[0]);
    for(std::list<std::pair<ULONGLONG,std::vector<UCHAR>>>::const_iterator it=keys.begin(); it!=keys.end(); ++it) {
        assert(sizeof(NXRM_KEY_BLOB) == (*it).second.size());
        memcpy(pBlob, &(*it).second[0], sizeof(NXRM_KEY_BLOB));
        // Move to next
        pBlob++;
    }

    // Set Hash
    _hash.resize(16, 0);
    nudf::crypto::ToMd5(&_blob[0], (ULONG)_blob.size(), _hash);
}

void CKeyBundle::Save(_In_ const nudf::crypto::CAesKeyBlob& key)
{
    std::vector<UCHAR> data;

    if(key.GetKeyBitsLength() != 256) {
        throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
    }
    if(_hash.size() != 16 || 0 == _blob.size() || 0 != (_blob.size() % sizeof(NXRM_KEY_BLOB))) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    Encrypt(key, &_blob[0], (ULONG)_blob.size(), _nxPaths.GetKeyConf());
}


CPolicyBundle& CPolicyBundle::operator = (const CPolicyBundle& bundle) throw()
{
    if(this != &bundle) {
        _xml = bundle.GetXml();
    }
    return *this;
}

void CPolicyBundle::Load(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ bool orig)
{
    if(key.GetKeyBitsLength() != 256) {
        throw WIN32ERROR2(ERROR_INVALID_PARAMETER);
    }

    std::vector<UCHAR> data;
    std::wstring wsFile = _nxPaths.GetPolicyConf();

    if(orig) {
        wsFile += L".orig";
    }

    Decrypt(key, wsFile, data);
    if(0 != (data.size() % sizeof(WCHAR))) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }

    if(0 == data.size()) {
        // No data
        return;
    }

    if(0 != data[data.size()-1] || 0 != data[data.size()-2]) {
        // Not end with L'\0'
        _xml = std::wstring((const WCHAR*)(&data[0]), data.size()/2);
    }
    else {
        _xml = (const WCHAR*)(&data[0]);
    }

    nudf::util::CXmlDocument doc;
    try {
        CComPtr<IXMLDOMElement> spRoot;
        std::wstring rootName;

        doc.LoadFromXml(_xml.c_str());
        if(!doc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        rootName = doc.GetNodeName(spRoot);
        if(0 != _wcsicmp(PROFILE_POLICY_NODENAME, rootName.c_str())) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        if(!doc.GetNodeAttribute(spRoot, L"timestamp", (FILETIME*)&_timestamp, NULL)) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
    }
    catch(const nudf::CException& e) {
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime = 0;
        _xml.clear();
        throw e;
    }
}

void CPolicyBundle::Load(_In_ IXMLDOMNode* pNode, _In_ bool orig)
{
    static const std::wstring wsSidPrefix(L"S-1-5");
    std::wstring rootName;

    rootName = nudf::util::XmlUtil::GetNodeName(pNode);
    if(0 != _wcsicmp(PROFILE_POLICY_NODENAME, rootName.c_str())) {
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(!nudf::util::XmlUtil::GetNodeAttribute(pNode, L"timestamp", (FILETIME*)&_timestamp, NULL)) {
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime = 0;
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    }
    if(!orig) {

        HRESULT hr = S_OK;
        CComPtr<IXMLDOMNodeList>    spList;
        CComPtr<IXMLDOMNode>        spUserGroupNode;
        CComPtr<IXMLDOMNode>        spUserNode;
        long count = 0;

        if(!nudf::util::XmlUtil::FindChildElement(pNode, L"USERGROUPMAP", &spUserGroupNode) || NULL==spUserGroupNode) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        hr = spUserGroupNode->get_childNodes(&spList);
        if(FAILED(hr)) {
            throw WIN32ERROR2(hr);
        }

        hr = spUserGroupNode->get_firstChild(&spUserNode);
        if(SUCCEEDED(hr) && NULL != spUserNode.p) {

            do {

                std::wstring name;
                std::wstring id;
                CComPtr<IXMLDOMNode> spCurNode;

                assert(NULL != spUserNode.p);

                spCurNode = spUserNode;
                spUserNode.Release();
                spCurNode->get_nextSibling(&spUserNode);

                // Is this element a valid SID/UNKNOWNUSER MAP?
                name = nudf::util::XmlUtil::GetNodeName(spCurNode);
                if(0 == _wcsicmp(name.c_str(), USER_NODENAME)
                    && nudf::util::XmlUtil::GetNodeAttribute(spCurNode, USER_ID_ATTRIBUTE, id)
                    && (
                        0 == _wcsicmp(id.c_str(), UNKNOWN_USER_GROUP_NAME)
                        || 0 == _wcsnicmp(id.c_str(), wsSidPrefix.c_str(), wsSidPrefix.length())
                        )
                   ) {
                    continue;
                }
                
                // Invalid Map Element, Remove it
                CComPtr<IXMLDOMNode> spRemovedNode;
                spUserGroupNode->removeChild(spCurNode, &spRemovedNode);

            } while(NULL!=spUserNode.p);
        }
    }
    _xml = nudf::util::XmlUtil::GetNodeXml(pNode);
}

void CPolicyBundle::Save(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ bool orig)
{
    std::wstring wsFile = _nxPaths.GetPolicyConf();
    if(orig) {
        wsFile += L".orig";
    }
    Encrypt(key, _xml.empty() ? NULL : _xml.c_str(), (ULONG)(_xml.empty() ? 0 : ((_xml.length() + 1) * sizeof(WCHAR))), wsFile);
}

void CPolicyBundle::SaveAsPlainFile(_In_ const std::wstring& file)
{
    nudf::util::CXmlDocument doc;
    doc.LoadFromXml(_xml.c_str());
    doc.SaveToFile(file.c_str());
    doc.Close();
}