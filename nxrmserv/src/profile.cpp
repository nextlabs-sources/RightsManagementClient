

#include <Windows.h>


#include <nudf\web\json.hpp>
#include <nudf\time.hpp>
#include <nudf\string.hpp>

#include "nxrmserv.h"
#include "sconfig.hpp"
#include "profile.hpp"


using namespace NX;


//
//  class authn_token
//
authn_token::authn_token(const std::wstring& s) : _expire_time(0)
{
    load(s);
}

authn_token::authn_token(const std::wstring& user_name, const std::wstring& principle_name, const std::wstring& user_id, const std::wstring& user_domain, unsigned long time_to_live /*minutes*/) :
    _user_name(user_name), _user_principle_name(principle_name), _user_id(user_id), _user_domain(user_domain), _expire_time(0)
{
    nudf::time::CTime ct;
    ct.Now();
    ct.AddMinutes(time_to_live);
    _expire_time = ct.ToUll();
}

authn_token::authn_token(const std::wstring& user_name, const std::wstring& principle_name, const std::wstring& user_id, const std::wstring& user_domain, const FILETIME& ft) :
    _user_name(user_name), _user_principle_name(principle_name), _user_id(user_id), _user_domain(user_domain), _expire_time(0)
{
    if (ft.dwHighDateTime != 0 || ft.dwLowDateTime != 0) {
        nudf::time::CTime ct(&ft);
        _expire_time = ct.ToUll();
    }
}

bool authn_token::expired() const noexcept
{
    if (never_expire()) {
        return false;
    }
    nudf::time::CTime ct;
    ct.Now();
    return (ct.ToUll() > expire_time());
}

void authn_token::set_expire() noexcept
{
    nudf::time::CTime ct;
    ct.Now();
    ct.SubMinutes(1);
    _expire_time = ct.ToUll();
}

void authn_token::clear() noexcept
{
    _user_name.clear();
    _user_principle_name.clear();
    _user_id.clear();
    _user_domain.clear();
    _expire_time = 0;
}

bool authn_token::empty() const noexcept
{
    return (_user_name.empty() || _user_id.empty() || _user_domain.empty());
}

std::wstring authn_token::serialize() const noexcept
{
    NX::web::json::value jtk = NX::web::json::value::object();
    std::wstring ws_expire_date;
    if (!never_expire()) {
        FILETIME ft = { 0, 0 };
        FILETIME lft = { 0, 0 };
        ft.dwHighDateTime = (unsigned long)(_expire_time >> 32);
        ft.dwLowDateTime = (unsigned long)_expire_time;
        FileTimeToLocalFileTime(&ft, &lft);
        ws_expire_date = nudf::string::FromSystemTime<wchar_t>(&lft, false);
    }
    jtk[L"authn_name"] = NX::web::json::value::string(user_name());
    jtk[L"authn_principle_name"] = NX::web::json::value::string(principle_name());
    jtk[L"authn_id"] = NX::web::json::value::string(user_id());
    jtk[L"authn_domain"] = NX::web::json::value::string(user_domain());
    jtk[L"expire_time"] = NX::web::json::value::string(ws_expire_date);
    return jtk.serialize();
}

void authn_token::load(const std::wstring& ws)
{
    try {

        NX::web::json::value jtk = NX::web::json::value::parse(ws);
        _user_name = jtk[L"authn_name"].as_string();
        _user_principle_name = jtk[L"authn_principle_name"].as_string();
        _user_id = jtk[L"authn_id"].as_string();
        _user_domain = jtk[L"authn_domain"].as_string();
        std::wstring local_expire_time = jtk[L"expire_time"].as_string();
        _expire_time = 0;
        if (!local_expire_time.empty()) {
            FILETIME lft = { 0, 0 };
            FILETIME ft = { 0, 0 };
            bool utc = true;
            if (nudf::string::ToSystemTime<wchar_t>(local_expire_time, &lft, &utc)) {
                if (!utc) {
                    LocalFileTimeToFileTime(&lft, &ft);
                }
                else {
                    ft.dwHighDateTime = lft.dwHighDateTime;
                    ft.dwLowDateTime = lft.dwLowDateTime;
                }
                _expire_time = ft.dwHighDateTime;
                _expire_time <<= 32;
                _expire_time += ft.dwLowDateTime;
            }
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
    }
}



//
//  class profile
//
profile::profile() : _authn_type(LocalAuthn), _dirty(false)
{
}

profile::profile(const std::wstring& id) : _id(id), _authn_type(LocalAuthn), _dirty(false)
{
}

profile::~profile()
{
}

bool profile::exists(const std::wstring& id) noexcept
{
    std::wstring dir = GLOBAL.dir_profiles() + L"\\" + id;
    return (INVALID_FILE_ATTRIBUTES != GetFileAttributesW(dir.c_str()));
}

void profile::load(const std::wstring& id)
{

    try {

        _id = id;
        std::transform(_id.begin(), _id.end(), _id.begin(), toupper);

        std::wstring dir_profile = GLOBAL.dir_profiles() + L"\\" + _id;
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(dir_profile.c_str())) {
            throw std::exception("profile folder does not exist");
        }
        std::wstring profile_file = dir_profile + L"\\profile.sjs";
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(profile_file.c_str())) {
            throw std::exception("profile does not exist");
        }

        std::string s = ::sconfig::load(profile_file, GLOBAL.agent_key().decrypt());
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);

        NX::web::json::value v = NX::web::json::value::parse(ws);
        if (v.is_null()) {
            throw std::exception("profile is empty");
        }
        if (!v.is_object()) {
            throw std::exception("profile is not in correct format");
        }

        if (!v[L"id"].is_string()) {
            throw std::exception("invalid profile user id");
        }
        _id = v[L"id"].as_string();

        if (!v[L"name"].is_string()) {
            throw std::exception("invalid profile user name");
        }
        _name = v[L"name"].as_string();

        if (v[L"domain"].is_string()) {
            _domain = v[L"domain"].as_string();
        }


        if (v[L"authn-type"].is_string()) {
            std::wstring authntype = v[L"authn-type"].as_string();
            if (0 == _wcsicmp(authntype.c_str(), L"External")) {
                _authn_type = ExternalAuthn;
            }
            else if (0 == _wcsicmp(authntype.c_str(), L"Local")) {
                _authn_type = LocalAuthn;
            }
            else {
                _authn_type = LocalAuthn;
                LOGWARN(ERROR_INVALID_DATA, L"unknown authn type (%s) for user %s (%s) --> treat as local authn", authntype.c_str(), _name.c_str(), _id.c_str());
            }
        }
        else {
            _authn_type = LocalAuthn;
        }

        if (_authn_type == ExternalAuthn) {
            if (v[L"authn-token"].is_string()) {
                _authn_token = authn_token(v[L"authn-token"].as_string());
            }
            if (v[L"authn-attributes"].is_object()) {
                const NX::web::json::object& authn_attributes = v[L"authn-attributes"].as_object();
                std::for_each(authn_attributes.begin(), authn_attributes.end(), [&](const std::pair<std::wstring, NX::web::json::value>& v) {
                    if (v.second.is_string()) {
                        _authn_attributes[v.first] = v.second.as_string();
                    }
                });
            }
        }

        if (v[L"attributes"].is_object()) {
            const NX::web::json::object& attributes = v[L"attributes"].as_object();
            std::for_each(attributes.begin(), attributes.end(), [&](const std::pair<std::wstring, NX::web::json::value>& v) {
                if (v.second.is_string()) {
                    _attributes[v.first] = v.second.as_string();
                }
            });
        }
    }
    catch (std::exception& e) {
        clear();
        throw e;
    }
}

std::wstring profile::serialize() noexcept
{
    if (empty()) {
        return std::wstring();
    }

    // create profile root
    NX::web::json::value v = NX::web::json::value::object();
    // basic information
    v[L"id"] = NX::web::json::value::string(_id);
    v[L"name"] = NX::web::json::value::string(_name);
    v[L"domain"] = NX::web::json::value::string(_domain);
    v[L"authn-type"] = NX::web::json::value::string(authn_type_string());
    v[L"authn-token"] = NX::web::json::value::string(_authn_token.serialize());
    // user attributes
    NX::web::json::value user_attrs = NX::web::json::value::object();
    std::for_each(_attributes.begin(), _attributes.end(), [&](const std::pair<std::wstring, std::wstring>& v) {
        user_attrs[v.first] = NX::web::json::value::string(v.second);
    });
    v[L"attributes"] = user_attrs;
    // authn user attributes
    NX::web::json::value authn_user_attrs = NX::web::json::value::object();
    std::for_each(_authn_attributes.begin(), _authn_attributes.end(), [&](const std::pair<std::wstring, std::wstring>& v) {
        authn_user_attrs[v.first] = NX::web::json::value::string(v.second);
    });
    v[L"authn-attributes"] = authn_user_attrs;

    return std::move(v.serialize());
}

bool profile::save() noexcept
{
    bool result = false;
    if (empty()) {
        return false;
    }

    try {

        std::wstring dir = GLOBAL.dir_profiles() + L"\\" + _id;
        std::wstring file = dir + L"\\profile.sjs";

        if (!NX::global::create_profile_folder(_id, false, false)) {
            throw std::exception("profile folder doesn't exist");
        }

        std::wstring ws = serialize();
        std::string s = NX::utility::conversions::utf16_to_utf8(ws);
        NX::sconfig::save(file, s, GLOBAL.agent_key().decrypt());
        // remove dirty flag
        _dirty = false;
        result = true;
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
    }

    return result;
}

bool profile::has_attribute(const std::wstring& name) const noexcept
{
    auto pos = std::find_if(_attributes.begin(), _attributes.end(), [&](const std::pair<std::wstring,std::wstring>& attr)->bool {
        return (0 == _wcsicmp(attr.first.c_str(), name.c_str()));
    });
    return (pos != _attributes.end());
}

std::wstring profile::get_attribute(const std::wstring& name) const noexcept
{
    auto pos = std::find_if(_attributes.begin(), _attributes.end(), [&](const std::pair<std::wstring,std::wstring>& attr)->bool {
        return (0 == _wcsicmp(attr.first.c_str(), name.c_str()));
    });
    
    if (pos != _attributes.end()) {
        return (*pos).second;
    }

    return std::wstring();
}