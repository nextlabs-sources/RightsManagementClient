

#include <Windows.h>
#include <iphlpapi.h>
#include <Mstcpip.h>

#include <exception>
#include <memory>

#include <nudf\exception.hpp>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>
#include <nudf\registry.hpp>
#include <nudf\web\json.hpp>
#include <nudf\convert.hpp>

#include <rapidxml\rapidxml.hpp>
#include <rapidxml\rapidxml_utils.hpp>
#include <rapidxml\rapidxml_print.hpp>

#include "nxrmserv.h"
#include "sys.hpp"
#include "session.hpp"
#include "rest.hpp"
#include "..\..\common\inc\nxversionnum.h"


static std::wstring build_register_params();
static bool process_register_response(const std::wstring& xml, NX::agent_info& _agent_info);

static std::wstring build_acknowledge_param();
static bool process_acknowledge_response(const std::wstring& xml, int& code);

static std::wstring build_heartbeat_param(bool force);

static bool process_audit_response(const std::wstring& xml);

static std::wstring build_authn_param(const std::wstring& user_name, const std::wstring& domain_name, const std::wstring& password);

static std::wstring build_eval_param(const NX::EVAL::eval_object& eo);
static bool process_eval_response(const std::wstring& body, NX::EVAL::eval_result* evalresult);

static std::wstring build_checkupdate_param();


namespace NX {
namespace rest {

class session_key
{
public:
    session_key();
    ~session_key();

    inline size_t size() const { return _key.size(); }
    inline const unsigned char* key() const { return _key.data(); }
    inline const std::wstring& base64_key() const { return _base64_key; }

private:
    std::vector<unsigned char>  _key;
    std::wstring                _base64_key;
};


NX::rest::session_key::session_key()
{
    nudf::crypto::CAesKeyBlob aes;
    aes.Generate(256);
    if (aes.GetKeySize() == 32) {
        _key.resize(32, 0);
        memcpy(&_key[0], aes.GetKey(), 32);
        _base64_key = NX::utility::conversions::to_base64(_key);
    }
}
NX::rest::session_key::~session_key()
{
}

}
}

NX::rest_client::rest_client() : _inactive(false)
{
    InitializeCriticalSection(&_lock);
}

NX::rest_client::~rest_client()
{
    ::DeleteCriticalSection(&_lock);
}

bool NX::rest_client::init()
{
    bool result = false;

    ::EnterCriticalSection(&_lock);
    try {
        if (!_inactive) {
            if (NULL != _client) {
                result = true;
            }
            else {
                _client = create_client(GLOBAL.register_info().server());
                if (NULL != _client) {
                    _service_base = GLOBAL.register_info().service_base();
                    _cert = GLOBAL.register_info().cert();
                    // (AGENT_NAME(VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER));
                    swprintf_s(nudf::string::tempstr<wchar_t>(_agent_name, 64), 63, L"NXRMC/%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER);
                    result = true;
                    LOGDMP(L"REST client init successfully");
                }
                else {
                    LOGDMP(L"Fail to create REST client: %d", GetLastError());
                }
            }
        }
    }
    catch (std::exception& e) {
        LOGDMP(L"Fail to init REST client: %S", (e.what() ? e.what() : "Unknown"));
        UNREFERENCED_PARAMETER(e);
    }
    catch (...) {
        LOGDMP(L"Fail to init REST client: Unknown Error");
    }
    ::LeaveCriticalSection(&_lock);

    return result;
}

void NX::rest_client::clear()
{
    ::EnterCriticalSection(&_lock);
    _inactive = true;
    if (NULL != _client) {
        delete _client;
        _client = NULL;
    }
    ::LeaveCriticalSection(&_lock);
}

std::wstring NX::rest_client::get_session_key()
{
    static NX::rest::session_key gSessionKey;
    return gSessionKey.base64_key();
}

void NX::rest_client::send_request(const NX::http::basic_request* req, NX::http::basic_response* res)
{
    try {
        LOGDMP(L"Prepare to send request to: %s", req->path().c_str());
        _client->send_request(req, res);
        LOGDMP(L"Request sent");
        if (!GLOBAL.connected()) {
            GLOBAL.set_connect_status(true);
            LOGINF(L"Connect to RMS");
            NX::serv_session::SessionMapType ss;
            GLOBAL.serv_session().copy(ss);
            std::for_each(ss.begin(), ss.end(), [&](const std::pair<unsigned long, std::shared_ptr<NX::session>>& it) {
                it.second->set_serv_status();
            });
        }
    }
    catch (std::exception& e) {
        LOGDMP(L"Fail to send request to: %s", req->path().c_str());
        LOGDMP(L"    ==> Exception: %S", (e.what() ? e.what() : "Unknown"));
        if (GLOBAL.connected()) {
            GLOBAL.set_connect_status(false);
            LOGINF(L"Lose connection to RMS");
            NX::serv_session::SessionMapType ss;
            GLOBAL.serv_session().copy(ss);
            std::for_each(ss.begin(), ss.end(), [&](const std::pair<unsigned long, std::shared_ptr<NX::session>>& it) {
                it.second->set_serv_status();
            });
        }
        throw e;
    }
}


NX::http::client* NX::rest_client::create_client(const std::wstring& server_url)
{
    NX::http::client* p = NULL;

    if (server_url.empty())
        return NULL;

    try {
        p = new NX::http::client(server_url, true, 2000);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        if (NULL != p) {
            delete p;
            p = NULL;
        }
    }

    return p;
}

bool NX::rest_client::register_agent(NX::agent_info& _agent_info)
{
    bool result = false;

    static const std::wstring rest_api_name(L"/RMS/service/RegisterAgent");

    if (!init())
        return false;

    try {

        std::vector<LPCWSTR> call_accept_types;
        std::vector<std::pair<std::wstring, std::wstring>> call_headers;
        std::shared_ptr<NX::http::string_request> call_request;
        std::shared_ptr<NX::http::string_response> call_response;
        const std::wstring call_body(build_register_params());

        call_accept_types.push_back(NX::http::mime_types::application_xml.c_str());
        call_accept_types.push_back(NX::http::mime_types::text.c_str());
        call_headers.push_back(std::pair<std::wstring, std::wstring>(NX::http::header_names::user_agent, _agent_name));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-KEY", get_session_key()));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-CERT", GLOBAL.agent_info().cert().empty() ? GLOBAL.register_info().cert() : GLOBAL.agent_info().cert()));
        call_request = std::shared_ptr<NX::http::string_request>(new NX::http::string_request(NX::http::methods::POST, rest_api_name, call_headers, call_accept_types, call_body));
        call_response = std::shared_ptr<NX::http::string_response>(new NX::http::string_response());
        // Call RESTful API
        send_request(call_request.get(), call_response.get());

        // Get result
        result = process_register_response(nudf::util::convert::Utf8ToUtf16(call_response->body()), _agent_info);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool NX::rest_client::request_heartbeat(rest_heartbeat_result& hbresult)
{
    bool result = false;

    static const std::wstring rest_api_name(L"/RMS/service/HeartBeat");

    if (!init())
        return false;

    try {

        std::vector<LPCWSTR> call_accept_types;
        std::vector<std::pair<std::wstring, std::wstring>> call_headers;
        std::shared_ptr<NX::http::string_request> call_request;
        std::shared_ptr<NX::http::string_response> call_response;
        const bool force_update = (GLOBAL.policy_bundle().empty() || GLOBAL.policy_need_force_update()) ? true : false;
        const std::wstring call_body(build_heartbeat_param(force_update));

        call_accept_types.push_back(NX::http::mime_types::application_xml.c_str());
        call_accept_types.push_back(NX::http::mime_types::text.c_str());
        call_headers.push_back(std::pair<std::wstring, std::wstring>(NX::http::header_names::user_agent, _agent_name));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-KEY", get_session_key()));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-CERT", GLOBAL.agent_info().cert().empty() ? GLOBAL.register_info().cert() : GLOBAL.agent_info().cert()));
        call_request = std::shared_ptr<NX::http::string_request>(new NX::http::string_request(NX::http::methods::POST, rest_api_name, call_headers, call_accept_types, call_body));
        call_response = std::shared_ptr<NX::http::string_response>(new NX::http::string_response());
        // Call RESTful API
        send_request(call_request.get(), call_response.get());

        // Get result
        result = hbresult.set_result(nudf::util::convert::Utf8ToUtf16(call_response->body()), force_update);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGDMP(L"Heart beat failed: %S", e.what() ? e.what() : "Unknown error");
        result = false;
    }
    catch (...) {
        LOGDMP(L"Heart beat failed: Unknown error");
        result = false;
    }

    return result;
}

bool NX::rest_client::send_acknowledge()
{
    bool result = false;

    static const std::wstring rest_api_name(L"/RMS/service/AckHeartBeat");

    if (!init())
        return false;

    try {

        std::vector<LPCWSTR> call_accept_types;
        std::vector<std::pair<std::wstring, std::wstring>> call_headers;
        std::shared_ptr<NX::http::string_request> call_request;
        std::shared_ptr<NX::http::string_response> call_response;
        const std::wstring call_body(build_acknowledge_param());

        call_accept_types.push_back(NX::http::mime_types::application_xml.c_str());
        call_accept_types.push_back(NX::http::mime_types::text.c_str());
        call_headers.push_back(std::pair<std::wstring, std::wstring>(NX::http::header_names::user_agent, _agent_name));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-KEY", get_session_key()));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-CERT", GLOBAL.agent_info().cert().empty() ? GLOBAL.register_info().cert() : GLOBAL.agent_info().cert()));
        call_request = std::shared_ptr<NX::http::string_request>(new NX::http::string_request(NX::http::methods::POST, rest_api_name, call_headers, call_accept_types, call_body));
        call_response = std::shared_ptr<NX::http::string_response>(new NX::http::string_response());
        // Call RESTful API
        send_request(call_request.get(), call_response.get());

        // Get result
        int code = 0;
        result = process_acknowledge_response(nudf::util::convert::Utf8ToUtf16(call_response->body()), code);
        if (result) {
            result = (0 == code || 200 == code);
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool NX::rest_client::send_audit_log(const std::string& s)
{
    bool result = false;

    static const std::wstring rest_api_name(L"/RMS/service/SendLog");

	if (!init()) {
		LOGERR(ERROR_INVALID_HANDLE, L"REST API is not initialized");
		return false;
	}

    try {

        std::vector<LPCWSTR> call_accept_types;
        std::vector<std::pair<std::wstring, std::wstring>> call_headers;
        std::shared_ptr<NX::http::string_request> call_request;
        std::shared_ptr<NX::http::string_response> call_response;

        call_accept_types.push_back(NX::http::mime_types::application_xml.c_str());
        call_accept_types.push_back(NX::http::mime_types::text.c_str());
        call_headers.push_back(std::pair<std::wstring, std::wstring>(NX::http::header_names::user_agent, _agent_name));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-KEY", get_session_key()));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-CERT", GLOBAL.agent_info().cert().empty() ? GLOBAL.register_info().cert() : GLOBAL.agent_info().cert()));
        call_request = std::shared_ptr<NX::http::string_request>(new NX::http::string_request(NX::http::methods::POST, rest_api_name, call_headers, call_accept_types, s));
        call_response = std::shared_ptr<NX::http::string_response>(new NX::http::string_response());
        // Call RESTful API
        send_request(call_request.get(), call_response.get());

        // Get result
        int code = 0;
        result = process_audit_response(nudf::util::convert::Utf8ToUtf16(call_response->body()));
        if (result) {
            result = (0 == code || 200 == code);
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
		LOGERR(GetLastError(), L"Fail to send audit log request");
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool NX::rest_client::request_authn(const std::wstring& user_name, const std::wstring& domain_name, const std::wstring& password, NX::rest_authn_result& authnresult)
{
    bool result = false;

    static const std::wstring rest_api_name(L"/RMS/service/Login");

    if (!init())
        return false;

    try {

        std::vector<LPCWSTR> call_accept_types;
        std::vector<std::pair<std::wstring, std::wstring>> call_headers;
        std::shared_ptr<NX::http::string_request> call_request;
        std::shared_ptr<NX::http::string_response> call_response;
        const std::wstring call_body(build_authn_param(user_name, domain_name, password));

        call_accept_types.push_back(NX::http::mime_types::application_xml.c_str());
        call_accept_types.push_back(NX::http::mime_types::text.c_str());
        call_headers.push_back(std::pair<std::wstring, std::wstring>(NX::http::header_names::user_agent, _agent_name));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-KEY", get_session_key()));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-CERT", GLOBAL.agent_info().cert().empty() ? GLOBAL.register_info().cert() : GLOBAL.agent_info().cert()));
        call_request = std::shared_ptr<NX::http::string_request>(new NX::http::string_request(NX::http::methods::POST, rest_api_name, call_headers, call_accept_types, call_body));
        call_response = std::shared_ptr<NX::http::string_response>(new NX::http::string_response());
        // Call RESTful API
        send_request(call_request.get(), call_response.get());

        // Get result
        int code = 0;
        result = authnresult.set_result(nudf::util::convert::Utf8ToUtf16(call_response->body()));
        if (result) {
            result = (0 == code || 200 == code);
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool NX::rest_client::request_eval(const NX::EVAL::eval_object& eo, NX::EVAL::eval_result* evalresult)
{
    bool result = false;

    static const std::wstring rest_api_name(L"/RMS/service/EvaluatePolicies");

    if (!init())
        return false;

    try {

        std::vector<LPCWSTR> call_accept_types;
        std::vector<std::pair<std::wstring, std::wstring>> call_headers;
        std::shared_ptr<NX::http::string_request> call_request;
        std::shared_ptr<NX::http::string_response> call_response;
        const std::wstring call_body(build_eval_param(eo));

        call_accept_types.push_back(NX::http::mime_types::application_xml.c_str());
        call_accept_types.push_back(NX::http::mime_types::text.c_str());
        call_headers.push_back(std::pair<std::wstring, std::wstring>(NX::http::header_names::user_agent, _agent_name));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-KEY", get_session_key()));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-CERT", GLOBAL.agent_info().cert().empty() ? GLOBAL.register_info().cert() : GLOBAL.agent_info().cert()));
        call_request = std::shared_ptr<NX::http::string_request>(new NX::http::string_request(NX::http::methods::POST, rest_api_name, call_headers, call_accept_types, call_body));
        call_response = std::shared_ptr<NX::http::string_response>(new NX::http::string_response());
        // Call RESTful API
        send_request(call_request.get(), call_response.get());

        // Get result
        result = process_eval_response(nudf::util::convert::Utf8ToUtf16(call_response->body()), evalresult);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

bool NX::rest_client::check_update(rest_checkupdate_result& updateresult)
{
    bool result = false;

    static const std::wstring rest_api_name(L"/RMS/service/CheckUpdates");

    if (!init())
        return false;

    try {

        std::vector<LPCWSTR> call_accept_types;
        std::vector<std::pair<std::wstring, std::wstring>> call_headers;
        std::shared_ptr<NX::http::string_request> call_request;
        std::shared_ptr<NX::http::string_response> call_response;
        const std::wstring call_body(build_checkupdate_param());

        call_accept_types.push_back(NX::http::mime_types::application_xml.c_str());
        call_accept_types.push_back(NX::http::mime_types::text.c_str());
        call_headers.push_back(std::pair<std::wstring, std::wstring>(NX::http::header_names::user_agent, _agent_name));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-KEY", get_session_key()));
        call_headers.push_back(std::pair<std::wstring, std::wstring>(L"X-NXL-S-CERT", GLOBAL.agent_info().cert().empty() ? GLOBAL.register_info().cert() : GLOBAL.agent_info().cert()));
        call_request = std::shared_ptr<NX::http::string_request>(new NX::http::string_request(NX::http::methods::POST, rest_api_name, call_headers, call_accept_types, call_body));
        call_response = std::shared_ptr<NX::http::string_response>(new NX::http::string_response());
        // Call RESTful API
        send_request(call_request.get(), call_response.get());

        // Get result
        result = updateresult.set_result(nudf::util::convert::Utf8ToUtf16(call_response->body()));
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

//
// class NX::rest::request_register / NX::rest::response_register
//
//#define MakeXmlElement(n, v)    L"<" #n L">" #v L"</" #n L">"

__forceinline std::wstring MakeXmlElement(const std::wstring& name, int value)
{
    std::wstring s;
    unsigned long size = (unsigned long)name.length() * 2 + 128;
    swprintf_s(nudf::string::tempstr<wchar_t>(s, size), size, L"<%s>%d</%s>", name.c_str(), value, name.c_str());
    return s;
}

__forceinline std::wstring MakeXmlElement(const std::wstring& name, const std::wstring& value)
{
    std::wstring s;
    s += L"<";
    s += name;
    s += L">";
    s += value;
    s += L"</";
    s += name;
    s += L">";
    return s;
}

std::wstring build_register_params()
{
    WCHAR wzTemp[MAX_PATH] = { 0 };
    std::vector<NX::sys::hardware::net_adapter> adapters;
    std::wstring mac_address_list;
    std::wstring _data;

    NX::sys::hardware::get_net_adapters(adapters);
    std::for_each(adapters.begin(), adapters.end(), [&](const NX::sys::hardware::net_adapter& adapter) {
        if ((IF_TYPE_ETHERNET_CSMACD == adapter.if_type() || IF_TYPE_IEEE80211 == adapter.if_type())
            && !adapter.physical_address().empty()
            && 0 != _wcsnicmp(adapter.description().c_str(), L"VMware", 6)) {
            if (!mac_address_list.empty()) mac_address_list += L",";
            mac_address_list += adapter.physical_address();
        }
    });

    _data = L"<RegisterAgentRequest><RegistrationData>";
    _data += L"<host>";
    _data += GLOBAL.host().fully_qualified_domain_name().empty() ? GLOBAL.host().name() : GLOBAL.host().fully_qualified_domain_name();
    _data += L"</host>";
    _data += L"<type>DESKTOP</type>";
    _data += L"<version>";
    _data += MakeXmlElement(L"major", VERSION_MAJOR);
    _data += MakeXmlElement(L"minor", VERSION_MINOR);
    _data += MakeXmlElement(L"maintenance", 0);
    _data += MakeXmlElement(L"patch", 0);
    _data += MakeXmlElement(L"build", BUILD_NUMBER);
    _data += L"</version>";
    _data += L"<osInformation>";
    _data += MakeXmlElement(L"osType", L"Windows");
    _data += L"<osVersion>";
    _data += NX::sys::os::os_name();
    _data += L"</osVersion>";
    _data += L"</osInformation>";
    _data += L"<hardwareDetails>";
    _data += L"<cpu>";
    _data += NX::sys::hardware::cpu_brand();
    _data += L"</cpu>";
    _data += L"<memoryAmount>";
    swprintf_s(wzTemp, MAX_PATH, L"%d", NX::sys::hardware::memory_in_mb());
    _data += wzTemp;
    _data += L"</memoryAmount>";
    _data += L"<memoryUnits>MB</memoryUnits>";
    _data += L"<macAddressList>";
    _data += mac_address_list;
    _data += L"</macAddressList>";
    _data += L"<deviceId>0</deviceId>";
    _data += L"</hardwareDetails>";
    _data += L"<groupInformation>";
    _data += L"<groupId>";
    _data += GLOBAL.register_info().group_id().empty() ? L"Unknown" : GLOBAL.register_info().group_id();
    _data += L"</groupId>";
    _data += L"<groupName>";
    _data += GLOBAL.register_info().group_name().empty() ? L"Unknown" : GLOBAL.register_info().group_name();
    _data += L"</groupName>";
    _data += L"</groupInformation>";
    _data += L"<tenantId>";
    _data += GLOBAL.register_info().tenant_id().empty() ? L"Unknown" : GLOBAL.register_info().tenant_id();
    _data += L"</tenantId>";
    _data += L"</RegistrationData></RegisterAgentRequest>";

    return std::move(_data);
}

static int load_frequency(rapidxml::xml_node<wchar_t>* node, const std::wstring& name)
{
    int                     frequency = 0;

    try {

        std::wstring wsTimeUnit;

        rapidxml::xml_node<wchar_t>* freq_node = node->first_node(name.c_str(), 0, false);
        if (NULL == freq_node)
            throw std::exception("fail to find frequency node");

        rapidxml::xml_node<wchar_t>* time_node = freq_node->first_node(L"time", 0, false);
        if (NULL == freq_node)
            throw std::exception("fail to find time node");

        rapidxml::xml_node<wchar_t>* timeunit_node = freq_node->first_node(L"time-Unit", 0, false);
        if (NULL != timeunit_node) {
            wsTimeUnit = timeunit_node->value();
        }

        frequency = _wtoi(time_node->value());
        if (0 == _wcsicmp(wsTimeUnit.c_str(), L"days")) {
            frequency *= 86400;
        }
        else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"hours")) {
            frequency *= 3600;
        }
        else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"minutes")) {
            frequency *= 60;
        }
        else {
            ; // by default it is seconds
        }
    }
    catch (...) {
        frequency = 120;
    }

    return frequency;
}

static void load_agent_info(rapidxml::xml_node<wchar_t>* node, NX::agent_info& info)
{
    try {

        std::wstring    ws;

        // Agent Id
        rapidxml::xml_node<wchar_t>* agentid_node = node->first_node(L"id", 0, false);
        if (NULL == agentid_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = agentid_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_id(ws);

        // RMS Agent Profile
        rapidxml::xml_node<wchar_t>* agentprofile_node = node->first_node(L"AgentProfile", 0, false);
        if (NULL == agentprofile_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        rapidxml::xml_node<wchar_t>* agentprofile_name_node = agentprofile_node->first_node(L"name", 0, false);
        if (NULL == agentprofile_name_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = agentprofile_name_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_agent_profile_name(ws);

        //  -> Timestamp
        rapidxml::xml_node<wchar_t>* agentprofile_time_node = agentprofile_node->first_node(L"modifiedDate", 0, false);
        if (NULL == agentprofile_time_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = agentprofile_time_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_agent_profile_time(ws);

        // RMS Comm Profile
        rapidxml::xml_node<wchar_t>* commprofile_node = node->first_node(L"commProfile", 0, false);
        if (NULL == commprofile_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        rapidxml::xml_node<wchar_t>* commprofile_name_node = commprofile_node->first_node(L"name", 0, false);
        if (NULL == commprofile_name_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = commprofile_name_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_comm_profile_name(ws);

        //  -> Timestamp
        rapidxml::xml_node<wchar_t>* commprofile_time_node = commprofile_node->first_node(L"modifiedDate", 0, false);
        if (NULL == commprofile_time_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = commprofile_time_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_comm_profile_time(ws);

        //  -> HeartBeat Frequency
        info.set_heartbeat_frequency(load_frequency(commprofile_node, L"heartBeatFrequency"));
        //  -> Log Frequency
        info.set_log_frequency(load_frequency(commprofile_node, L"logFrequency"));
    }
    catch (const nudf::CException& e) {
        info.clear();
        throw e;
    }
}

bool process_register_response(const std::wstring& xml, NX::agent_info& _agent_info)
{
    bool result = false;

    try {

        std::wstring    ws;
        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(xml.begin(), xml.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* response = doc.first_node(L"RegisterAgentResponse", 0, false);
        if (NULL == response) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Get StartupConf <StartupConfiguration>
        rapidxml::xml_node<wchar_t>* startupconf_node = response->first_node(L"StartupConfiguration", 0, false);
        if (NULL == startupconf_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        load_agent_info(startupconf_node, _agent_info);

        // Get Cert <certificate>
        rapidxml::xml_node<wchar_t>* cert_node = response->first_node(L"certificate", 0, false);
        if (NULL == cert_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = cert_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _agent_info.set_cert(ws);
        result = true;
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        _agent_info.clear();
    }
    catch (...) {
        _agent_info.clear();
    }

    return result;
}


//
// class NX::rest::request_acknowledge
//

std::wstring build_acknowledge_param()
{
    std::wstring _data;
    NX::logged_on_users lus;

    _data = L"<AcknowledgeHeartBeatRequest>";
    _data += MakeXmlElement(L"id", GLOBAL.agent_info().id());
    _data += L"<acknowledgementData>";
    _data += L"<profileStatus>";
    _data += MakeXmlElement(L"lastCommittedAgentProfileName", GLOBAL.agent_info().agent_profile_name());
    _data += MakeXmlElement(L"lastCommittedAgentProfileTimestamp", GLOBAL.agent_info().agent_profile_time());
    _data += MakeXmlElement(L"lastCommittedCommProfileName", GLOBAL.agent_info().comm_profile_name());
    _data += MakeXmlElement(L"lastCommittedCommProfileTimestamp", GLOBAL.agent_info().comm_profile_time());
    _data += L"</profileStatus>";
    _data += L"<policyAssemblyStatus agentHost=\"";
    _data += GLOBAL.host().in_domain() ? GLOBAL.host().fully_qualified_domain_name() : GLOBAL.host().name();
    _data += L"\" agentType=\"DESKTOP\" timestamp=\"";
    _data += GLOBAL.policy_bundle().time();
    _data += L"\">";
    std::for_each(lus.users().begin(), lus.users().end(), [&](const std::pair<std::wstring, std::wstring>& u) {
        _data += L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>";
        _data += u.first;
        _data += L"</systemId></policyUsers>";
    });
    std::vector<std::wstring> external_users = GLOBAL.external_users().get_id_list();
    std::for_each(external_users.begin(), external_users.end(), [&](const std::wstring& user_id) {
        _data += L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>";
        _data += user_id;
        _data += L"</systemId></policyUsers>";
    });
    _data += L"</policyAssemblyStatus>";
    _data += L"</acknowledgementData>";
    _data += L"</AcknowledgeHeartBeatRequest>";

    return std::move(_data);
}

bool process_acknowledge_response(const std::wstring& xml, int& code)
{
    bool result = false;

    try {

        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(xml.begin(), xml.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* response = doc.first_node(L"AcknowledgeHeartBeatResponse", 0, false);
        if (NULL == response) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Get StartupConf <fault>
        code = 0;
        rapidxml::xml_node<wchar_t>* fault_node = response->first_node(L"fault", 0, false);
        if (NULL != fault_node) {
            rapidxml::xml_node<wchar_t>* errorcode_node = fault_node->first_node(L"ErrorCode", 0, false);
            if(NULL != errorcode_node)
                code = _wtoi(errorcode_node->value());
        }
        result = true;
    }
    catch (nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

//
// class NX::rest::request_heartbeat / NX::rest::response_heartbeat
//

static std::wstring build_heartbeat_param(bool force)
{
    std::wstring _data;
    NX::logged_on_users lus;

    static const std::wstring zero_timestamp(L"2000-01-01T00:00:00.000+00:00");

    std::wstring policy_bundle_time;
    if (GLOBAL.policy_bundle().empty() || GLOBAL.policy_need_force_update()) {
        policy_bundle_time = zero_timestamp;
    }
    else {
        policy_bundle_time = GLOBAL.policy_bundle().time();
    }

    _data = L"<HeartBeatRequest tenantId=\"";
    _data += GLOBAL.register_info().tenant_id();
    _data += L"\">";
    _data += MakeXmlElement(L"agentId", GLOBAL.agent_info().id());
    _data += L"<heartbeat>";
    _data += L"<profileStatus>";
    _data += MakeXmlElement(L"lastCommittedAgentProfileName", GLOBAL.agent_info().agent_profile_name());
    _data += MakeXmlElement(L"lastCommittedAgentProfileTimestamp", GLOBAL.agent_info().agent_profile_time());
    _data += MakeXmlElement(L"lastCommittedCommProfileName", GLOBAL.agent_info().comm_profile_name());
    _data += MakeXmlElement(L"lastCommittedCommProfileTimestamp", GLOBAL.agent_info().comm_profile_time());
    _data += L"</profileStatus>";
    _data += L"<policyAssemblyStatus agentHost=\"";
    _data += GLOBAL.host().in_domain() ? GLOBAL.host().fully_qualified_domain_name() : GLOBAL.host().name();
    _data += L"\" agentType=\"DESKTOP\" timestamp=\"";
    _data += force ? zero_timestamp : GLOBAL.policy_bundle().time();
    _data += L"\">";
    std::for_each(lus.users().begin(), lus.users().end(), [&](const std::pair<std::wstring, std::wstring>& u) {
        _data += L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>";
        _data += u.first;
        _data += L"</systemId></policyUsers>";
    });
    std::vector<std::wstring> external_users = GLOBAL.external_users().get_id_list();
    std::for_each(external_users.begin(), external_users.end(), [&](const std::wstring& user_id) {
        auto pos = std::find_if(lus.users().begin(), lus.users().end(), [&](const std::pair<std::wstring, std::wstring>& u) -> bool {
            return (0 == _wcsicmp(user_id.c_str(), u.first.c_str()));
        });
        if (pos == lus.users().end()) {
            // not exist, add it
            _data += L"<policyUsers><userSubjectType>windowsSid</userSubjectType><systemId>";
            _data += user_id;
            _data += L"</systemId></policyUsers>";
        }
    });
    _data += L"</policyAssemblyStatus>";
    _data += L"</heartbeat>";
    _data += L"</HeartBeatRequest>";

    return std::move(_data);
}

static void load_agent_info_update(rapidxml::xml_node<wchar_t>* node, NX::agent_info& info)
{
    try {

        std::wstring    ws;

        // RMS Agent Profile
        rapidxml::xml_node<wchar_t>* agentprofile_node = node->first_node(L"AgentProfile", 0, false);
        if (NULL == agentprofile_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        rapidxml::xml_node<wchar_t>* agentprofile_name_node = agentprofile_node->first_node(L"name", 0, false);
        if (NULL == agentprofile_name_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = agentprofile_name_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_agent_profile_name(ws);

        //  -> Timestamp
        rapidxml::xml_node<wchar_t>* agentprofile_time_node = agentprofile_node->first_node(L"modifiedDate", 0, false);
        if (NULL == agentprofile_time_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = agentprofile_time_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_agent_profile_time(ws);

        // RMS Comm Profile
        rapidxml::xml_node<wchar_t>* commprofile_node = node->first_node(L"commProfile", 0, false);
        if (NULL == commprofile_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        rapidxml::xml_node<wchar_t>* commprofile_name_node = commprofile_node->first_node(L"name", 0, false);
        if (NULL == commprofile_name_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = commprofile_name_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_comm_profile_name(ws);

        //  -> Timestamp
        rapidxml::xml_node<wchar_t>* commprofile_time_node = commprofile_node->first_node(L"modifiedDate", 0, false);
        if (NULL == commprofile_time_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        ws = commprofile_time_node->value();
        if (ws.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        info.set_comm_profile_time(ws);

        //  -> HeartBeat Frequency
        info.set_heartbeat_frequency(load_frequency(commprofile_node, L"heartBeatFrequency"));
        //  -> Log Frequency
        info.set_log_frequency(load_frequency(commprofile_node, L"logFrequency"));

        info.set_id(GLOBAL.agent_info().id());
    }
    catch (const nudf::CException& e) {
        LOG_EXCEPTION(e);
        info.clear();
        throw e;
    }
    catch (const std::exception& e) {
        info.clear();
        throw e;
    }
}

static NX::web::json::value load_keys(rapidxml::xml_node<wchar_t>* node)
{
    std::vector<NX::web::json::value>   keys_vector;

    try {

        rapidxml::xml_node<wchar_t>* keyring_node = node->first_node(L"keyRing", 0, false);
        while (keyring_node) {

            rapidxml::xml_node<wchar_t>* current_keyring = keyring_node;
            keyring_node = keyring_node->next_sibling(L"keyRing", 0, false);

            rapidxml::xml_attribute<wchar_t>* attr_keyring_name = current_keyring->first_attribute(L"KeyRingName", 0, false);
            if (NULL == attr_keyring_name->value() || 0 == (*attr_keyring_name->value()))
                continue;

            const std::wstring wsKeyRingName(attr_keyring_name->value());

            rapidxml::xml_node<wchar_t>* key_node = current_keyring->first_node(L"key", 0, false);
            while (key_node) {

                rapidxml::xml_node<wchar_t>* current_key = key_node;
                key_node = key_node->next_sibling(L"key", 0, false);
            
                rapidxml::xml_node<wchar_t>* key_id_node = current_key->first_node(L"KeyId", 0, false);
                if (NULL == key_id_node)
                    continue;
                rapidxml::xml_node<wchar_t>* key_data_node = current_key->first_node(L"KeyData", 0, false);
                if (NULL == key_data_node)
                    continue;
                rapidxml::xml_node<wchar_t>* key_time_node = current_key->first_node(L"TimeStamp", 0, false);
                if (NULL == key_time_node)
                    continue;

                std::wstring wsKeyId(key_id_node->value());
                std::wstring wsKeyData(key_data_node->value());
                std::wstring wsTimeStamp(key_time_node->value());
                if (wsKeyId.empty() || wsKeyData.empty() || wsTimeStamp.empty()) {
                    continue;
                }

                NX::web::json::value key_obj = NX::web::json::value::object();
                key_obj[L"ring"] = NX::web::json::value::string(wsKeyRingName);
                key_obj[L"id"] = NX::web::json::value::string(wsKeyId);
                key_obj[L"time"] = NX::web::json::value::string(wsTimeStamp);
                // key is in base64 mode
                key_obj[L"key"] = NX::web::json::value::string(wsKeyData);

                keys_vector.push_back(key_obj);
            }
        }
    }
    catch (...) {
        ; // NOTHING
    }

    return NX::web::json::value::array(keys_vector);
}

static NX::web::json::value load_app_white_list(rapidxml::xml_node<wchar_t>* node)
{
    std::vector<NX::web::json::value>   app_vector;

    try {

        rapidxml::xml_node<wchar_t>* app_node = node->first_node(L"Application", 0, false);
        while (app_node) {

            rapidxml::xml_node<wchar_t>* current_app = app_node;
            app_node = app_node->next_sibling(L"Application", 0, false);

            std::wstring    app_name;
            std::wstring    app_publisher;
            std::wstring    app_rights;
            std::vector<std::wstring> rights_vector;

            rapidxml::xml_attribute<wchar_t>* attr_app_name = current_app->first_attribute(L"name", 0, false);
            if (NULL == attr_app_name)
                continue;
            app_name = attr_app_name->value();
            rapidxml::xml_attribute<wchar_t>* attr_app_publisher = current_app->first_attribute(L"publisher", 0, false);
            if (NULL != attr_app_publisher) {
                app_publisher = attr_app_publisher->value();
            }
            rapidxml::xml_attribute<wchar_t>* attr_app_rights = current_app->first_attribute(L"rights", 0, false);
            if (NULL == attr_app_rights) {
                rights_vector.push_back(L"RIGHT_VIEW");
            }
            else {
                if (NULL == attr_app_rights->value()) {
                    rights_vector.push_back(L"RIGHT_VIEW");
                }
                else {
                    app_rights = attr_app_rights->value();
                    nudf::string::Split(app_rights, L',', rights_vector);
                    if (rights_vector.empty()) {
                        rights_vector.push_back(L"RIGHT_VIEW");
                    }
                }
            }

            NX::web::json::value app_obj = NX::web::json::value::object();
            app_obj[L"name"] = NX::web::json::value::string(app_name);
            app_obj[L"publisher"] = NX::web::json::value::string(app_publisher);

            std::vector<NX::web::json::value> rights_json_vector;
            std::for_each(rights_vector.begin(), rights_vector.end(), [&](const std::wstring& s) {rights_json_vector.push_back(NX::web::json::value::string(s)); });

            app_obj[L"rights"] = NX::web::json::value::array(rights_json_vector);
            app_vector.push_back(app_obj);
        }
    }
    catch (...) {
        ; // NOTHING
    }

    return NX::web::json::value::array(app_vector);
}

static NX::web::json::value load_agent_white_list(rapidxml::xml_node<wchar_t>* node)
{
    std::wstring exts;

    NX::web::json::value whitelist = NX::web::json::value::object();

    rapidxml::xml_node<wchar_t>* extensions_node = node->first_node(L"Extensions", 0, false);
    if (extensions_node)
        exts = extensions_node->value();

    if (exts.empty()) {
        whitelist[L"extensions"] = NX::web::json::value::string(L".*");
    }
    else {
        whitelist[L"extensions"] = NX::web::json::value::string(exts);
    }

    rapidxml::xml_node<wchar_t>* apps_node = node->first_node(L"Applications", 0, false);
    if (apps_node) {
        whitelist[L"applications"] = load_app_white_list(apps_node);
    }
    else {
        whitelist[L"applications"] = NX::web::json::value::array();
    }

    return whitelist;
}

static NX::web::json::value load_classify_bundle(rapidxml::xml_node<wchar_t>* node)
{
    NX::web::json::value bundle = NX::web::json::value::object();
    bundle[L"profiles"] = NX::web::json::value::object();
    bundle[L"labels"] = NX::web::json::value::object();

    rapidxml::xml_node<wchar_t>* profiles_node = node->first_node(L"Profiles", 0, false);
    if (NULL == profiles_node)
        throw WIN32ERROR2(ERROR_INVALID_DATA);
    rapidxml::xml_node<wchar_t>* labellist_node = node->first_node(L"LabelList", 0, false);
    if (NULL == labellist_node)
        throw WIN32ERROR2(ERROR_INVALID_DATA);

    NX::web::json::value& profiles = bundle[L"profiles"];
    NX::web::json::value& labels = bundle[L"labels"];

    rapidxml::xml_node<wchar_t>* profile_node = profiles_node->first_node();
    while (profile_node) {
        rapidxml::xml_node<wchar_t>* current_profile = profile_node;
        profile_node = profile_node->next_sibling();
        if (current_profile->type() != rapidxml::node_type::node_element)
            continue;

        std::wstring profile_name = current_profile->name() ? current_profile->name() : L"";
        if (profile_name.empty())
            continue;
        std::wstring profile_label_list = current_profile->value() ? current_profile->value() : L"";
        if (profile_label_list.empty())
            continue;

        std::transform(profile_name.begin(), profile_name.end(), profile_name.begin(), tolower);
        std::transform(profile_label_list.begin(), profile_label_list.end(), profile_label_list.begin(), tolower);
        std::vector<std::wstring> labels;
        nudf::string::Split<wchar_t>(profile_label_list, L',', labels);
        if (labels.empty()) {
            continue;
        }

        // add a profile
        std::vector<NX::web::json::value> labels_vector;
        std::for_each(labels.begin(), labels.end(), [&](const std::wstring& s) {labels_vector.push_back(NX::web::json::value::string(s)); });
        profiles[profile_name] = NX::web::json::value::array(labels_vector);
    }

    rapidxml::xml_node<wchar_t>* label_node = labellist_node->first_node();
    while (label_node) {

        rapidxml::xml_node<wchar_t>* current_label = label_node;
        label_node = label_node->next_sibling();
        if (current_label->type() != rapidxml::node_type::node_element)
            continue;

        std::wstring label_id;
        std::wstring label_name;
        std::wstring label_display_name;
        std::vector<NX::web::json::value> label_values_vector;
        int          default_value = -1;
        bool         multi_select = false;
        bool         mandatory = false;

        rapidxml::xml_attribute<wchar_t>* attr_label_id = current_label->first_attribute(L"id", 0, false);
        if (NULL == attr_label_id)
            continue;
        label_id = attr_label_id->value() ? attr_label_id->value() : L"";
        if (label_id.empty())
            continue;
        
        rapidxml::xml_attribute<wchar_t>* attr_label_name = current_label->first_attribute(L"name", 0, false);
        if (NULL == attr_label_name)
            continue;
        label_name = attr_label_name->value() ? attr_label_name->value() : L"";
        if (label_name.empty())
            continue;

        rapidxml::xml_attribute<wchar_t>* attr_label_dispname = current_label->first_attribute(L"display-name", 0, false);
        if (attr_label_dispname) {
            label_display_name = attr_label_dispname->value() ? attr_label_dispname->value() : L"";
        }

        rapidxml::xml_attribute<wchar_t>* attr_label_defaultval = current_label->first_attribute(L"default-value", 0, false);
        if (attr_label_defaultval && attr_label_defaultval->value()) {
            default_value = _wtoi(attr_label_defaultval->value());
        }

        rapidxml::xml_attribute<wchar_t>* attr_label_mandatory = current_label->first_attribute(L"mandatory", 0, false);
        if (attr_label_mandatory && attr_label_mandatory->value()) {
            mandatory = (0 == _wcsicmp(attr_label_mandatory->value(), L"true")) ? true : false;
        }

        rapidxml::xml_attribute<wchar_t>* attr_label_multiselect = current_label->first_attribute(L"multi-select", 0, false);
        if (attr_label_multiselect && attr_label_multiselect->value()) {
            multi_select = (0 == _wcsicmp(attr_label_multiselect->value(), L"true")) ? true : false;
        }

        std::transform(label_id.begin(), label_id.end(), label_id.begin(), tolower);

        rapidxml::xml_node<wchar_t>* value_node = current_label->first_node(L"VALUE", 0, false);
        while (value_node) {

            rapidxml::xml_node<wchar_t>* current_value = value_node;
            value_node = value_node->next_sibling(L"VALUE", 0, false);
            if (current_value->type() != rapidxml::node_type::node_element)
                continue;

            std::wstring value_text;
            std::wstring sub_label_id;
            std::wstring description;
            int          value_priority = 0;

            rapidxml::xml_attribute<wchar_t>* attr_value_text = current_value->first_attribute(L"value", 0, false);
            if (attr_value_text) {
                value_text = attr_value_text->value() ? attr_value_text->value() : L"";
            }
            if (value_text.empty())
                continue;

            rapidxml::xml_attribute<wchar_t>* attr_value_priority = current_value->first_attribute(L"priority", 0, false);
            if (attr_value_priority && attr_value_priority->value()) {
                value_priority = _wtoi(attr_value_priority->value());
            }

            rapidxml::xml_attribute<wchar_t>* attr_value_sublabel = current_value->first_attribute(L"sub-label", 0, false);
            if (attr_value_sublabel) {
                sub_label_id = attr_value_sublabel->value() ? attr_value_sublabel->value() : L"";
            }

            rapidxml::xml_attribute<wchar_t>* attr_value_description = current_value->first_attribute(L"description", 0, false);
            if (attr_value_description) {
                description = attr_value_description->value() ? attr_value_description->value() : L"";
            }

            NX::web::json::value value_obj = NX::web::json::value::object();
            value_obj[L"value"] = NX::web::json::value::string(value_text);
            value_obj[L"sub-label"] = NX::web::json::value::string(sub_label_id);
            value_obj[L"description"] = NX::web::json::value::string(description);
            value_obj[L"priority"] = NX::web::json::value::number(value_priority);

            label_values_vector.push_back(value_obj);
        }

        // add a profile
        NX::web::json::value label_obj = NX::web::json::value::object();
        label_obj[L"id"] = NX::web::json::value::string(label_id);
        label_obj[L"name"] = NX::web::json::value::string(label_name);
        label_obj[L"display-name"] = NX::web::json::value::string(label_display_name);
        label_obj[L"default-value"] = NX::web::json::value::number(default_value);
        label_obj[L"multi-select"] = NX::web::json::value::boolean(multi_select);
        label_obj[L"mandatory"] = NX::web::json::value::boolean(mandatory);
        label_obj[L"values"] = NX::web::json::value::array(label_values_vector);
        labels[label_id] = label_obj;
    }

    return bundle;
}

bool NX::rest_heartbeat_result::set_result(const std::wstring& xml, bool forced)
{
    bool result = false;

    try {

        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(xml.begin(), xml.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* response = doc.first_node(L"HeartBeatResponse", 0, false);
        if(NULL == response)
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* agent_updates = response->first_node(L"AgentUpdates", 0, false);
        if (NULL == agent_updates) {
            throw std::exception("no updates");
        }

        result = true;

        _forced = forced;

        try {
            load_agent_info_update(agent_updates, _agent_info);
        }
        catch (const nudf::CException& e) {
            _agent_info.clear();
            UNREFERENCED_PARAMETER(e);
            LOGERR(0, L"invalid agent information");
        }
        catch (const std::exception& e) {
            _agent_info.clear();
            UNREFERENCED_PARAMETER(e);
            LOGERR(0, L"invalid agent information");
        }
        catch (...) {
            ; // Nothing
        }

        rapidxml::xml_node<wchar_t>* classify_profile = agent_updates->first_node(L"ClassificationProfile", 0, false);
        if (NULL != classify_profile) {

            rapidxml::xml_node<wchar_t>* classify_node = classify_profile->first_node(L"Classify", 0, false);
            if (NULL != classify_node) {

                rapidxml::xml_node<wchar_t>* whitelist_node = classify_node->first_node(L"WhiteList", 0, false);
                if (NULL != whitelist_node) {
                    _agent_whitelist = load_agent_white_list(whitelist_node);
                }

                // classify bundle
                try {
                    _classify_bundle = load_classify_bundle(classify_node);

                    if (_classify_bundle[L"profiles"].size() == 0) {
                        throw std::exception("empty classify profile list");
                    }
                    if (!_classify_bundle[L"profiles"].has_field(L"default")) {
                        throw std::exception("default classify profile does not exist");
                    }
                    if (_classify_bundle[L"labels"].size() == 0) {
                        throw std::exception("empty classify label list");
                    }
                }
                catch (const nudf::CException& e) {
                    _classify_bundle = NX::web::json::value::object();
                    UNREFERENCED_PARAMETER(e);
                    LOGERR(0, L"invalid classification information");
                }
                catch (const std::exception& e) {
                    _classify_bundle = NX::web::json::value::object();
                    UNREFERENCED_PARAMETER(e);
                    LOGERR(0, L"invalid classification information");
                }
                catch (...) {
                    ; // Nothing
                }
            }
        }

        // Key Update
        rapidxml::xml_node<wchar_t>* keyrings_node = agent_updates->first_node(L"keyRings", 0, false);
        if (NULL != keyrings_node) {
            try {
                _keys_bundle = load_keys(keyrings_node);
            }
            catch (const std::exception& e) {
                _keys_bundle = NX::web::json::value::array();
                UNREFERENCED_PARAMETER(e);
                LOGERR(0, L"invalid keys information");
            }
        }

        // Policy Update
        rapidxml::xml_node<wchar_t>* policybundle_node = agent_updates->first_node(L"policyDeploymentBundle", 0, false);
        if (NULL != policybundle_node) {
            try {
                rapidxml::xml_node<wchar_t>* policybundle_root = policybundle_node->first_node(L"POLICYBUNDLE", 0, false);
                if (NULL != policybundle_root) {
                    rapidxml::xml_attribute<wchar_t>* attr_bundle_time = policybundle_root->first_attribute(L"timestamp", 0, false);
                    if (NULL == attr_bundle_time)
                        throw std::exception("policy time stamp not exist");
                    _policy_bundle_time = attr_bundle_time->value() ? attr_bundle_time->value() : L"";
                    if (_policy_bundle_time.empty())
                        throw std::exception("policy time stamp not exist");

                    rapidxml::print(std::back_inserter(_policy_bundle), *policybundle_root);
                    std::transform(_policy_bundle_time.begin(), _policy_bundle_time.end(), _policy_bundle_time.begin(), toupper);
                    LOGDBG(L"HeatBeat returns policy bundle (%s)", _policy_bundle_time.c_str());
                    if (force_update()) {
                        LOGDMP(xml.c_str());
                    }
                }
            }
            catch (const std::exception& e) {
                _keys_bundle = NX::web::json::value::array();
                UNREFERENCED_PARAMETER(e);
                LOGERR(0, L"invalid keys information");
            }
        }
    }
    catch (const nudf::CException& e) {
        result = false;
        UNREFERENCED_PARAMETER(e);
    }
    catch (const std::exception& e) {
        result = false;
        UNREFERENCED_PARAMETER(e);
    }
    catch (...) {
        result = false;
    }

    return result;
}



//
// class NX::rest::request_audit / NX::rest::response_audit
//

bool process_audit_response(const std::wstring& xml)
{
    bool result = false;

    try {

        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(xml.begin(), xml.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* root = doc.first_node(L"logService", 0, false);
		if (NULL == root) {
			LOGERR(ERROR_INVALID_DATA, L"Bad audit response %s", xml.c_str());
			throw WIN32ERROR2(ERROR_INVALID_DATA);
		}

        rapidxml::xml_node<wchar_t>* response_node = root->first_node(L"logResponse", 0, false);
		if (NULL == response_node) {
			LOGERR(ERROR_INVALID_DATA, L"Bad audit response %s", xml.c_str());
			throw WIN32ERROR2(ERROR_INVALID_DATA);
		}

        rapidxml::xml_node<wchar_t>* result_node = response_node->first_node(L"response", 0, false);
		if (NULL == result_node) {
			LOGERR(ERROR_INVALID_DATA, L"Bad audit response %s", xml.c_str());
			throw WIN32ERROR2(ERROR_INVALID_DATA);
		}
        if (result_node && result_node->value()) {
            result = (0 == _wcsicmp(result_node->value(), L"Success"));
        }
    }
    catch (...) {
        result = false;
    }

    return result;
}


//
// class NX::rest::request_authn / NX::rest::response_authn
//
std::wstring build_authn_param(const std::wstring& user_name, const std::wstring& domain_name, const std::wstring& password)
{
    std::wstring xml = L"<LoginService tenantId=\"N/A\" version=\"1\" agentId=\"";
    xml += GLOBAL.agent_info().id();
    xml += L"\"><LoginRequest><UserName>";
    xml += user_name;
    xml += L"</UserName><Domain>";
    xml += domain_name;
    xml += L"</Domain><Password>";
    xml += password;
    xml += L"</Password><IDPType>AD</IDPType></LoginRequest></LoginService>";

    return std::move(xml);
}

std::wstring NX::rest_authn_result::error_message() const noexcept
{
    std::wstring msg;

    switch (_result)
    {
    case 0:
        msg = L"rest internal exception";
        break;
    case 200:
        msg = L"ok";
        break;
    case 400:
        msg = L"malformed input";
        break;
    case 401:
        msg = L"authorization failed";
        break;
    case 460:
        msg = L"domain name not present";
        break;
    case 461:
        msg = L"general server error";
        break;
    default:
        msg = L"unknown error";
        break;
    }

    return std::move(msg);
}

bool NX::rest_authn_result::set_result(const std::wstring& xml)
{
    bool result = false;

    if (xml.empty()) {
        return false;
    }

    try {

        LOGDMP(L"REST: Authn Result ->");
        LOGDMP(xml.c_str());

        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(xml.begin(), xml.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* root_node = doc.first_node(L"LoginService", 0, false);
        if (NULL == root_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* response_node = root_node->first_node(L"LoginResponse", 0, false);
        if (NULL == response_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* result_node = response_node->first_node(L"LoginResult", 0, false);
        if (NULL == result_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        if (NULL == result_node->value())
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        if (0 == _wcsicmp(result_node->value(), L"true") || 0 == _wcsicmp(result_node->value(), L"200")) {

            rapidxml::xml_node<wchar_t>* uid_node = response_node->first_node(L"UId", 0, false);
            if (NULL == uid_node)
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            _user_id = uid_node->value() ? uid_node->value() : L"";
            if (_user_id.empty()) {
                LOGERR(ERROR_INVALID_DATA, L"login response: no user sid");
                throw WIN32ERROR();
            }

            rapidxml::xml_node<wchar_t>* uname_node = response_node->first_node(L"PrincipalName", 0, false);
            if (NULL == uname_node)
                throw WIN32ERROR2(ERROR_INVALID_DATA);
            _user_name = uname_node->value() ? uname_node->value() : L"";
            if (_user_name.empty()) {
                LOGERR(ERROR_INVALID_DATA, L"login response: no user sid");
                throw WIN32ERROR();
            }

            rapidxml::xml_node<wchar_t>* exptime_node = response_node->first_node(L"ExpireTime", 0, false);
            if (NULL != exptime_node && NULL != exptime_node->value()) {
                FILETIME ft = { 0, 0 };
                bool utc = true;
                if (nudf::string::ToSystemTime<wchar_t>(exptime_node->value(), &ft, &utc)) {
                    if (!utc) {
                        LocalFileTimeToFileTime(&ft, &_expire_time);
                    }
                    else {
                        memcpy(&_expire_time, &ft, sizeof(FILETIME));
                    }
                }
            }

            rapidxml::xml_node<wchar_t>* uattrs_node = response_node->first_node(L"UserAttributes", 0, false);
            if (NULL != uattrs_node) {
                rapidxml::xml_node<wchar_t>* uattr_node = uattrs_node->first_node(L"Attribute", 0, false);
                while (uattr_node) {

                    rapidxml::xml_node<wchar_t>* current_uattr = uattrs_node;
                    uattrs_node = uattrs_node->next_sibling(L"Attribute", 0, false);

                    rapidxml::xml_node<wchar_t>* attr_name_node = current_uattr->first_node(L"Name", 0, false);
                    rapidxml::xml_node<wchar_t>* attr_value_node = current_uattr->first_node(L"Value", 0, false);
                    if (attr_name_node && attr_name_node->value() && attr_value_node && attr_value_node->value()) {
                        _user_attrs[attr_name_node->value()] = attr_value_node->value();
                    }
                }
            }

            // succeed
            _result = 200;
            result = true;
        }
        else {

            rapidxml::xml_node<wchar_t>* error_node = response_node->first_node(L"error", 0, false);
            if (error_node) {
                rapidxml::xml_node<wchar_t>* error_code_node = error_node->first_node(L"errorCode", 0, false);
                if (error_code_node && error_code_node->value()) {
                    int e = _wtoi(error_code_node->value());
                    if (0 != e) {
                        result = true;
                        _result = e;
                        assert(_result != 200);
                    }
                }
            }
        }
    }
    catch (...) {
        _result = 461;
        throw std::exception("fail to parse response");
    }

    return result;
}

void NX::rest_authn_result::clear() noexcept
{
    _result = 0;
    _user_id.clear();
    _user_attrs.clear();
}




//
// class NX::rest::request_eval / NX::rest::response_eval
//

static __int64 ipv4_string_to_ll(const std::wstring& sip)
{
    IN_ADDR addr = { 0 };
    LPCWSTR terminator = NULL;

    typedef LONG (NTAPI* RtlIpv4StringToAddressW_t)(
        _In_ PCTSTR  S,
        _In_ BOOLEAN Strict,
        _Out_ LPCTSTR *Terminator,
        _Out_ IN_ADDR *Addr
        );

    static RtlIpv4StringToAddressW_t RtlIpv4StringToAddressW = NULL;
    if (NULL == RtlIpv4StringToAddressW) {
        HMODULE hNtDll= GetModuleHandleW(L"ntdll.dll");
        if (NULL != hNtDll) {
            RtlIpv4StringToAddressW = (RtlIpv4StringToAddressW_t)GetProcAddress(hNtDll, "RtlIpv4StringToAddressW");
        }
    }

    if (RtlIpv4StringToAddressW) {
        if (ERROR_SUCCESS == RtlIpv4StringToAddressW(sip.c_str(), FALSE, &terminator, &addr)) {
            return (__int64)addr.S_un.S_addr;
        }
    }

    return 0;
}
static std::wstring ipv4_string_to_ll_sz(const std::wstring& sip)
{
    __int64 ip = ipv4_string_to_ll(sip);
    std::wstring s;
    swprintf_s(nudf::string::tempstr<wchar_t>(s, MAX_PATH), MAX_PATH, L"%I64d", ip);
    return std::move(s);
}


std::wstring build_eval_param(const NX::EVAL::eval_object& eo)
{
    static long volatile s_query_id = 1;

    std::wstring _data;
    __int64 d_query_id = _wtoi(GLOBAL.agent_info().id().c_str());
    d_query_id <<= 32;
    long d_query_id_low = InterlockedAdd(&s_query_id, 1);
    if (d_query_id_low > 0x70000000) {
        s_query_id = 1;
    }
    d_query_id += d_query_id_low;
    std::wstring ws_query_id;
    swprintf_s(nudf::string::tempstr<wchar_t>(ws_query_id, 64), 64, L"%I64d", d_query_id);

    std::wstring user_name = eo.attributes().get_single(L"user.name").serialize();
    std::wstring user_id = eo.attributes().get_single(L"user.id").serialize();
    std::wstring app_name = eo.attributes().get_single(L"application.name").serialize();
    std::wstring app_pid = eo.attributes().get_single(L"application.pid").serialize();
    std::wstring host_ip = eo.attributes().get_single(L"host.inet_addr").serialize();
    // convert ip string to long, and convert long to string
    //host_ip = ipv4_string_to_ll_sz(host_ip);
    std::wstring res_name = eo.attributes().get_single(L"resource.fso.path").serialize();
    std::map<std::wstring, std::vector<std::wstring>> user_attr_map;
    std::map<std::wstring, std::vector<std::wstring>> app_attr_map;
    std::map<std::wstring, std::vector<std::wstring>> host_attr_map;
    std::map<std::wstring, std::vector<std::wstring>> res_attr_map;
    std::for_each(eo.attributes().cbegin(), eo.attributes().cend(), [&](const NX::EVAL::attribute_multimap::value_type& v) {
        if (boost::algorithm::istarts_with(v.first, L"user.")) {
            if (v.first == L"user.name" || v.first == L"user.id") {
                return;
            }
            user_attr_map[v.first.substr(5)].push_back(v.second.serialize());
        }
        else if (boost::algorithm::istarts_with(v.first, L"application.")) {
            if (v.first == L"application.name" || v.first == L"application.pid") {
                return;
            }
            app_attr_map[v.first.substr(12)].push_back(v.second.serialize());
        }
        else if (boost::algorithm::istarts_with(v.first, L"host.")) {
            if (v.first == L"host.inet_addr") {
                return;
            }
            host_attr_map[v.first.substr(5)].push_back(v.second.serialize());
        }
        else if (boost::algorithm::istarts_with(v.first, L"resource.fso.")) {
            if (v.first == L"resource.fso.path") {
                return;
            }
            res_attr_map[v.first.substr(13)].push_back(v.second.serialize());
        }
        else {
            ; // environment attributes
        }
    });
    if (app_pid.empty()) app_pid = L"0";
    _data = L"<PolicyQueries xmlns=\"https://www.nextlabs.com/rms/policy/eval\">";
    _data += L"<PolicyQuery id=\"" + ws_query_id + L"\">";
    _data += L"<Request>";
    _data += L"<Subject>";
    _data += L"<User name=\"" + user_name + L"\" id=\"" + user_id + L"\">";
    std::for_each(user_attr_map.begin(), user_attr_map.end(), [&](const std::pair<std::wstring, std::vector<std::wstring>>& v) {
        // add user attributes
        _data += L"<Attribute name=\"" + v.first + L"\">";
        std::for_each(v.second.begin(), v.second.end(), [&](const std::wstring& attr) {
            _data += L"<AttributeValue>" + attr + L"</AttributeValue>";
        });
        _data += L"</Attribute>";
    });
    _data += L"</User>";
    _data += L"<Application name=\"" + app_name + L"\" pid=\"" + app_pid + L"\">";
    std::for_each(app_attr_map.begin(), app_attr_map.end(), [&](const std::pair<std::wstring, std::vector<std::wstring>>& v) {
        // add user attributes
        _data += L"<Attribute name=\"" + v.first + L"\">";
        std::for_each(v.second.begin(), v.second.end(), [&](const std::wstring& attr) {
            _data += L"<AttributeValue>" + attr + L"</AttributeValue>";
        });
        _data += L"</Attribute>";
    });
    _data += L"</Application>";
    _data += L"<Host ip=\"" + host_ip + L"\">";
    std::for_each(host_attr_map.begin(), host_attr_map.end(), [&](const std::pair<std::wstring, std::vector<std::wstring>>& v) {
        // add user attributes
        _data += L"<Attribute name=\"" + v.first + L"\">";
        std::for_each(v.second.begin(), v.second.end(), [&](const std::wstring& attr) {
            _data += L"<AttributeValue>" + attr + L"</AttributeValue>";
        });
        _data += L"</Attribute>";
    });
    _data += L"</Host>";
    _data += L"</Subject>";
    _data += L"<Resources>";
    _data += L"<Resource type=\"fso\" dimension=\"from\" name=\"" + res_name + L"\">";
    std::for_each(res_attr_map.begin(), res_attr_map.end(), [&](const std::pair<std::wstring, std::vector<std::wstring>>& v) {
        // add user attributes
        _data += L"<Attribute name=\"" + v.first + L"\">";
        std::for_each(v.second.begin(), v.second.end(), [&](const std::wstring& attr) {
            _data += L"<AttributeValue>" + attr + L"</AttributeValue>";
        });
        _data += L"</Attribute>";
    });
    _data += L"</Resource>";
    _data += L"</Resources>";
    _data += L"<Environments>";
    _data += L"<Environment name=\"environment\">";
    _data += L"<Attribute name=\"time_since_last_heartbeat\">";
    _data += L"<AttributeValue>" + eo.attributes().get_single(L"environment.time_since_last_heartbeat").serialize() + L"</AttributeValue>";
    _data += L"</Attribute>";
    _data += L"<Attribute name=\"dont-care-acceptable\">";
    _data += L"<AttributeValue>yes</AttributeValue>";
    _data += L"</Attribute>";
    _data += L"</Environment>";
    _data += L"<Environment name=\"current_time\">";
    _data += L"<Attribute name=\"identity\">";
    _data += L"<AttributeValue>" + eo.attributes().get_single(L"environment.current_time.identity").serialize() + L"</AttributeValue>";
    _data += L"</Attribute>";
    _data += L"</Environment>";
    _data += L"</Environments>";
    _data += L"<Rights>";
    _data += L"<Right>RIGHT_VIEW</Right>";
    _data += L"<Right>RIGHT_EDIT</Right>";
    _data += L"<Right>RIGHT_PRINT</Right>";
    _data += L"<Right>RIGHT_CLIPBOARD</Right>";
    _data += L"<Right>RIGHT_SAVEAS</Right>";
    _data += L"<Right>RIGHT_DECRYPT</Right>";
    _data += L"<Right>RIGHT_SCREENCAP</Right>";
    _data += L"<Right>RIGHT_SEND</Right>";
    _data += L"<Right>RIGHT_CLASSIFY</Right>";
    _data += L"</Rights>";
    _data += L"<performObligations>true</performObligations>";
    _data += L"</Request>";
    _data += L"</PolicyQuery>";
    _data += L"</PolicyQueries>";

    return std::move(_data);
}

bool process_eval_response(const std::wstring& body, NX::EVAL::eval_result* evalresult)
{
    bool result = false;

    try {

        int error_code = 0;

        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(body.begin(), body.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* root_node = doc.first_node(L"PolicyQuery", 0, false);
        if (NULL == root_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        rapidxml::xml_node<wchar_t>* response_node = root_node->first_node(L"Response", 0, false);
        if (NULL == response_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* rights_node = response_node->first_node(L"Rights", 0, false);
        rapidxml::xml_node<wchar_t>* obligations_node = response_node->first_node(L"Obligations", 0, false);
        rapidxml::xml_node<wchar_t>* error_node = response_node->first_node(L"Error", 0, false);

        if (error_node) {
            rapidxml::xml_node<wchar_t>* error_code_node = error_node->first_node(L"ErrorCode", 0, false);
            if (error_code_node && error_code_node->value())
                error_code = _wtoi(error_code_node->value());
        }

        if (0 != error_code)
            throw std::exception("Eval failed");

        result = true;

        if (rights_node) {
            rapidxml::xml_node<wchar_t>* right_node = rights_node->first_node(L"RIGHT", 0, false);
            while (right_node) {
                rapidxml::xml_node<wchar_t>* current_right = rights_node;
                rights_node = rights_node->next_sibling(L"RIGHT", 0, false);
                if (current_right->value()) {
                    evalresult->grant_rights(current_right->value());
                }
            }
        }

        // Obligations
        if (obligations_node != NULL) {

            rapidxml::xml_node<wchar_t>* ob_node = obligations_node->first_node(L"RIGHT", 0, false);
            while (ob_node) {

                rapidxml::xml_node<wchar_t>* current_ob = ob_node;
                ob_node = ob_node->next_sibling(L"Obligation", 0, false);

                std::wstring ob_name;
                std::map<std::wstring, std::wstring> ob_params;

                rapidxml::xml_attribute<wchar_t>* attr_ob_name = current_ob->first_attribute(L"name", 0, false);
                if (NULL == attr_ob_name)
                    continue;
                ob_name = attr_ob_name->value() ? attr_ob_name->value() : L"";
                if (ob_name.empty())
                    continue;

                rapidxml::xml_node<wchar_t>* ob_param_node = current_ob->first_node(L"Attribute", 0, false);
                while (ob_param_node) {

                    rapidxml::xml_node<wchar_t>* current_param = ob_param_node;
                    ob_param_node = ob_param_node->next_sibling(L"Attribute", 0, false);

                    std::wstring param_name;
                    std::wstring param_value;

                    rapidxml::xml_attribute<wchar_t>* attr_param_name = current_param->first_attribute(L"name", 0, false);
                    if (NULL == attr_param_name)
                        continue;
                    rapidxml::xml_node<wchar_t>* param_value_node = current_param->first_node(L"AttributeValue", 0, false);
                    if (NULL == param_value_node)
                        continue;
                    param_name = attr_param_name->value() ? attr_param_name->value() : L"";
                    param_value = param_value_node->value() ? param_value_node->value() : L"";

                    if(!param_name.empty() && !param_value.empty())
                        ob_params[param_name] = param_value;
                }

                evalresult->hit_obligation(std::shared_ptr<NX::EVAL::obligation>(new NX::EVAL::obligation(ob_name, ob_params)));
            }
        }
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    catch (...) {
        result = false;
    }

    return result;
}

//
// class NX::rest::details::request
//

std::wstring build_checkupdate_param()
{
    // Set Request         
    std::wstring reqinfo = L"<CheckUpdates><CheckUpdatesRequest><CurrentVersion>";
    //   - CurrentVersion
    reqinfo += GLOBAL.product().version().version_str();
    reqinfo += L"</CurrentVersion><LastUpdatedDate></LastUpdatedDate><groupInformation><groupId>";
    //   - groupId
    reqinfo += GLOBAL.register_info().group_id().empty() ? L"NextLabs" : GLOBAL.register_info().group_id();
    reqinfo += L"</groupId><groupName>";
    //   - groupName
    reqinfo += GLOBAL.register_info().group_name().empty() ? L"NextLabs" : GLOBAL.register_info().group_name();
    reqinfo += L"</groupName></groupInformation><tenantId>";
    //   - tenantId
    reqinfo += GLOBAL.register_info().tenant_id().empty() ? L"N/A" : GLOBAL.register_info().tenant_id();
    reqinfo += L"</tenantId><agentId>";
    //   - agentId
    reqinfo += GLOBAL.agent_info().id();
    reqinfo += L"</agentId><osType>Windows</osType><osVersion>";
    //   - osVersion
    reqinfo += NX::sys::os::os_name();
#ifdef _WIN64
    reqinfo += L"</osVersion><architecture>64bit</architecture></CheckUpdatesRequest></CheckUpdates>";
#else
    reqinfo += L"</osVersion><architecture>32bit</architecture></CheckUpdatesRequest></CheckUpdates>";
#endif

    return std::move(reqinfo);
}

bool NX::rest_checkupdate_result::set_result(const std::wstring& body)
{
    bool result = false;

    try {

        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(body.begin(), body.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* root_node = doc.first_node(L"CheckUpdates", 0, false);
        if (NULL == root_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* response_node = root_node->first_node(L"CheckUpdatesResponse", 0, false);
        if (NULL == response_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* new_version_node = response_node->first_node(L"NewVersion", 0, false);
        if (NULL == new_version_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        if (NULL == new_version_node->value())
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* checksum_node = response_node->first_node(L"CheckSum", 0, false);
        if (NULL == checksum_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        if (NULL == checksum_node->value())
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        rapidxml::xml_node<wchar_t>* url_node = response_node->first_node(L"DownloadURL", 0, false);
        if (NULL == url_node)
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        if (NULL == url_node->value())
            throw WIN32ERROR2(ERROR_INVALID_DATA);

        _new_version = new_version_node->value();
        if (_new_version == std::wstring(L"0")) {
            // "0" means no update
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _checksum = checksum_node->value();
        _download_url = url_node->value();
        if (_new_version.empty() || _checksum.empty() || _download_url.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        result = true;
    }
    catch (...) {
        result = false;
        _new_version.clear();
        _download_url.clear();
        _checksum.clear();
    }

    return result;
}
