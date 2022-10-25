

#pragma once
#ifndef __NUDF_URI_HPP__
#define __NUDF_URI_HPP__

#include <string>



namespace NX {


//
//                      hierarchical part
//          ©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´
//                      authority               path
//          ©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´©°©¤©¤©¤©Ø©¤©¤©¤©¤©´
//    abc://username:password@example.com:123/path/data?key=value#fragid1
//    ©¸©Ð©¼   ©¸©¤©¤©¤©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¤©¤©¤©¼ ©¸©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¼ ©¸©Ð©¼           ©¸©¤©¤©¤©Ð©¤©¤©¤©¼ ©¸©¤©¤©Ð©¤©¤©¼
//  scheme  user information     host     port            query   fragment
//
class uri
{
public:
    uri();
    uri(const std::wstring& s);
    uri(const std::wstring& scheme,
        const std::wstring& user,
        const std::wstring& password,
        const std::wstring& host,
        unsigned short port,
        const std::wstring& path,
        const std::wstring& query,
        const std::wstring& fragment);
    virtual ~uri();

    inline bool empty() const noexcept { return _scheme.empty(); }
    inline const std::wstring& scheme() const noexcept { return _scheme; }
    inline const std::wstring& user() const noexcept { return _user; }
    inline const std::wstring& password() const noexcept { return _password; }
    inline const std::wstring& host() const noexcept { return _host; }
    inline unsigned short port() const noexcept { return _port; }
    inline const std::wstring& path() const noexcept { return _path; }
    inline const std::wstring& query() const noexcept { return _query; }
    inline const std::wstring& fragment() const noexcept { return _fragment; }
    inline bool is_port_default() const noexcept { return (0 == _port); }

    void clear() noexcept;
    uri& operator = (const uri& other) noexcept;
    std::wstring serialize();
    void parse(const std::wstring& s);

private:
    std::wstring    _scheme;
    std::wstring    _user;
    std::wstring    _password;
    std::wstring    _host;
    unsigned short  _port;
    std::wstring    _path;
    std::wstring    _query;
    std::wstring    _fragment;
};



}



#endif