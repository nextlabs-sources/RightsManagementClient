
#include <Windows.h>
#include <assert.h>

#include <nudf\exception.hpp>
#include <nudf\string.hpp>
#include <nudf\registry.hpp>
#include <nudf\resutil.hpp>
#include <nudf\time.hpp>
#include <nudf\shared\rightsdef.h>

#include "nxrmeng.h"
#include "audit.hpp"


using namespace nxrm::engine;


#define LOGSVC_ROOT_NAME        L"logService"
#define LOGSVC_REQUEST_NAME     L"logRequest"
#define LOGSVC_LOGS_NAME        L"logs"
#define LOGSVC_LOG_NAME         L"log"

#define LOGSVC_ATTR_TENANTID_NAME   L"tenantId"
#define LOGSVC_ATTR_AGENTID_NAME    L"agentId"
#define LOGSVC_ATTR_VERSION_NAME    L"version"


CAuditObject::CAuditObject() : CXmlDocument(), _count(0)
{
    ::InitializeCriticalSection(&_lock);
}

CAuditObject::~CAuditObject()
{
    ::DeleteCriticalSection(&_lock);
}

void CAuditObject::Create(_In_ const std::wstring& tenantId, _In_ const std::wstring& agentId)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNode> spRoot;
    CComPtr<IXMLDOMNode> spRequest;
    
    nudf::util::CXmlDocument::Create();
    
    try {
        // Creater Root
        AppendChildElement(LOGSVC_ROOT_NAME, &spRoot);
        this->SetNodeAttribute(spRoot, LOGSVC_ATTR_TENANTID_NAME, tenantId);
        this->SetNodeAttribute(spRoot, LOGSVC_ATTR_AGENTID_NAME, agentId);
        this->SetNodeAttribute(spRoot, LOGSVC_ATTR_VERSION_NAME, L"5");
        // Create Request Element
        AppendChildElement(spRoot, LOGSVC_REQUEST_NAME, &spRequest);
        // Create Logs Element
        AppendChildElement(spRequest, LOGSVC_LOGS_NAME, &_logs);
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CAuditObject::Close() throw()
{
    _logs.Release();
    nudf::util::CXmlDocument::Close();
}

void CAuditObject::LoadFromFile(_In_ LPCWSTR file)
{
    CComPtr<IXMLDOMElement> spRoot;
    CComPtr<IXMLDOMNode>    spRequest;
    std::wstring            wsName;
    
    CXmlDocument::LoadFromFile(file);
    
    if(!GetDocRoot(&spRoot)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    wsName = GetNodeName(spRoot);
    if(0 != _wcsicmp(wsName.c_str(), LOGSVC_ROOT_NAME)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    if(!FindChildElement(spRoot, LOGSVC_REQUEST_NAME, &spRequest)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!FindChildElement(spRoot, LOGSVC_LOGS_NAME, &_logs)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    GetCountFromXml();
}

void CAuditObject::LoadFromXml(_In_ LPCWSTR xml)
{
    CComPtr<IXMLDOMElement> spRoot;
    CComPtr<IXMLDOMNode>    spRequest;
    std::wstring            wsName;
    
    CXmlDocument::LoadFromXml(xml);
    
    if(!GetDocRoot(&spRoot)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    wsName = GetNodeName(spRoot);
    if(0 != _wcsicmp(wsName.c_str(), LOGSVC_ROOT_NAME)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    if(!FindChildElement(spRoot, LOGSVC_REQUEST_NAME, &spRequest)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }
    if(!FindChildElement(spRoot, LOGSVC_LOGS_NAME, &_logs)) {
        Close();
        throw WIN32ERROR2(ERROR_FILE_INVALID);
    }

    GetCountFromXml();
}

bool CAuditObject::GetCountFromXml() throw()
{
    HRESULT hr = S_OK;
    long    lCount = 0;
    CComPtr<IXMLDOMNodeList> spChildList;

    if(NULL == _logs.p) {
        return false;
    }

    hr = _logs->get_childNodes(&spChildList);
    if(FAILED(hr)) {
        return false;
    }

    hr = spChildList->get_length(&lCount);
    if(FAILED(hr)) {
        return false;
    }

    _count = lCount;
    return true;
}

void CAuditObject::InsertAuditItem(const CAuditItem& ai)
{

    try {

        CComPtr<IXMLDOMElement> spLog;
        CComPtr<IXMLDOMNode>    spElem;
        std::wstring wsTimestamp;
        std::wstring wsRights;

        // Element <log>
        ai.GetStringTimestamp(wsTimestamp);
        this->CreateElement(LOGSVC_LOG_NAME, &spLog);
        this->SetNodeAttribute(spLog, L"uid", ai.GetUid());
        this->SetNodeAttribute(spLog, L"timestamp", wsTimestamp);
        this->SetNodeAttribute(spLog, L"type", ai.GetType());

        // Element <Rights>
        ai.GetStringRights(wsRights);
        this->AppendChildElement(spLog, L"Rights", &spElem);
        this->SetNodeText(spElem, wsRights);
        spElem.Release();

        // Element <Operation>
        this->AppendChildElement(spLog, L"Operation", &spElem);
        this->SetNodeText(spElem, ai.GetOperation());
        spElem.Release();

        // Element <Environment>
        this->AppendChildElement(spLog, L"Environment", &spElem);
        for(int i=0; i<(int)ai.GetEnvs().size(); i++) {
            CComPtr<IXMLDOMNode>    spSubElem;
            this->AppendChildElement(spElem, ai.GetEnvs()[i].first.c_str(), &spSubElem);
            this->SetNodeText(spSubElem, ai.GetEnvs()[i].second);
        }
        spElem.Release();

        // Element <User>
        CComPtr<IXMLDOMNode>    spUserName;
        CComPtr<IXMLDOMNode>    spUserId;
        CComPtr<IXMLDOMNode>    spUserAttrs;
        CComPtr<IXMLDOMNode>    spUserContext;
        this->AppendChildElement(spLog, L"User", &spElem);
        this->AppendChildElement(spElem, L"Name", &spUserName);
        this->SetNodeText(spUserName, ai.GetAuditUserName());
        spUserName.Release();
        this->AppendChildElement(spElem, L"Sid", &spUserId);
        this->SetNodeText(spUserId, ai.GetAuditUserSid());
        spUserId.Release();
        this->AppendChildElement(spElem, L"Context", &spUserContext);
        if(!ai.GetAuditUserContext().empty()) {
            this->SetNodeText(spUserContext, ai.GetAuditUserContext());
        }
        spUserContext.Release();
        this->AppendChildElement(spElem, L"Attributes", &spUserAttrs);
        //for(int i=0; i<(int)ai.GetAuditUserAttributes().size(); i++) {
        //    CComPtr<IXMLDOMNode>    spAttribute;
        //    this->AppendChildElement(spUserAttrs, L"Attribute", &spAttribute);
        //    if(NULL != spAttribute.p) {
        //        this->SetNodeAttribute(spAttribute, L"Name", ai.GetAuditUserAttributes()[i].first);
        //        this->SetNodeAttribute(spAttribute, L"Value", ai.GetAuditUserAttributes()[i].second);
        //        spAttribute.Release();
        //    }
        //}
        spUserAttrs.Release();
        spElem.Release();

        // Element <Host>
        CComPtr<IXMLDOMNode>    spHostName;
        CComPtr<IXMLDOMNode>    spHostIp;
        this->AppendChildElement(spLog, L"Host", &spElem);
        this->AppendChildElement(spElem, L"Name", &spHostName);
        this->SetNodeText(spHostName, ai.GetAuditHostName());
        spHostName.Release();
        if(!ai.GetAuditHostIpv4().empty()) {
            this->AppendChildElement(spElem, L"Ipv4", &spHostIp);
            this->SetNodeText(spHostIp, ai.GetAuditHostIpv4());
            spHostIp.Release();
        }
        spElem.Release();

        // Element <Application>
        CComPtr<IXMLDOMNode>    spAppImage;
        this->AppendChildElement(spLog, L"Application", &spElem);
        this->AppendChildElement(spElem, L"Image", &spAppImage);
        this->SetNodeText(spAppImage, ai.GetAppImage());
        spAppImage.Release();
        if(!ai.GetAppPublisher().empty()) {
            CComPtr<IXMLDOMNode>    spAppPublisher;
            this->AppendChildElement(spElem, L"Publisher", &spAppPublisher);
            this->SetNodeText(spAppPublisher, ai.GetAppPublisher());
            spAppPublisher.Release();
        }
        spElem.Release();

        // Element <Resource>
        CComPtr<IXMLDOMNode>    spResPath;
        this->AppendChildElement(spLog, L"Resource", &spElem);
        this->AppendChildElement(spElem, L"Path", &spResPath);
        this->SetNodeText(spResPath, ai.GetResPath());
        spResPath.Release();
        if(!ai.GetResTags().empty()) {
            CComPtr<IXMLDOMNode>    spResTags;
            this->AppendChildElement(spElem, L"Tags", &spResTags);
            for(int i=0; i<(int)ai.GetResTags().size(); i++) {
                CComPtr<IXMLDOMNode>    spTag;
                this->AppendChildElement(spResTags, L"Tag", &spTag);
                if(NULL != spTag.p) {
                    this->SetNodeAttribute(spTag, L"Name", ai.GetResTags()[i].first);
                    this->SetNodeAttribute(spTag, L"Value", ai.GetResTags()[i].second);
                    spTag.Release();
                }
            }
            spResTags.Release();
        }
        spElem.Release();

        // Element <HitPolicies>
        this->AppendChildElement(spLog, L"HitPolicies", &spElem);
        for(int i=0; i<(int)ai.GetEvalPolicies().size(); i++) {
            CComPtr<IXMLDOMNode>    spPolicy;
            this->AppendChildElement(spElem, L"Policy", &spPolicy);
            if(NULL != spPolicy.p) {
                this->SetNodeAttribute(spPolicy, L"Id", ai.GetEvalPolicies()[i].first);
                this->SetNodeAttribute(spPolicy, L"Name", ai.GetEvalPolicies()[i].second);
                spPolicy.Release();
            }
        }

        // Good, Add it
        CComPtr<IXMLDOMNode> spNewLog;
        HRESULT hr = S_OK;
        ::EnterCriticalSection(&_lock);
        hr = _logs->appendChild(spLog, &spNewLog);
        if(SUCCEEDED(hr)) {
            // Increase the count
            _count++;
        }
        ::LeaveCriticalSection(&_lock);
    }
    catch(const nudf::CException& e) {
        throw e;
    }
}

void CAuditObject::Dump(_In_ const std::wstring& file)
{
    if(NULL == GetDoc()) {
        return;
    }

    ::EnterCriticalSection(&_lock);
    try {
        CComPtr<IXMLDOMNode> spFirstNode;
        // Save to file
        this->SaveToFile(file.c_str());
        // Remove all the children
        while(SUCCEEDED(_logs->get_firstChild(&spFirstNode)) && NULL != spFirstNode.p) {
            CComPtr<IXMLDOMNode> spRemovedNode;
            _logs->removeChild(spFirstNode, &spRemovedNode);
            spRemovedNode.Release();
        }
        _count = 0;
    }
    catch(const nudf::CException& e) {
        throw e;
    }
    ::LeaveCriticalSection(&_lock);
}


//
//
//

class CAuditIdGen
{
public:
    CAuditIdGen()
    {
        ::InitializeCriticalSection(&_lock);
        InitId();
    }
    ~CAuditIdGen()
    {
        ::DeleteCriticalSection(&_lock);
    }

    __int64 GetNextId() throw()
    {
        __int64 id = 0;
        ::EnterCriticalSection(&_lock);
        id = _id.QuadPart;
        ++_id.QuadPart;
        if(0 != _id.HighPart) {
            _id.QuadPart = 0;
        }
        ::LeaveCriticalSection(&_lock);
        return id;
    }

private:
    void InitId()
    {
        SYSTEMTIME st20150101;
        memset(&st20150101, 0, sizeof(st20150101));
        st20150101.wYear = 2015;
        st20150101.wMonth = 1;
        st20150101.wDay = 1;
        nudf::time::CTime tm20100101(&st20150101);
        nudf::time::CTime tmnow;
        tmnow.Now();
        _id.QuadPart = 0;
        _id.LowPart = (DWORD)(tmnow.ToSecondsSince1970Jan1st() - tm20100101.ToSecondsSince1970Jan1st());
    }

private:
    CRITICAL_SECTION _lock;
    LARGE_INTEGER    _id;
};

static CAuditIdGen gAIdGen;

CAuditItem::CAuditItem() : _uid(0), _rights(0)
{
}

CAuditItem::CAuditItem(_In_ int agentId) : _uid(0), _rights(0)
{
    Initialize(agentId);
}

CAuditItem::~CAuditItem()
{
}

CAuditItem& CAuditItem::operator = (const CAuditItem& ai) throw()
{
    if(this != &ai) {        
        _uid            = ai.GetUid();
        _timestamp      = ai.GetTimestamp();
        _type           = ai.GetType();
        _rights         = ai.GetRights();
        _operation      = ai.GetOperation();
        _envs           = ai.GetEnvs();
        _userName       = ai.GetAuditUserName();
        _userSid        = ai.GetAuditUserSid();
        _userContext    = ai.GetAuditUserContext();
        _userAttrs      = ai.GetAuditUserAttributes();
        _hostName       = ai.GetAuditHostName();
        _hostIpv4       = ai.GetAuditHostIpv4();
        _appImage       = ai.GetAppImage();
        _appPublisher   = ai.GetAppPublisher();
        _resPath        = ai.GetResPath();
        _resTags        = ai.GetResTags();
        _evalPolicies   = ai.GetEvalPolicies();
    }

    return *this;
}

void CAuditItem::Initialize(_In_ int agentId)
{
    _uid = agentId;
    _uid <<= 32;
    _uid += gAIdGen.GetNextId();
    _timestamp.Now();
}

void CAuditItem::GetStringTimestamp(_Out_ std::wstring& st) const throw()
{
    SYSTEMTIME lt;
    FILETIME   lft = {0, 0};
    std::wstring sb;

    memset(&lt, 0, sizeof(lt));
    FileTimeToLocalFileTime(&_timestamp.GetTime(), &lft);
    FileTimeToSystemTime(&lft, &lt);

    nudf::time::CTimeZone tz;
    tz.BiasToString(sb);

    swprintf_s(nudf::string::tempstr<wchar_t>(st, 256), 256, L"%04d-%02d-%02dT%02d:%02d:%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
    st += sb;
}

void CAuditItem::GetStringRights(_Out_ std::wstring& sr) const throw()
{
    if(BUILTIN_RIGHT_VIEW == (_rights & BUILTIN_RIGHT_VIEW)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_VIEW;
    }
    if(BUILTIN_RIGHT_EDIT == (_rights & BUILTIN_RIGHT_EDIT)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_EDIT;
    }
    if(BUILTIN_RIGHT_PRINT == (_rights & BUILTIN_RIGHT_PRINT)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_PRINT;
    }
    if(BUILTIN_RIGHT_CLIPBOARD == (_rights & BUILTIN_RIGHT_CLIPBOARD)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_CLIPBOARD;
    }
    if(BUILTIN_RIGHT_SAVEAS == (_rights & BUILTIN_RIGHT_SAVEAS)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_SAVEAS;
    }
    if(BUILTIN_RIGHT_DECRYPT == (_rights & BUILTIN_RIGHT_DECRYPT)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_DECRYPT;
    }
    if(BUILTIN_RIGHT_SCREENCAP == (_rights & BUILTIN_RIGHT_SCREENCAP)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_SCREENCAP;
    }
    if(BUILTIN_RIGHT_SEND == (_rights & BUILTIN_RIGHT_SEND)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_SEND;
    }
    if(BUILTIN_RIGHT_CLASSIFY == (_rights & BUILTIN_RIGHT_CLASSIFY)) {
        if(!sr.empty()) sr += L",";
        sr += RIGHT_DISP_CLASSIFY;
    }

    if(sr.empty()) {
        sr = L"NONE";
    }
}