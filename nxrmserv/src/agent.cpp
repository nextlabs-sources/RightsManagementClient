

#include <Windows.h>


#include <string>
#include <algorithm>
#include <sstream>

#include <nudf\web\json.hpp>
#include <nudf\string.hpp>
#include <nudf\convert.hpp>

#pragma warning(push)
#pragma warning(disable: 4244)
#include <rapidxml\rapidxml.hpp>
#include <rapidxml\rapidxml_utils.hpp>
#pragma warning(pop)


#include "nxrmserv.h"
#include "sconfig.hpp"
#include "secure.hpp"
#include "agent.hpp"


using namespace NX;


static const std::wstring default_cert(L"MIIDmzCCAoOgAwIBAgIEI8+FCTANBgkqhkiG9w0BAQsFADB+MQswCQYDVQQGEwJVUzELMAkGA1UECBMCQ0ExEjAQBgNVBAcTCVNhbiBNYXRlbzERMA8GA1UEChMITmV4dExhYnMxGjAYBgNVBAsTEVJpZ2h0cyBNYW5hZ2VtZW50MR8wHQYDVQQDExZSaWdodHMgTWFuYWdlbWVudCBUZW1wMB4XDTE1MDUyNzE0MDI0MVoXDTI1MDUyNDE0MDI0MVowfjELMAkGA1UEBhMCVVMxCzAJBgNVBAgTAkNBMRIwEAYDVQQHEwlTYW4gTWF0ZW8xETAPBgNVBAoTCE5leHRMYWJzMRowGAYDVQQLExFSaWdodHMgTWFuYWdlbWVudDEfMB0GA1UEAxMWUmlnaHRzIE1hbmFnZW1lbnQgVGVtcDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPUFAKjEmwL/oWQpafK5vOz8/dOEf9mmcjKawhxNhJJ5R+4olKHF9ZaKMve542ehSKngA53buaPsvP96ix7j8O8E2DHnYqbR5I9jNfZarIWjAwfO94TDvQovuIFBCWMnJXsRUtVm36cF6WpQdqctIbgvSbjGbgYacqGZ6QaaWrySGxupjyB8lfD6dYBG5lXFRcFA7QQVbNAGM7Xis2S3sPZOch4VJK7faX2xRyW6sIKL0FU8W9HCbm2PjG+XBr+dmsP3lk6HOqlSEy55HRYldMI/KCSlTGIcUHjH0qpiBxceSHILgY+YOqJ3l6/d8k9ui3MK2XGUhNFgwwLYFruk1l8CAwEAAaMhMB8wHQYDVR0OBBYEFGKcZJB9ZVJ6q/T2DNJIUoQEqx6sMA0GCSqGSIb3DQEBCwUAA4IBAQA9oFoR9GYVvba1WTdq2sl7kqTxqTPkUtD5LGi5A7q1yxMkAwsR2kW00L5dbRmADT7PjE3x42V2ZHHuYhDjGg/zm+2xHVrUWl2ZxHodmHz6+qDbdAZ3+9U4Zz7nt2oxDFghp/eE1adXa2kfAIZzn8VVamD6TS9O0R/KyXToYgpjLmz6QD9GFsz5wGbVsnJGWTxfiNjX3LnFIkqJU8rHn1DcMyB3/xd3ytUJzKrAnD8f46JpfR1amJOQAxiDy5+kW1OnclGBImS9iisvCmwU3+UNixbFAAxymBA9VvAO90sw0tHcLN7M1NSpenVlAnJTHhGuLSepk8gv4jAEsa9+DPKR");

void register_info::load(const std::wstring& file)
{
    try {

        std::string utf8filename = nudf::util::convert::Utf16ToUtf8(file);
        rapidxml::file<char> xmlFile(utf8filename.c_str());
        rapidxml::xml_document<> doc;

        doc.parse<0>(xmlFile.data());

        rapidxml::xml_node<>* root = doc.first_node("REGISTER", 0, false);
        if (NULL == root) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Parse server information
        rapidxml::xml_node<>* server_node = root->first_node("SERVER", 0, false);
        if (NULL == server_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        std::wstring url = nudf::util::convert::Utf8ToUtf16(server_node->value());
        if (0 == _wcsnicmp(url.c_str(), L"https://", 8)) {
            _server = L"https://";
            url = url.substr(8);
        }
        else if (0 == _wcsnicmp(url.c_str(), L"http://", 7)) {
            _server = L"http://";
            url = url.substr(7);
        }
        else {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        std::wstring::size_type pos = url.find_first_of(L'/');
        if (pos == std::wstring::npos) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _server += url.substr(0, pos);
        _service_base = url.substr(pos);
        if (L'/' != _service_base.c_str()[_service_base.length() - 1]) {
            _service_base += L"/";
        }

        rapidxml::xml_node<>* tenant_node = root->first_node("TENANTID", 0, false);
        if (NULL != tenant_node) {
            _tenant_id = nudf::util::convert::Utf8ToUtf16(tenant_node->value());
        }

        rapidxml::xml_node<>* groupid_node = root->first_node("GROUPID", 0, false);
        if (NULL != groupid_node) {
            _group_id = nudf::util::convert::Utf8ToUtf16(groupid_node->value());
        }

        rapidxml::xml_node<>* groupname_node = root->first_node("GROUPNAME", 0, false);
        if (NULL != groupname_node) {
            _group_name = nudf::util::convert::Utf8ToUtf16(groupname_node->value());
        }

        rapidxml::xml_node<>* cert_node = root->first_node("CERTIFICATE", 0, false);
        if (NULL != cert_node) {
            _cert = nudf::util::convert::Utf8ToUtf16(cert_node->value());
        }
        if (_cert.empty()) {
            _cert = default_cert;
        }

        rapidxml::xml_node<>* authn_node = root->first_node("AUTHN", 0, false);
        if (authn_node != NULL) {
            rapidxml::xml_node<>* authntype_node = authn_node->first_node("TYPE", 0, false);
            if (NULL != authntype_node) {
                _authn_type = nudf::util::convert::Utf8ToUtf16(authntype_node->value());
                std::transform(_authn_type.begin(), _authn_type.end(), _authn_type.begin(), tolower);
                if (_authn_type == L"local") {
                    ; // nothing
                }
                else if (_authn_type == L"external") {
                    rapidxml::xml_node<>* authndomains_node = authn_node->first_node("DOMAINS", 0, false);
                    if (NULL != authndomains_node) {
                        rapidxml::xml_node<>* domain_node = authndomains_node->first_node("DOMAIN", 0, false);
                        while (domain_node) {
                            std::wstring str_domain = nudf::util::convert::Utf8ToUtf16(domain_node->value());
                            if (!str_domain.empty()) {
                                _authn_domains.push_back(str_domain);
                            }
                            domain_node = domain_node->next_sibling("DOMAIN", 0, false);
                        }
                    }
                }
                else {
                    // unknown authn type: treat as internal
                    _authn_type = L"local";
                }
            }
        }
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        throw std::exception("register_info: invalid content in register.xml");
    }
    catch (const std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        throw std::exception("register_info: invalid content in register.xml");
    }
}

agent_info::agent_info() : _heartbeat_frequency(0), _log_frequency(0), _heartbeat_time(0)
{
}

agent_info::~agent_info()
{
}

void agent_info::set_heartbeat_time(const std::wstring& time) noexcept
{
    FILETIME ft = { 0, 0 };
    bool utc = true;
    if (nudf::string::ToSystemTime<wchar_t>(time, &ft, &utc)) {
        _heartbeat_time = ft.dwHighDateTime;
        _heartbeat_time <<= 32;
        _heartbeat_time += ft.dwLowDateTime;
    }
}
void agent_info::set_heartbeat_time(const FILETIME* ft/* = NULL*/) noexcept
{
    if (NULL != ft && 0 != ft->dwHighDateTime && 0 != ft->dwLowDateTime) {
        _heartbeat_time = ft->dwHighDateTime;
        _heartbeat_time <<= 32;
        _heartbeat_time += ft->dwLowDateTime;
    }
    else {
        FILETIME tmp_ft = { 0, 0 };
        GetSystemTimeAsFileTime(&tmp_ft);
        _heartbeat_time = tmp_ft.dwHighDateTime;
        _heartbeat_time <<= 32;
        _heartbeat_time += tmp_ft.dwLowDateTime;
    }
}

std::wstring agent_info::heartbeat_time_text() const noexcept
{

    if (0 == _heartbeat_time) {
        return std::wstring(L"N/A");
    }
    else {
        std::wstring ws_heartbeat_time;
        FILETIME ft = { 0, 0 };
        FILETIME lft = { 0, 0 };
        SYSTEMTIME st = { 0 };
        ft.dwHighDateTime = (unsigned long)(_heartbeat_time >> 32);
        ft.dwLowDateTime = (unsigned long)(_heartbeat_time);
        FileTimeToLocalFileTime(&ft, &lft);
        FileTimeToSystemTime(&lft, &st);
        static const wchar_t* months[12] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };
        const wchar_t* ext = L" AM";
        if (st.wHour >= 12) {
            ext = L" PM";
            if (st.wHour > 12) {
                st.wHour -= 12;
            }
        }
        swprintf_s(nudf::string::tempstr<wchar_t>(ws_heartbeat_time, 128), 128, L"%02d %s %04d, %02d:%02d %s", st.wDay, months[st.wMonth - 1], st.wYear, st.wHour, st.wMinute, ext);
        return ws_heartbeat_time;
    }
    __assume(0);
}

agent_info& agent_info::operator = (const agent_info& other) noexcept
{
    if (this != &other) {
        _id = other.id();
        _cert = other.cert();
        _heartbeat_frequency = other.heartbeat_frequency();
        _log_frequency = other.log_frequency();
        _agent_profile_name = other.agent_profile_name();
        _agent_profile_time = other.agent_profile_time();
        _comm_profile_name = other.comm_profile_name();
        _comm_profile_time = other.comm_profile_time();
        _heartbeat_time = other.heartbeat_time();
    }
    return *this;
}

bool agent_info::operator == (const agent_info& other) noexcept
{
    if (this == &other) {
        return true;
    }

    return (_id == other.id()
        && _cert == other.cert()
        && _heartbeat_frequency == other.heartbeat_frequency()
        && _log_frequency == other.log_frequency()
        && _agent_profile_name == other.agent_profile_name()
        && _agent_profile_time == other.agent_profile_time()
        && _comm_profile_name == other.comm_profile_name()
        && _comm_profile_time == other.comm_profile_time());
}

void agent_info::clear() noexcept
{
    _id.clear();
    _cert.clear();
    _heartbeat_frequency = 0;
    _log_frequency = 0;
    _agent_profile_name.clear();
    _agent_profile_time.clear();
    _comm_profile_name.clear();
    _comm_profile_time.clear();
}

static const secure_mem key_base(std::vector<unsigned char>({
    0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F,
    0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F,
    0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F,
    0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F
}));

static const std::wstring key_agent_id(L"agent_id");
static const std::wstring key_agent_cert(L"agent_cert");
static const std::wstring key_heartbeat_frequency(L"heartbeat_frequency");
static const std::wstring key_log_frequency(L"log_frequency");
static const std::wstring key_agent_profile_name(L"agent_profile_name");
static const std::wstring key_agent_profile_time(L"agent_profile_time");
static const std::wstring key_comm_profile_name(L"comm_profile_name");
static const std::wstring key_comm_profile_time(L"comm_profile_time");
static const std::wstring key_heartbeat_time(L"heartbeat_time");

void agent_info::load(const std::wstring& file)
{
    std::vector<unsigned char> key = key_base.decrypt();
    std::string s = sconfig::load(file, key);
    std::stringstream ss(s);
    NX::web::json::value v = NX::web::json::value::parse(ss);

    if (!v.has_field(key_agent_id)) {
        throw std::exception("agent profile: no id");
    }
    if (!v.has_field(key_agent_cert)) {
        throw std::exception("agent profile: no certificate");
    }
    if (!v.has_field(key_heartbeat_frequency)) {
        throw std::exception("agent profile: no heartbeat frequency");
    }
    if (!v.has_field(key_log_frequency)) {
        throw std::exception("agent profile: no log frequency");
    }
    if (!v.has_field(key_agent_profile_name)) {
        throw std::exception("agent profile: no agent profile name");
    }
    if (!v.has_field(key_agent_profile_time)) {
        throw std::exception("agent profile: no agent profile time");
    }
    if (!v.has_field(key_comm_profile_name)) {
        throw std::exception("agent profile: no communication profile name");
    }
    if (!v.has_field(key_comm_profile_time)) {
        throw std::exception("agent profile: no communication profile time");
    }

    if (v.has_field(key_heartbeat_time) && v[key_heartbeat_time].is_string()) {
        FILETIME ft = { 0, 0 };
        bool utc = true;
        if (nudf::string::ToSystemTime<wchar_t>(v[key_heartbeat_time].as_string(), &ft, &utc)) {
            _heartbeat_time = ft.dwHighDateTime;
            _heartbeat_time <<= 32;
            _heartbeat_time += ft.dwLowDateTime;
        }
    }

    _id = v[key_agent_id].as_string();
    std::transform(_id.begin(), _id.end(), _id.begin(), tolower);

    _cert = v[key_agent_cert].as_string();
    std::transform(_cert.begin(), _cert.end(), _cert.begin(), toupper);

    _heartbeat_frequency = v[key_heartbeat_frequency].as_integer();
    _log_frequency = v[key_log_frequency].as_integer();

    _agent_profile_name = v[key_agent_profile_name].as_string();
    std::transform(_agent_profile_name.begin(), _agent_profile_name.end(), _agent_profile_name.begin(), toupper);
    _agent_profile_time = v[key_agent_profile_time].as_string();
    std::transform(_agent_profile_time.begin(), _agent_profile_time.end(), _agent_profile_time.begin(), toupper);
    _comm_profile_name = v[key_comm_profile_name].as_string();
    std::transform(_comm_profile_name.begin(), _comm_profile_name.end(), _comm_profile_name.begin(), toupper);
    _comm_profile_time = v[key_comm_profile_time].as_string();
    std::transform(_comm_profile_time.begin(), _comm_profile_time.end(), _comm_profile_time.begin(), toupper);
}

void agent_info::save(const std::wstring& file)
{
    NX::web::json::value v = NX::web::json::value::object();
    std::wstring hb_time = L"";
    if (is_heartbeat_time_valid()) {
        FILETIME ft = { 0, 0 };
        ft.dwHighDateTime = (unsigned long)(_heartbeat_time >> 32);
        ft.dwLowDateTime = (unsigned long)(_heartbeat_time);
        hb_time = nudf::string::FromSystemTime<wchar_t>(&ft, true);
    }
    v[key_agent_id] = NX::web::json::value::string(_id);
    v[key_agent_cert] = NX::web::json::value::string(_cert);
    v[key_heartbeat_frequency] = NX::web::json::value::number(_heartbeat_frequency);
    v[key_log_frequency] = NX::web::json::value::number(_log_frequency);
    v[key_agent_profile_name] = NX::web::json::value::string(_agent_profile_name);
    v[key_agent_profile_time] = NX::web::json::value::string(_agent_profile_time);
    v[key_comm_profile_name] = NX::web::json::value::string(_comm_profile_name);
    v[key_comm_profile_time] = NX::web::json::value::string(_comm_profile_time);
    v[key_heartbeat_time] = NX::web::json::value::string(hb_time);

    std::wstring ws = v.serialize();
    std::string s = NX::utility::conversions::utf16_to_utf8(ws);
    std::vector<unsigned char> key = key_base.decrypt();
    sconfig::save(file, s, key);
}

std::string agent_info::decrypt(const std::wstring& file) noexcept
{
    std::vector<unsigned char> key = key_base.decrypt();
    std::string s = sconfig::load(file, key);
    return std::move(s);
}

void agent_info::load_xml(const std::wstring& xml)
{
    try {

        rapidxml::xml_document<wchar_t> doc;
        doc.parse<0>((wchar_t*)xml.c_str());

        rapidxml::xml_node<wchar_t>* root_node = doc.first_node(L"RegisterAgentResponse", 0, false);

        if (NULL == root_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Get Cert <certificate>
        rapidxml::xml_node<wchar_t>* cert_node = root_node->first_node(L"certificate", 0, false);
        if (NULL == cert_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _cert = cert_node->value();
        if (_cert.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Get StartupConf <StartupConfiguration>
        rapidxml::xml_node<wchar_t>* startconf_node = root_node->first_node(L"StartupConfiguration", 0, false);
        if (NULL == startconf_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // Agent Id
        rapidxml::xml_node<wchar_t>* agentid_node = startconf_node->first_node(L"id", 0, false);
        if (NULL == agentid_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _id = agentid_node->value();
        if (_id.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // RMS Agent Profile
        rapidxml::xml_node<wchar_t>* agentprofile_node = startconf_node->first_node(L"AgentProfile", 0, false);
        if (NULL == agentprofile_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        rapidxml::xml_node<wchar_t>* agentprofile_name_node = agentprofile_node->first_node(L"name", 0, false);
        if (NULL == agentprofile_name_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _agent_profile_name = agentprofile_name_node->value();
        if (_agent_profile_name.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Timestamp
        rapidxml::xml_node<wchar_t>* agentprofile_time_node = agentprofile_node->first_node(L"modifiedDate", 0, false);
        if (NULL == agentprofile_time_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _agent_profile_time = agentprofile_time_node->value();
        if (_agent_profile_time.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }

        // RMS Comm Profile
        rapidxml::xml_node<wchar_t>* commprofile_node = startconf_node->first_node(L"commProfile", 0, false);
        if (NULL == commprofile_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Name
        rapidxml::xml_node<wchar_t>* commprofile_name_node = commprofile_node->first_node(L"name", 0, false);
        if (NULL == commprofile_name_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _comm_profile_name = commprofile_name_node->value();
        if (_comm_profile_name.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        //  -> Timestamp
        rapidxml::xml_node<wchar_t>* commprofile_time_node = commprofile_node->first_node(L"modifiedDate", 0, false);
        if (NULL == commprofile_time_node) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        _comm_profile_time = commprofile_time_node->value();
        if (_comm_profile_time.empty()) {
            throw WIN32ERROR2(ERROR_INVALID_DATA);
        }
        
        //  -> HeartBeat Frequency
        rapidxml::xml_node<wchar_t>* commprofile_hb_freq_node = commprofile_node->first_node(L"heartBeatFrequency", 0, false);
        if (NULL != commprofile_hb_freq_node) {
            _heartbeat_frequency = 86400; // 1 day
            rapidxml::xml_node<wchar_t>* commprofile_hb_freqtime_node = commprofile_hb_freq_node->first_node(L"time", 0, false);
            if (NULL != commprofile_hb_freqtime_node) {
                std::wstring wsFrequency = commprofile_hb_freqtime_node->value();
                if (!wsFrequency.empty()) {
                    int nFrequency = 0;
                    if (nudf::string::ToInt<wchar_t>(wsFrequency, &nFrequency) && nFrequency > 0) {
                        _heartbeat_frequency = nFrequency;
                        rapidxml::xml_node<wchar_t>* commprofile_hb_frequnit_node = commprofile_hb_freq_node->first_node(L"time-Unit", 0, false);
                        if (NULL != commprofile_hb_frequnit_node) {
                            std::wstring wsTimeUnit = commprofile_hb_frequnit_node->value();
                            if (0 == _wcsicmp(wsTimeUnit.c_str(), L"days")) {
                                _heartbeat_frequency *= 86400;
                            }
                            else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"hours")) {
                                _heartbeat_frequency *= 3600;
                            }
                            else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"minutes")) {
                                _heartbeat_frequency *= 60;
                            }
                            else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"milliseconds")) {
                                _heartbeat_frequency /= 1000;
                                if (0 == _heartbeat_frequency) {
                                    _heartbeat_frequency = 1;
                                }
                            }
                            else {
                                ; // Nothing
                            }
                        }
                    }
                }
            }
        }
        //  -> Log Frequency
        rapidxml::xml_node<wchar_t>* commprofile_log_freq_node = commprofile_node->first_node(L"logFrequency", 0, false);
        if (NULL != commprofile_log_freq_node) {
            _log_frequency = 300; // 5 minutes
            rapidxml::xml_node<wchar_t>* commprofile_log_freqtime_node = commprofile_log_freq_node->first_node(L"time", 0, false);
            if (NULL != commprofile_log_freqtime_node) {
                std::wstring wsFrequency = commprofile_log_freqtime_node->value();
                if (!wsFrequency.empty()) {
                    int nFrequency = 0;
                    if (nudf::string::ToInt<wchar_t>(wsFrequency, &nFrequency) && nFrequency > 0) {
                        _log_frequency = nFrequency;
                        rapidxml::xml_node<wchar_t>* commprofile_log_frequnit_node = commprofile_log_freq_node->first_node(L"time-Unit", 0, false);
                        if (NULL != commprofile_log_frequnit_node) {
                            std::wstring wsTimeUnit = commprofile_log_frequnit_node->value();
                            if (0 == _wcsicmp(wsTimeUnit.c_str(), L"days")) {
                                _log_frequency *= 86400;
                            }
                            else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"hours")) {
                                _log_frequency *= 3600;
                            }
                            else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"minutes")) {
                                _log_frequency *= 60;
                            }
                            else if (0 == _wcsicmp(wsTimeUnit.c_str(), L"milliseconds")) {
                                _log_frequency /= 1000;
                                if (0 == _log_frequency) {
                                    _log_frequency = 1;
                                }
                            }
                            else {
                                ; // Nothing
                            }
                        }
                    }
                }
            }
        }
    }
    catch (const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
        throw std::exception("invalid agent profile xml");
    }
}

//
//
//

external_user_info::external_user_info() : _dirty(false)
{
}

external_user_info::~external_user_info()
{
    if (_dirty) {
        save();
    }
}

void external_user_info::load()
{
    if (GLOBAL.agent_info().empty()) {
        // agent not registered yet
        return;
    }
    std::wstring file = GLOBAL.dir_conf() + L"\\external_users.sjs";

    try {
        std::string s = NX::sconfig::load(file, GLOBAL.agent_key().decrypt());
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);

        NX::exclusive_locker locker(this->_lock);
        _map.clear();
        if (v.is_array()) {
            // good
            std::for_each(v.as_array().begin(), v.as_array().end(), [&](const NX::web::json::value& user) {
                if (!user.is_object()) {
                    return;
                }
                try {
                    std::wstring user_id = user.at(L"id").as_string();
                    std::wstring user_name = user.at(L"name").as_string();
                    std::wstring user_domain = user.at(L"domain").as_string();
                    std::transform(user_id.begin(), user_id.end(), user_id.begin(), toupper);
                    std::transform(user_name.begin(), user_name.end(), user_name.begin(), tolower);
                    std::transform(user_domain.begin(), user_domain.end(), user_domain.begin(), tolower);
                    _map[user_id] = std::pair<std::wstring, std::wstring>(user_name, user_domain);
                }
                catch (std::exception& e) {
                    UNREFERENCED_PARAMETER(e);
                }
            });
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGERR(1, L"fail to load external user info");
    }
}

void external_user_info::save()
{
    if (GLOBAL.agent_info().empty()) {
        // agent not registered yet
        return;
    }
    std::wstring file = GLOBAL.dir_conf() + L"\\external_users.sjs";

    try {
        NX::shared_locker locker(this->_lock);
        std::vector<NX::web::json::value>   users_array;
        std::for_each(_map.begin(), _map.end(), [&](const std::pair<std::wstring, std::pair<std::wstring, std::wstring>>& item) {
            NX::web::json::value user = NX::web::json::value::object();
            user[L"id"] = NX::web::json::value::string(item.first);
            user[L"name"] = NX::web::json::value::string(item.second.first);
            user[L"domain"] = NX::web::json::value::string(item.second.second);
            users_array.push_back(user);
        });

        NX::web::json::value v = NX::web::json::value::array(users_array);
        std::wstring ws = v.serialize();
        std::string s = NX::utility::conversions::utf16_to_utf8(ws);
        NX::sconfig::save(file, s, GLOBAL.agent_key().decrypt());
        _dirty = false;
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        LOGERR(1, L"fail to save external user info");
    }
}

void external_user_info::insert(const std::wstring& user_id, const std::wstring& user_name, const std::wstring& user_domain)
{
    std::wstring id(user_id);
    std::wstring name(user_name);
    std::wstring domain(user_domain);
    std::transform(id.begin(), id.end(), id.begin(), toupper);
    std::transform(name.begin(), name.end(), name.begin(), tolower);
    std::transform(domain.begin(), domain.end(), domain.begin(), tolower);
    NX::exclusive_locker locker(this->_lock);
    _map[id] = std::pair<std::wstring, std::wstring>(name, domain);
    _dirty = true;
}

std::pair<std::wstring, std::wstring> external_user_info::find_user(const std::wstring& user_id) const noexcept
{
    NX::shared_locker locker(this->_lock);
    auto it = std::find_if(_map.begin(), _map.end(), [&](const std::pair<std::wstring, std::pair<std::wstring, std::wstring>>& item) -> bool {
        return (0 == _wcsicmp(user_id.c_str(), item.first.c_str()));
    });
    if (it != _map.end()) {
        return (*it).second;
    }
    return std::pair<std::wstring, std::wstring>(L"", L"");
}

bool external_user_info::user_exists(const std::wstring& user_id) const noexcept
{
    NX::shared_locker locker(this->_lock);
    auto it = std::find_if(_map.begin(), _map.end(), [&](const std::pair<std::wstring, std::pair<std::wstring, std::wstring>>& item) -> bool {
        return (0 == _wcsicmp(user_id.c_str(), item.first.c_str()));
    });
    return (it != _map.end());
}

std::vector<std::wstring> external_user_info::get_id_list() const noexcept
{
    std::vector<std::wstring> list;
    std::for_each(_map.begin(), _map.end(), [&](const std::pair<std::wstring, std::pair<std::wstring, std::wstring>>& item) {
        list.push_back(item.first);
    });
    return std::move(list);
}


//
//
//

whitelist_info::app_info::app_info(const std::wstring& name, const std::wstring& publisher)
{
    try {
        std::wstring ws;

        ws = NX::EVAL::value_object::nextlabs_wildcards_to_regex_copy(name);
        if (ws.empty()) {
            ws = L".*";
        }
        _name = std::wregex(ws, std::regex_constants::icase);

        ws = NX::EVAL::value_object::nextlabs_wildcards_to_regex_copy(publisher);
        if (ws.empty()) {
            ws = L".*";
        }
        _publisher = std::wregex(ws, std::regex_constants::icase);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        _name = std::wregex(L".*", std::regex_constants::icase);
        _publisher = std::wregex(L".*", std::regex_constants::icase);
    }
}

bool whitelist_info::app_info::check(const std::wstring& image, const std::wstring& publisher) const noexcept
{
    bool result = false;
    try {
        result = (std::regex_match(image, _name) && std::regex_match(publisher, _publisher));
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        result = false;
    }
    return result;
}

whitelist_info::whitelist_info()
{
}

whitelist_info::whitelist_info(const NX::web::json::value& agent_whitelist)
{
    set(agent_whitelist);
}

whitelist_info::~whitelist_info()
{
}

void whitelist_info::set(const NX::web::json::value& v) noexcept
{
    NX::exclusive_locker locker(this->_lock);
    try {

        _hash = global::json_hash(v);

        if (v.has_field(L"extensions")) {
            _ext_list = v.at(L"extensions").as_string();
        }
        if (v.has_field(L"applications")) {
            const NX::web::json::array& apps = v.at(L"applications").as_array();
            for (auto it = apps.begin(); it != apps.end(); ++it) {
                std::wstring name;
                std::wstring publisher;
                std::vector<std::wstring> vrights;
                if ((*it).has_field(L"name")) {
                    name = (*it).at(L"name").as_string();
                }
                if ((*it).has_field(L"publisher")) {
                    publisher = (*it).at(L"publisher").as_string();
                }
                if ((*it).has_field(L"rights")) {
                    const NX::web::json::array& rv = (*it).at(L"rights").as_array();
                    for (auto rit = rv.begin(); rit != rv.end(); ++rit) {
                        vrights.push_back((*rit).as_string());
                    }
                }
                if (vrights.empty()) {
                    vrights.push_back(L"RIGHT_VIEW");
                }
                _app_list.push_back(std::pair<app_info, NX::EVAL::rights>(app_info(name, publisher), NX::EVAL::rights(vrights)));
            }
        }
    }
    catch (std::exception& e) {
        clear();
        UNREFERENCED_PARAMETER(e);
    }
}

whitelist_info& whitelist_info::operator = (const whitelist_info& other) noexcept
{
    if (this != &other) {
        _ext_list = other.ext_list();
        _app_list = other.app_list();
        _hash = other.hash();
    }
    return *this;
}

whitelist_info& whitelist_info::operator = (const NX::web::json::value& v) noexcept
{
    set(v);
    return *this;
}

void whitelist_info::clear() noexcept
{
    _ext_list.clear();
    _app_list.clear();
    _hash.clear();
}

NX::EVAL::rights whitelist_info::check_app(const std::wstring& image, const std::wstring& publisher) const noexcept
{
    NX::EVAL::rights r;
    NX::shared_locker locker(this->_lock);
    auto pos = std::find_if(_app_list.begin(), _app_list.end(), [&](const std::pair<app_info, NX::EVAL::rights>& v)-> bool {
        return v.first.check(image, publisher);
    });
    if (pos != _app_list.end()) {
        r = (*pos).second;
    }
    return r;
}
