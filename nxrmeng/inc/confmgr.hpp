
#ifndef __NXRM_ENGINE_FILE_MGR_H__
#define __NXRM_ENGINE_FILE_MGR_H__


#include <nudf\shared\keydef.h>
#include <nudf\exception.hpp>
#include <nudf\crypto.hpp>
#include <nudf\thread.hpp>
#include <nudf\xmlparser.hpp>

namespace nxrm {
namespace engine {



class CSecureFile
{
public:
    CSecureFile();
    virtual ~CSecureFile();

protected:
    void Encrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& source_file, _In_ const std::wstring& target_file);
    void Encrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_opt_ const void* pb, _In_ const ULONG cb, _In_ const std::wstring& target_file);
    void Decrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& source_file, _In_ const std::wstring& target_file);
    void Decrypt(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& source_file, _Out_ std::vector<UCHAR>& target);
};

class CConfFile : public CSecureFile
{
public:
    CConfFile();
    virtual ~CConfFile();

    void GenerateTempFile(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ const std::wstring& tempdir);
    void FreeTempFile();

    const std::wstring& GetSource() const throw() {return _file;}
    const std::wstring& GetTemp() const throw() {return _tempfile;}
    
protected:
private:
    std::wstring    _file;
    std::wstring    _tempfile;
    HANDLE          _h;
};

class CKeyBundle : public CSecureFile
{
public:
    CKeyBundle(){}
    virtual ~CKeyBundle(){}

    CKeyBundle& operator = (const CKeyBundle& bundle) throw();

    void Load(_In_ const nudf::crypto::CAesKeyBlob& key);
    void Load(_In_ IXMLDOMNode* pNode);
    void Save(_In_ const nudf::crypto::CAesKeyBlob& key);

    inline const NXRM_KEY_BLOB* GetKeyBlobs() const throw() {return (0 != (_blob.size()/sizeof(NXRM_KEY_BLOB))) ? ((const NXRM_KEY_BLOB*)(&_blob[0])) : NULL;}
    inline ULONG GetKeyBlobSize() const throw() {return (ULONG)((_blob.size()/sizeof(NXRM_KEY_BLOB)) * sizeof(NXRM_KEY_BLOB));}
    inline const std::vector<UCHAR>& GetHash() const throw() {return _hash;}
    inline const std::vector<UCHAR>& GetData() const throw() {return _blob;}
    inline void Clear() throw() {_hash.clear(); _blob.clear();}
    inline void ClearBlob() throw() {_blob.clear();}

private:
    std::vector<UCHAR>  _hash;
    std::vector<UCHAR>  _blob;
};

class CPolicyBundle : public CSecureFile
{
public:
    CPolicyBundle()
    {
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime = 0;
    }
    virtual ~CPolicyBundle(){}

    CPolicyBundle& operator = (const CPolicyBundle& bundle) throw();

    void Load(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ bool orig=false);
    void Load(_In_ IXMLDOMNode* pNode, _In_ bool orig=false);
    void Save(_In_ const nudf::crypto::CAesKeyBlob& key, _In_ bool orig=false);
    void SaveAsPlainFile(_In_ const std::wstring& file);

    inline const std::wstring& GetXml() const throw() {return _xml;}
    inline const FILETIME& GetTimestamp() const throw() {return _timestamp;}
    inline ULONGLONG GetTimestampUll() const throw() {return *((ULONGLONG*)(&_timestamp));}
    inline void Clear() throw() {_xml.clear();}

private:
    std::wstring    _xml;
    FILETIME        _timestamp;
};

class CClsConfFile : public CConfFile
{
public:
    CClsConfFile(){}
    virtual ~CClsConfFile(){}
};

class CPolicyConfFile : public CConfFile
{
public:
    CPolicyConfFile(){}
    virtual ~CPolicyConfFile(){}
};

       
}   // namespace engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_FILE_MGR_H__