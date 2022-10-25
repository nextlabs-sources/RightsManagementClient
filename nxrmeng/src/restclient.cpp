

#include <Windows.h>

#include <exception>

#include <nudf\exception.hpp>
#include <nudf\xmlparser.hpp>
#include <nudf\crypto.hpp>

#include "nxrmeng.h"
#include "engine.hpp"
#include "restclient.hpp"
#include "..\..\common\inc\nxversionnum.h"



namespace {

class rms_server
{
public:
    rms_server()
    {
        static const std::wstring http_proto_str(L"http://");
        static const std::wstring https_proto_str(L"https://");

        try {

            nudf::util::CXmlDocument doc;
            CComPtr<IXMLDOMElement>  spRoot;
            CComPtr<IXMLDOMNode>     spServer;
            doc.LoadFromFile(_nxPaths.GetRegisterConf().c_str());
            if (!doc.GetDocRoot(&spRoot)) {
                throw WIN32ERROR();
            }
            if (!nudf::util::XmlUtil::FindChildElement(spRoot, L"SERVER", &spServer)) {
                throw WIN32ERROR();
            }
            std::wstring url = nudf::util::XmlUtil::GetNodeText(spServer);
            if (url.empty() || url.length() < https_proto_str.length()) {
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }
            std::wstring::size_type pos = url.find(L"://");
            if (pos == std::wstring::npos) {
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            }
            _server = url.substr(0, pos + 3);
            url = url.substr(pos + 3);
            pos = url.find(L"/");
            _server += url.substr(0, pos);
            _path = url.substr(pos);
            if (_path.empty() || L'/' != _path.c_str()[_path.length() - 1]) {
                _path += L"/";
            }
        }
        catch (const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e);
            _server.clear();
            _path.clear();
        }
    }

    virtual ~rms_server() throw()
    {
    }
    
    inline const std::wstring& server() const throw() { return _server; }
    inline const std::wstring& path() const throw() { return _path; }

private:
    std::wstring _server;
    std::wstring _path;
};

class rmc_agent
{
public:
    rmc_agent()
    {
        load();
    }

    ~rmc_agent()
    {
    }

    inline const std::wstring& agent_id() const { return _id; }
    inline const std::wstring& agent_cert() const { return _cert; }
    inline const std::wstring& rms_server() const { return _server; }
    inline const std::wstring& rms_path() const { return _path; }

protected:
    void load() noexcept
    {
        try {

            nudf::util::CXmlDocument doc;
            CComPtr<IXMLDOMElement> spRoot;
            CComPtr<IXMLDOMNode>    spProfile;
            CComPtr<IXMLDOMNode>    spAgentId;
            CComPtr<IXMLDOMNode>    spAgentCert;
            CComPtr<IXMLDOMNode>    spServer;
            CComPtr<IXMLDOMNode>    spPort;
            CComPtr<IXMLDOMNode>    spPath;

            std::wstring agent_id;
            std::wstring agent_cert(L"MIIDmzCCAoOgAwIBAgIEI8+FCTANBgkqhkiG9w0BAQsFADB+MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCVNhbiBNYXRlbzERMA8GA1UEChMITmV4dExhYnMxGjAYBgNVBAsTEVJpZ2h0cyBNYW5hZ2VtZW50MR8wHQYDVQQDExZSaWdodHMgTWFuYWdlbWVudCBUZW1wMB4XDTE1MDUyNzE0MDI0MVoXDTI1MDUyNDE0MDI0MVowfjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRIwEAYDVQQHEwlTYW4gTWF0ZW8xETAPBgNVBAoTCE5leHRMYWJzMRowGAYDVQQLExFSaWdodHMgTWFuYWdlbWVudDEfMB0GA1UEAxMWUmlnaHRzIE1hbmFnZW1lbnQgVGVtcDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPUFAKjEmwL/oWQpafK5vOz8/dOEf9mmcjKawhxNhJJ5R+4olKHF9ZaKMve542ehSKngA53buaPsvP96ix7j8O8E2DHnYqbR5I9jNfZarIWjAwfO94TDvQovuIFBCWMnJXsRUtVm36cF6WpQdqctIbgvSbjGbgYacqGZ6QaaWrySGxupjyB8lfD6dYBG5lXFRcFA7QQVbNAGM7Xis2S3sPZOch4VJK7faX2xRyW6sIKL0FU8W9HCbm2PjG+XBr+dmsP3lk6HOqlSEy55HRYldMI/KCSlTGIcUHjH0qpiBxceSHILgY+YOqJ3l6/d8k9ui3MK2XGUhNFgwwLYFruk1l8CAwEAAaMhMB8wHQYDVR0OBBYEFGKcZJB9ZVJ6q/T2DNJIUoQEqx6sMA0GCSqGSIb3DQEBCwUAA4IBAQA9oFoR9GYVvba1WTdq2sl7kqTxqTPkUtD5LGi5A7q1yxMkAwsR2kW00L5dbRmADT7PjE3x42V2ZHHuYhDjGg/zm+2xHVrUWl2ZxHodmHz6+qDbdAZ3+9U4Zz7nt2oxDFghp/eE1adXa2kfAIZzn8VVamD6TS9O0R/KyXToYgpjLmz6QD9GFsz5wGbVsnJGWTxfiNjX3LnFIkqJU8rHn1DcMyB3/xd3ytUJzKrAnD8f46JpfR1amJOQAxiDy5+kW1OnclGBImS9iisvCmwU3+UNixbFAAxymBA9VvAO90sw0tHcLN7M1NSpenVlAnJTHhGuLSepk8gv4jAEsa9+DPKR");
            std::wstring rms_serv(L"https://");
            std::wstring rms_port(L"8443");
            std::wstring rms_path(L"/RMS/service/");

            doc.LoadFromFile(_nxPaths.GetProfile().c_str());
            if (!doc.GetDocRoot(&spRoot)) {
                throw WIN32ERROR();
            }

            if (!nudf::util::XmlUtil::FindChildElement(spRoot, L"Profile", &spProfile)) {
                throw WIN32ERROR();
            }

            if (!nudf::util::XmlUtil::FindChildElement(spProfile, L"AgentId", &spAgentId)) {
                throw WIN32ERROR();
            }
            std::wstring s = nudf::util::XmlUtil::GetNodeText(spAgentId);
            if(s.empty()) {
                throw WIN32ERROR();
            }
            agent_id = s;
            
            if (nudf::util::XmlUtil::FindChildElement(spProfile, L"AgentCertificate", &spAgentCert)) {
                s = nudf::util::XmlUtil::GetNodeText(spAgentCert);
                if (!s.empty()) {
                    agent_cert = s;
                }
            }

            if (!nudf::util::XmlUtil::FindChildElement(spProfile, L"RmsServer", &spServer)) {
                throw WIN32ERROR();
            }
            s = nudf::util::XmlUtil::GetNodeText(spServer);
            if(s.empty()) {
                throw WIN32ERROR();
            }
            rms_serv += s;

            if (nudf::util::XmlUtil::FindChildElement(spProfile, L"RmsPort", &spPort)) {
                s = nudf::util::XmlUtil::GetNodeText(spPort);
                if (!s.empty()) {
                    rms_port = s;
                }
            }
            rms_serv += L":" + s;

            if (nudf::util::XmlUtil::FindChildElement(spProfile, L"RmsPath", &spPath)) {
                s = nudf::util::XmlUtil::GetNodeText(spPath);
                if (!s.empty()) {
                    rms_path = s;
                    if (L'/' != rms_path.c_str()[rms_path.length() - 1]) {
                        rms_path += L"/";
                    }
                }
            }

            _id = agent_id;
            _cert = agent_cert;
            _server = rms_serv;
            _path = rms_path;
        }
        catch (const nudf::CException& e) {
            UNREFERENCED_PARAMETER(e);
            _id.clear();
            _cert.clear();
            _server.clear();
            _path.clear();
        }
    }

private:
    std::wstring    _server;
    std::wstring    _path;
    std::wstring    _cert;
    std::wstring    _id;

};

}


//
// class NX::rest::details::session_key
//
NX::rest::details::session_key::session_key()
{
    nudf::crypto::CAesKeyBlob aes;
    aes.Generate(256);
    if (aes.GetKeySize() == 32) {
        _key.resize(32, 0);
        memcpy(&_key[0], aes.GetKey(), 32);
        _base64_key = NX::utility::conversions::to_base64(_key);
    }
}
NX::rest::details::session_key::~session_key()
{
}

//
// class NX::rest::rmcclient
//
NX::rest::rmclient::rmclient() : NX::rest::details::client()
{
    static rms_server serv;
    _config.set_validate_certificates(false);
    _client = std::shared_ptr<NX::web::http::client::http_client>(new NX::web::http::client::http_client(serv.server(), _config));
}

NX::rest::rmclient::rmclient(const std::wstring& server) : NX::rest::details::client()
{
    _config.set_validate_certificates(false);
    _client = std::shared_ptr<NX::web::http::client::http_client>(new NX::web::http::client::http_client(server, _config));
}

NX::rest::rmclient::~rmclient()
{
}


//
// class NX::rest::request_register
//

NX::rest::request_register::request_register() : NX::rest::details::request()
{
    static rms_server serv;

    _service_name = serv.path() + L"RegisterAgent";
    NX::web::uri name(service_name());
    _request.set_request_uri(name);

    _request.headers().add(L"X-NXL-S-CERT", L"MIIDmzCCAoOgAwIBAgIEI8+FCTANBgkqhkiG9w0BAQsFADB+MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCVNhbiBNYXRlbzERMA8GA1UEChMITmV4dExhYnMxGjAYBgNVBAsTEVJpZ2h0cyBNYW5hZ2VtZW50MR8wHQYDVQQDExZSaWdodHMgTWFuYWdlbWVudCBUZW1wMB4XDTE1MDUyNzE0MDI0MVoXDTI1MDUyNDE0MDI0MVowfjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRIwEAYDVQQHEwlTYW4gTWF0ZW8xETAPBgNVBAoTCE5leHRMYWJzMRowGAYDVQQLExFSaWdodHMgTWFuYWdlbWVudDEfMB0GA1UEAxMWUmlnaHRzIE1hbmFnZW1lbnQgVGVtcDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPUFAKjEmwL/oWQpafK5vOz8/dOEf9mmcjKawhxNhJJ5R+4olKHF9ZaKMve542ehSKngA53buaPsvP96ix7j8O8E2DHnYqbR5I9jNfZarIWjAwfO94TDvQovuIFBCWMnJXsRUtVm36cF6WpQdqctIbgvSbjGbgYacqGZ6QaaWrySGxupjyB8lfD6dYBG5lXFRcFA7QQVbNAGM7Xis2S3sPZOch4VJK7faX2xRyW6sIKL0FU8W9HCbm2PjG+XBr+dmsP3lk6HOqlSEy55HRYldMI/KCSlTGIcUHjH0qpiBxceSHILgY+YOqJ3l6/d8k9ui3MK2XGUhNFgwwLYFruk1l8CAwEAAaMhMB8wHQYDVR0OBBYEFGKcZJB9ZVJ6q/T2DNJIUoQEqx6sMA0GCSqGSIb3DQEBCwUAA4IBAQA9oFoR9GYVvba1WTdq2sl7kqTxqTPkUtD5LGi5A7q1yxMkAwsR2kW00L5dbRmADT7PjE3x42V2ZHHuYhDjGg/zm+2xHVrUWl2ZxHodmHz6+qDbdAZ3+9U4Zz7nt2oxDFghp/eE1adXa2kfAIZzn8VVamD6TS9O0R/KyXToYgpjLmz6QD9GFsz5wGbVsnJGWTxfiNjX3LnFIkqJU8rHn1DcMyB3/xd3ytUJzKrAnD8f46JpfR1amJOQAxiDy5+kW1OnclGBImS9iisvCmwU3+UNixbFAAxymBA9VvAO90sw0tHcLN7M1NSpenVlAnJTHhGuLSepk8gv4jAEsa9+DPKR");
}

NX::rest::request_register::~request_register()
{
}

//
// class NX::rest::request_authn
//
NX::rest::request_authn::request_authn() : NX::rest::details::request2()
{
    set_service_name(L"Login");
    NX::web::uri name(service_name());
    _request.set_request_uri(name);
}

NX::rest::request_authn::request_authn(const std::wstring& user_name, const std::wstring& domain_name, const std::wstring& password) : NX::rest::details::request2()
{
    set_service_name(L"Login");
    NX::web::uri name(service_name());
    _request.set_request_uri(name);

    // build data
    std::wstring xml = L"<LoginService tenantId=\"N/A\" version=\"1\" agentId=\"";
    xml += agent_id();
    xml += L"\"><LoginRequest><UserName>";
    xml += user_name;
    xml += L"</UserName><Domain>";
    xml += domain_name;
    xml += L"</Domain><Password>";
    xml += password;
    xml += L"</Password><IDPType>AD</IDPType></LoginRequest></LoginService>";

    set_data(xml);
}

NX::rest::request_authn::~request_authn()
{
}

//
// class NX::rest::response_authn
//
NX::rest::response_authn::response_authn() : NX::rest::details::response(), _result(-1)
{
}

NX::rest::response_authn::~response_authn()
{
}

void NX::rest::response_authn::set_response(const std::wstring& body)
{
    nudf::util::CXmlDocument doc;
    response::set_response(body);


    _result = -1;

    if (empty()) {
        throw std::exception("body is empty");
    }

    try {

        CComPtr<IXMLDOMElement>  spRoot;
        CComPtr<IXMLDOMNode>     spResponse;
        CComPtr<IXMLDOMNode>     spResult;
        CComPtr<IXMLDOMNode>     spAttributes;

        std::wstring res_result;

        try {
            doc.LoadFromXml(data().c_str());
        }
        catch (const nudf::CException& e) {
            throw e;
        }
        if (!doc.GetDocRoot(&spRoot)) {
            throw WIN32ERROR();
        }

        if (!nudf::util::XmlUtil::FindChildElement(spRoot, L"LoginResponse", &spResponse)) {
            throw WIN32ERROR();
        }

        if (!nudf::util::XmlUtil::FindChildElement(spResponse, L"LoginResult", &spResult)) {
            throw WIN32ERROR();
        }
        res_result = nudf::util::XmlUtil::GetNodeText(spResult);
        if (res_result.empty()) {
            throw WIN32ERROR();
        }

        if (0 == _wcsicmp(res_result.c_str(), L"true") || 0 == _wcsicmp(res_result.c_str(), L"0")) {
            // Succeed
            CComPtr<IXMLDOMNode>     spId;
            CComPtr<IXMLDOMNode>     spAttributes;

            // get user id
            if (!nudf::util::XmlUtil::FindChildElement(spResponse, L"UId", &spId)) {
                throw WIN32ERROR();
            }
            _user_id = nudf::util::XmlUtil::GetNodeText(spId);
            if (_user_id.empty()) {
                throw WIN32ERROR();
            }

            // get user attributes
            if (nudf::util::XmlUtil::FindChildElement(spResponse, L"UserAttributes", &spAttributes)) {
                CComPtr<IXMLDOMNodeList>     spChildList;
                HRESULT hr = spAttributes->get_childNodes(&spChildList);
                if (SUCCEEDED(hr) && NULL != spChildList.p) {
                    long count = 0;
                    hr = spChildList->get_length(&count);
                    if (SUCCEEDED(hr) && count > 0) {
                        for (long i = 0; i < count; i++) {
                            CComPtr<IXMLDOMNode>    spAttr;
                            hr = spChildList->get_item(i, &spAttr);
                            if (SUCCEEDED(hr) && NULL != spAttr.p) {
                                CComPtr<IXMLDOMNode>    spName;
                                CComPtr<IXMLDOMNode>    spValue;
                                std::wstring elemName;
                                std::wstring attrName;
                                std::wstring attrValue;

                                // Only handle <Attribute> element
                                elemName = nudf::util::XmlUtil::GetNodeName(spAttr);
                                if (0 != _wcsicmp(elemName.c_str(), L"Attribute")) {
                                    continue;
                                }

                                if (nudf::util::XmlUtil::FindChildElement(spAttr, L"Name", &spName)) {
                                    attrName = nudf::util::XmlUtil::GetNodeText(spName);
                                }
                                if (nudf::util::XmlUtil::FindChildElement(spAttr, L"Value", &spValue)) {
                                    attrValue = nudf::util::XmlUtil::GetNodeText(spValue);
                                }
                                if (!attrName.empty() && !attrValue.empty()) {
                                    _user_attrs[attrName] = attrValue;
                                }
                            }
                        }
                    }
                }
            }

            // succeed
            _result = 0;
        }
        else {
            if (0 == _wcsicmp(res_result.c_str(), L"false")) {
                CComPtr<IXMLDOMNode> spError;
                if (nudf::util::XmlUtil::FindChildElement(spResponse, L"error", &spError)) {
                    CComPtr<IXMLDOMNode> spErrorCode;
                    if (nudf::util::XmlUtil::FindChildElement(spError, L"errorCode", &spErrorCode)) {
                        res_result = nudf::util::XmlUtil::GetNodeText(spErrorCode);
                    }
                }
            }

            int e = _wtoi(res_result.c_str());
            if (0 != e) {
                _result = e;
            }
        }
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        throw std::exception("fail to parse response");
    }
}

void NX::rest::response_authn::clear() noexcept
{
    _result = -1;
    _user_id.clear();
    _user_attrs.clear();
    response::clear();
}


//
// class NX::rest::details::request
//
#define AGENT_NAME(major, minor, build)  L"NXRMC/" #major L"." #minor L"." #build

NX::rest::details::request::request() : _request(NX::web::http::http_request(NX::web::http::methods::POST))
{
    static std::wstring agent_name(AGENT_NAME(VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER));
    static rms_server serv;
    _service_name = serv.path();
    _rms_server = serv.server();

    _request.headers().add(NX::web::http::header_names::user_agent, AGENT_NAME(VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER));
    _request.headers().add(L"X-NXL-S-KEY", _session_key.base64_key());
}

NX::rest::details::request::~request()
{
}

void NX::rest::details::request::set_data(const std::wstring& s) noexcept
{
    _request.set_body(s);
    _data = NX::utility::conversions::utf16_to_utf8(s);
}


//
// class NX::rest::details::request_register
//

NX::rest::details::request2::request2() : NX::rest::details::request()
{
    static rmc_agent agent;
    _service_name = agent.rms_path();
    _rms_server = agent.rms_server();
    _agent_id = agent.agent_id();
    _request.headers().add(L"X-NXL-S-CERT", agent.agent_cert());
}

NX::rest::details::request2::~request2()
{
}


//
// class NX::rest::details::response
//
NX::rest::details::response::response()
{
}

NX::rest::details::response::~response()
{
}


//
// class NX::rest::details::client
//
NX::rest::details::client::client()
{
}

NX::rest::details::client::~client()
{
}

void NX::rest::details::client::request(const NX::rest::details::request* req, NX::rest::details::response* res) noexcept
{
    try {
        _client->request(req->get_request()).then([&](NX::web::http::http_response response)
        {
            auto body = response.extract_string().get();
            res->set_response(body);
        }).wait();
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        res->clear();
    }
}