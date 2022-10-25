
#ifndef __NXRM_ENGINE_RMSUTIL_H__
#define __NXRM_ENGINE_RMSUTIL_H__


#include <string>

#include <nudf\httputil.hpp>
#include <nudf\crypto.hpp>


namespace nxrm {
namespace engine {



class CRmsAgentProfile;

class CRmsServer
{
public:
    CRmsServer() : _port(0){}
    virtual ~CRmsServer() throw(){}

    CRmsServer& operator = (const CRmsServer& server) throw();
    void Load(_In_ const std::wstring& conf);

    inline const std::wstring& GetServer() const throw() {return _server;}
    inline const std::wstring& GetServicePath() const throw() {return _path;}
    inline USHORT GetPort() const throw() {return _port;}

    virtual void OnLoad(_In_ IXMLDOMElement* pRoot);


private:
    std::wstring _server;
    std::wstring _path;
    USHORT       _port;
};

class CRmsRegisterServer : public CRmsServer
{
public:
    CRmsRegisterServer() : CRmsServer() {}
    virtual ~CRmsRegisterServer() throw() {}

    CRmsRegisterServer& operator = (const CRmsRegisterServer& server) throw();

    inline const std::wstring& GetGroupId() const throw() {return _groupId;}
    inline const std::wstring& GetGroupName() const throw() {return _groupName;}
    inline const std::wstring& GetTenantId() const throw() {return _tenantId;}
    inline const std::wstring& GetCert() const throw() {return _cert;}

    virtual void OnLoad(_In_ IXMLDOMElement* pRoot);

private:
    std::wstring _groupId;
    std::wstring _groupName;
    std::wstring _tenantId;
    std::wstring _cert;
};

class CRmsRequest : public nudf::http::CRequest
{
public:
    CRmsRequest() : nudf::http::CRequest(L"POST") {}
    CRmsRequest(_In_ const std::wstring& path) : nudf::http::CRequest(L"POST", path) {}
    virtual ~CRmsRequest(){}
};

class CRmsRegisterService : public CRmsRequest
{
public:
    CRmsRegisterService() : CRmsRequest() {}
    virtual ~CRmsRegisterService(){}
    void Register(_In_ const CRmsRegisterServer& server, _Out_ std::wstring& regdata);
protected:
    void PrepareRegisterInfo(_In_ const CRmsRegisterServer& regserver, _Out_ std::string& reginfo);
};

class CRmsHeartBeatService : public CRmsRequest
{
public:
    CRmsHeartBeatService();
    virtual ~CRmsHeartBeatService();
    void Beat(_In_ const CRmsAgentProfile& profile, _Out_ std::wstring& update);
    void Acknowledge(_In_ const CRmsAgentProfile& profile);

    inline const std::vector<std::pair<std::wstring,std::wstring>>& GetLoggedOnUsersInBeat() const throw() {return _vLoggedonUsers;}

    virtual void Close() throw()
    {
        CRmsRequest::Close();
        _session.Close();
    }

protected:
    void PrepareHeartBeatInfo(_Out_ std::string& reginfo);
    void PrepareAcknowledgeInfo(_Out_ std::string& ackinfo);;

private:
    nudf::http::CSession _session;
    std::vector<std::pair<std::wstring,std::wstring>> _vLoggedonUsers;
};

class CRmsUpdateService : public CRmsRequest
{
public:
    CRmsUpdateService() : CRmsRequest() {}
    virtual ~CRmsUpdateService(){}
    void Update(_In_ const CRmsAgentProfile& profile,  _Out_ std::wstring& new_version, _Out_ std::wstring& download_url, _Out_ std::wstring& checksum);
};

class CRmsLogService : public CRmsRequest
{
public:
    CRmsLogService() : CRmsRequest() {}
    virtual ~CRmsLogService(){}
    void LogAudit(_In_ const CRmsAgentProfile& profile, _In_ const std::wstring& file);
};


class CAgentInfo
{
public:
    CAgentInfo();
    virtual ~CAgentInfo();

    CAgentInfo& operator = (const CAgentInfo& info) throw();
    void Load(_In_ const std::wstring& conf);

    inline const std::wstring& GetVersion() const throw() {return _version;}
    inline const SYSTEMTIME& GetInstallDate() const throw() {return _installdate;}
    inline const SYSTEMTIME& GetLastCheckDate() const throw() {return _lastcheckdate;}

private:
    std::wstring    _version;
    SYSTEMTIME      _installdate;
    SYSTEMTIME      _lastcheckdate;
};


class CRmsAgentProfile
{
public:
    CRmsAgentProfile() : _hbFrequency(0), _logFrequency(0) {}
    virtual ~CRmsAgentProfile() {}

    void LoadFromRegisterData(_In_ const std::wstring& regdata);
    void Load();
    void Save();
    void Clear();

    inline const std::wstring& GetAgentId() const throw() {return _agentId;}
    inline const std::wstring& GetAgentCert() const throw() {return _agentCert;}
    inline const std::wstring& GetRmsServer() const throw() {return _rms_server;}
    inline const std::wstring& GetRmsPath() const throw() {return _rms_path;}
    inline USHORT GetRmsPort() const throw() {return _rms_port;}
    inline ULONG GetHeartBeatFrequency() const throw() {return _hbFrequency;}
    inline ULONG GetLogFrequency() const throw() {return _logFrequency;}
    inline const std::wstring& GetRmsAgentProfileName() const throw() {return _rmsAgentProfileName;}
    inline const std::wstring& GetRmsAgentProfileTimestamp() const throw() {return _rmsAgentProfileTimestamp;}
    inline const std::wstring& GetRmsCommProfileName() const throw() {return _rmsCommProfileName;}
    inline const std::wstring& GetRmsCommProfileTimestamp() const throw() {return _rmsCommProfileTimestamp;}
    inline const nudf::crypto::CRsaPubKeyBlob& GetAgentKey() const throw() {return _agentKey;}
    inline const nudf::crypto::CAesKeyBlob& GetSessionKey() const throw() {return _sessionKey;}
    inline const nudf::crypto::CLegacyRsaPriKeyBlob& GetSignKey() const throw() {return _signKey;}

    inline void SetAgentId(_In_ const std::wstring& id) throw() {_agentId = id;}
    inline void SetAgentCert(_In_ const std::wstring& cert) throw() {_agentCert = cert;}
    inline void SetRmsServer(_In_ const std::wstring& server) throw() {_rms_server = server;}
    inline void SetRmsPath(_In_ const std::wstring& path) throw() {_rms_path = path;}
    inline void SetRmsPort(_In_ USHORT port) throw() {_rms_port = port;}
    inline void SetHeartBeatFrequency(_In_ ULONG frequency) throw() {_hbFrequency = frequency;}
    inline void SetLogFrequency(_In_ ULONG frequency) throw() {_logFrequency = frequency;}
    inline void SetRmsAgentProfileName(_In_ const std::wstring& name) throw() {_rmsAgentProfileName = name;}
    inline void SetRmsAgentProfileTimestamp(_In_ const std::wstring& timestamp) throw() {_rmsAgentProfileTimestamp = timestamp;}
    inline void SetRmsCommProfileName(_In_ const std::wstring& name) throw() {_rmsCommProfileName = name;}
    inline void SetRmsCommProfileTimestamp(_In_ const std::wstring& timestamp) throw() {_rmsCommProfileTimestamp = timestamp;}

    inline CRmsAgentProfile& operator = (const CRmsAgentProfile& profile) throw()
    {
        if(this != &profile) {
            _agentId = profile.GetAgentId();
            _rms_server = profile.GetRmsServer();
            _rms_path = profile.GetRmsPath();
            _rms_port = profile.GetRmsPort();
            _hbFrequency = profile.GetHeartBeatFrequency();
            _logFrequency = profile.GetLogFrequency();
            _rmsAgentProfileName = profile.GetRmsAgentProfileName();
            _rmsAgentProfileTimestamp = profile.GetRmsAgentProfileTimestamp();
            _rmsCommProfileName = profile.GetRmsCommProfileName();
            _rmsCommProfileTimestamp = profile.GetRmsCommProfileTimestamp();
            _agentKey = profile.GetAgentKey();
            _sessionKey = profile.GetSessionKey();
            _signKey = profile.GetSignKey();
        }
    }

protected:
    void GenerateKeys(const std::wstring& cert);
    void SaveCertFile(const std::wstring& cert);
    void GenerateSignKey();
    void GenerateSessionKey();
    void GenerateSessionPhrase(_Out_ std::wstring& phrase);
    void LoadKeys();

private:
    std::wstring    _agentId;
    std::wstring    _agentCert;
    std::wstring    _rms_server;
    std::wstring    _rms_path;
    USHORT          _rms_port;
    ULONG           _hbFrequency;   // In seconds
    ULONG           _logFrequency;  // In seconds
    nudf::crypto::CRsaPubKeyBlob _agentKey;
    nudf::crypto::CAesKeyBlob    _sessionKey;
    nudf::crypto::CLegacyRsaPriKeyBlob _signKey;

private:
    std::wstring    _rmsAgentProfileName;
    std::wstring    _rmsAgentProfileTimestamp;
    std::wstring    _rmsCommProfileName;
    std::wstring    _rmsCommProfileTimestamp;
};

class CAppInfo
{
public:
    CAppInfo() : _rights(0)
    {
    }
    CAppInfo(const std::wstring& name, const std::wstring& publisher, const ULONGLONG rights) : _name(name), _publisher(publisher), _rights(rights)
    {
    }

    virtual ~CAppInfo()
    {
    }

    CAppInfo& operator = (const CAppInfo& app)
    {
        if(this != &app) {
            _name = app.GetName();
            _publisher = app.GetPublisher();
            _rights = app.GetRights();
        }
        return *this;
    }

    bool MatchName(const std::wstring& name) const throw()
    {
        if(_name.empty()) {
            return true;
        }
        return (0 == _wcsicmp(name.c_str(), _name.c_str())) ? true : false;
    }

    bool MatchPublisher(const std::wstring& publisher) const throw()
    {
        if(_publisher.empty()) {
            return true;
        }
        return (0 == _wcsicmp(publisher.c_str(), _publisher.c_str())) ? true : false;
    }

    inline const std::wstring& GetName() const throw() {return _name;}
    inline const std::wstring& GetPublisher() const throw() {return _publisher;}
    inline ULONGLONG GetRights() const throw() {return _rights;}
    inline void SetName(const std::wstring& name) {_name = name;}
    inline void SetPublisher(const std::wstring& publisher) {_publisher = publisher;}
    inline void SetPublisher(const ULONGLONG rights) {_rights = rights;}

private:
    std::wstring    _name;
    std::wstring    _publisher;
    ULONGLONG       _rights;
};

class CRmsAgentClassify
{
public:
    CRmsAgentClassify() {::InitializeCriticalSection(&_whiteappslock);}
    virtual ~CRmsAgentClassify() {::DeleteCriticalSection(&_whiteappslock);}

    inline const std::wstring& GetSignature() const throw() {return _signature;}
    inline const std::wstring& GetXml() const throw() {return _xml;}
    inline void SetXml(const std::wstring& xml) throw() {_xml = xml;}

    inline const std::vector<CAppInfo>& GetAppWhiteList() const throw() {return _whiteapps;}

    CRmsAgentClassify& operator = (const CRmsAgentClassify& cls) throw();
    
    void LoadFromRegisterData(_In_ const std::wstring& regdata, const nudf::crypto::CLegacyRsaPriKeyBlob& key);
    void LoadFromNode(_In_ nudf::util::CXmlDocument* doc, _In_ IXMLDOMNode* pNode, const nudf::crypto::CLegacyRsaPriKeyBlob& key);
    void LoadWhiteAppListNode(_In_ nudf::util::CXmlDocument* doc, _In_ IXMLDOMNode* pNode);
    void Load(const std::wstring& file, const nudf::crypto::CRsaPubKeyBlob& key);
    void Save(const std::wstring& file);
    ULONGLONG CheckAppInWhiteList(_In_ const std::wstring& appName, _In_ const std::wstring& appPublisher) const throw();

private:
    std::wstring Sign(_In_ const std::wstring& xml, _In_ const nudf::crypto::CLegacyRsaPriKeyBlob& key);

private:
    std::wstring    _xml;
    std::wstring    _signature;
    std::vector<CAppInfo> _whiteapps;
    mutable CRITICAL_SECTION _whiteappslock;
};

class CRmsAgentKeyProfile
{
public:
    CRmsAgentKeyProfile() {}
    virtual ~CRmsAgentKeyProfile() {}

    inline const std::wstring& GetXml() const throw() {return _xml;}
    inline void SetXml(const std::wstring& xml) throw() {_xml = xml;}
    
    void LoadFromRegisterData(_In_ const std::wstring& regdata);
    void Load(const std::wstring& file, const nudf::crypto::CAesKeyBlob& key);
    void Save(const std::wstring& file, const nudf::crypto::CAesKeyBlob& key);

private:
    std::wstring    _xml;
};

class CRmsUtil
{
public:
    CRmsUtil();
    virtual ~CRmsUtil();

    void Register();
    void HeartBeat();
    void CheckSoftwareUpdate();
};


       
}   // namespace engine
}   // namespace nxrm


#endif  // #ifndef __NXRM_ENGINE_RMSUTIL_H__