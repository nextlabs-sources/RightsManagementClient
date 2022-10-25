

#include <windows.h>

#include <nudf\uri.hpp>



//
//  class NX::uri
//

NX::uri::uri() : _port(0)
{
}

NX::uri::uri(const std::wstring& s) : _port(0)
{
    try {
        parse(s);
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        clear();
    }
}

NX::uri::uri(const std::wstring& scheme,
             const std::wstring& user,
             const std::wstring& password,
             const std::wstring& host,
             unsigned short port,
             const std::wstring& path,
             const std::wstring& query,
             const std::wstring& fragment)
             : _scheme(scheme), _user(user), _password(password), _host(host), _port(port), _path(path), _query(query), _fragment(fragment)
{
}

NX::uri::~uri()
{
}

void NX::uri::clear() noexcept
{
    _scheme.clear();
    _user.clear();
    _password.clear();
    _host.clear();
    _port = 0;
    _path.clear();
    _query.clear();
    _fragment.clear();
}

NX::uri& NX::uri::operator = (const NX::uri& other) noexcept
{
    if (this != &other) {
        _scheme = other.scheme();
        _user = other.user();
        _password = other.password();
        _host = other.host();
        _port = other.port();
        _path = other.path();
        _query = other.query();
        _fragment = other.fragment();
    }
    return *this;
}

std::wstring NX::uri::serialize()
{
    if (empty()) {
        return std::wstring();
    }

    std::wstring full_uri(scheme());
    full_uri.append(L"://");
    if (!user().empty()) {
        full_uri.append(user());
        if (!password().empty()) {
            full_uri.append(L":");
            full_uri.append(password());
        }
        full_uri.append(L"@");
    }
    full_uri.append(host());
    if (0 != port()) {
        full_uri.append(L":");
        full_uri.append(std::to_wstring((int)port()));
    }

    if (!path().empty()) {
        full_uri.append(path());
        if (!query().empty()) {
            full_uri.append(L"?");
            full_uri.append(query());
            if (!fragment().empty()) {
                full_uri.append(L"#");
                full_uri.append(fragment());
            }
        }
    }

    return std::move(full_uri);
}

void NX::uri::parse(const std::wstring& s)
{
    std::wstring remain_uri;

    if (s.empty()) {
        return;
    }

    //
    //  scheme
    //
    auto pos = s.find(L"://");
    if (pos == std::wstring::npos) {
        SetLastError(ERROR_INVALID_DATA);
        clear();
        throw std::exception("no scheme");
    }
    _scheme = s.substr(0, pos);
    remain_uri = s.substr(pos + 3);

    //
    //  authority
    //
    std::wstring authority;
    pos = remain_uri.find(L"/");
    if (pos == std::wstring::npos) {
        authority = remain_uri;
        remain_uri.clear();
    }
    else {
        authority = remain_uri.substr(0, pos);
        remain_uri = s.substr(pos);
    }
    pos = authority.find(L"@");
    if (pos != std::wstring::npos) {
        // with user info
        std::wstring user_info;
        std::wstring host_info;
        user_info = authority.substr(0, pos);
        host_info = authority.substr(pos + 1);
        pos = user_info.find(L":");
        if (pos != std::wstring::npos) {
            _user = user_info.substr(0, pos);
            _password = user_info.substr(pos + 1);
        }
        else {
            _user = user_info;
        }
        pos = host_info.find(L":");
        if (pos != std::wstring::npos) {
            _host = host_info.substr(0, pos);
            _port = _wtoi(host_info.substr(pos + 1).c_str());
        }
        else {
            _host = host_info;
        }
    }
    else {
        // without user info
        pos = authority.find(L":");
        if (pos != std::wstring::npos) {
            _host = authority.substr(0, pos);
            _port = _wtoi(authority.substr(pos + 1).c_str());
        }
        else {
            _host = authority;
        }
    }

    //
    //  path, query and fragment
    //
    pos = remain_uri.find(L"?");
    if (pos == std::wstring::npos) {
        _path = remain_uri;
    }
    else {
        _path = remain_uri.substr(0, pos);
        std::wstring query_and_fragment = remain_uri.substr(pos + 1);
        pos = query_and_fragment.find(L"#");
        if (pos == std::wstring::npos) {
            _query = query_and_fragment;
        }
        else {
            _query = query_and_fragment.substr(0, pos);
            _fragment = query_and_fragment.substr(pos + 1);
        }
    }
}