
#ifndef __NXRM_ENGINE_AUDIT_H__
#define __NXRM_ENGINE_AUDIT_H__

#include <atlbase.h>
#include <atlcomcli.h>
#include <string>
#include <nudf\xmlparser.hpp>
#include <nudf\time.hpp>



#define MAX_AUDIT_ITEM_COUNT    1024

namespace nxrm {
namespace engine {


class CAuditItem
{
public:
    CAuditItem();
    CAuditItem(_In_ int agentId);
    virtual ~CAuditItem();

    void Initialize(_In_ int agentId);
    void GetStringTimestamp(_Out_ std::wstring& st) const throw();
    void GetStringRights(_Out_ std::wstring& sr) const throw();
    CAuditItem& operator = (const CAuditItem& ai) throw();

    inline const __int64& GetUid() const throw() {return _uid;}
    inline const nudf::time::CTime& GetTimestamp() const throw() {return _timestamp;}
    inline const std::wstring& GetType() const throw() {return _type;}
    inline const ULONGLONG& GetRights() const throw() {return _rights;}
    inline const std::wstring& GetOperation() const throw() {return _operation;}
    inline const std::vector<std::pair<std::wstring,std::wstring>>& GetEnvs() const throw() {return _envs;}
    inline const std::wstring& GetAuditUserName() const throw() {return _userName;}
    inline const std::wstring& GetAuditUserSid() const throw() {return _userSid;}
    inline const std::wstring& GetAuditUserContext() const throw() {return _userContext;}
    inline const std::vector<std::pair<std::wstring,std::wstring>>& GetAuditUserAttributes() const throw() {return _userAttrs;}
    inline const std::wstring& GetAuditHostName() const throw() {return _hostName;}
    inline const std::wstring& GetAuditHostIpv4() const throw() {return _hostIpv4;}
    inline const std::wstring& GetAppImage() const throw() {return _appImage;}
    inline const std::wstring& GetAppPublisher() const throw() {return _appPublisher;}
    inline const std::wstring& GetResPath() const throw() {return _resPath;}
    inline const std::vector<std::pair<std::wstring,std::wstring>>& GetResTags() const throw() {return _resTags;}
    inline const std::vector<std::pair<int,std::wstring>>& GetEvalPolicies() const throw() {return _evalPolicies;}

    inline void SetType(_In_ const std::wstring& type) throw() {_type = type;}
    inline void SetRights(_In_ ULONGLONG rights) throw() {_rights = rights;}
    inline void SetOperation(_In_ const std::wstring& operation) throw() {_operation = operation;}
    inline void AddEnvVariable(_In_ const std::wstring& name, _In_ const std::wstring& value) throw() {_envs.push_back(std::pair<std::wstring,std::wstring>(name,value));}
    inline void SetAuditUserName(_In_ const std::wstring& name) throw() {_userName = name;}
    inline void SetAuditUserSid(_In_ const std::wstring& id) throw() {_userSid = id;}
    inline void SetAuditUserContext(_In_ const std::wstring& context) throw() {_userContext = context;}
    inline void AddAuditUserAttributes(_In_ const std::wstring& name, _In_ const std::wstring& value) throw() {_userAttrs.push_back(std::pair<std::wstring,std::wstring>(name,value));}
    inline void SetAuditHostName(_In_ const std::wstring& name) throw() {_hostName = name;}
    inline void SetAuditHostIpv4(_In_ const std::wstring& ip) throw() {_hostIpv4 = ip;}
    inline void SetAppImage(_In_ const std::wstring& image) throw() {_appImage = image;}
    inline void SetAppPublisher(_In_ const std::wstring& publisher) throw() {_appPublisher = publisher;}
    inline void SetResPath(_In_ const std::wstring& path) throw() {_resPath = path;}
    inline void AddResTag(_In_ const std::wstring& name, _In_ const std::wstring& value) throw() {_resTags.push_back(std::pair<std::wstring,std::wstring>(name,value));}
    inline void AddEvalPolicy(_In_ int id, _In_ const std::wstring& value) throw() {_evalPolicies.push_back(std::pair<int,std::wstring>(id,value));}

private:
    __int64         _uid;
    nudf::time::CTime   _timestamp;
    std::wstring    _type;
    ULONGLONG       _rights;
    std::wstring    _operation;
    std::vector<std::pair<std::wstring,std::wstring>>   _envs;
    std::wstring    _userName;
    std::wstring    _userSid;
    std::wstring    _userContext;
    std::vector<std::pair<std::wstring,std::wstring>>   _userAttrs;
    std::wstring    _hostName;
    std::wstring    _hostIpv4;
    std::wstring    _appImage;
    std::wstring    _appPublisher;
    std::wstring    _resPath;
    std::vector<std::pair<std::wstring,std::wstring>>   _resTags;
    std::vector<std::pair<int,std::wstring>>   _evalPolicies;
};

class CAuditObject : public nudf::util::CXmlDocument
{
public:
    CAuditObject();
    virtual ~CAuditObject();
    
    virtual void Create(_In_ const std::wstring& tenantId, _In_ const std::wstring& agentId);
    virtual void LoadFromFile(_In_ LPCWSTR file);
    virtual void LoadFromXml(_In_ LPCWSTR xml);
    virtual void Close() throw();

    void InsertAuditItem(const CAuditItem& ai);
    void Dump(_In_ const std::wstring& file);

    inline long GetCount() const throw() {return _count;}

protected:
    bool GetCountFromXml() throw();

private:
    long                 _count;
    CRITICAL_SECTION     _lock;
    CComPtr<IXMLDOMNode> _logs;  
};



}
}

#endif