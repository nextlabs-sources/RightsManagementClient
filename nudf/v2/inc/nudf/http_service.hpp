

#pragma once
#ifndef __NUDF_HTTP_SERVICE_HPP__
#define __NUDF_HTTP_SERVICE_HPP__

#include <winhttp.h>
#include <http.h>
#include <string>
#include <vector>
#include <map>


namespace NX {

namespace http {


class ssl_conf
{
public:
    ssl_conf();
    ssl_conf(const ssl_conf& other);
    ssl_conf(unsigned short ssl_port, const GUID& ssl_app_id, std::vector<unsigned char>& ssl_cert_hash, const wchar_t* ssl_cert_store_name = NULL, unsigned long ssl_cert_check_mode = 0, unsigned long ssl_flags = HTTP_SERVICE_CONFIG_SSL_FLAG_NEGOTIATE_CLIENT_CERT);
    ssl_conf(unsigned short ssl_port, const std::wstring& ssl_app_id, std::wstring& ssl_cert_hash, const wchar_t* ssl_cert_store_name = NULL, unsigned long ssl_cert_check_mode = 0, unsigned long ssl_flags = HTTP_SERVICE_CONFIG_SSL_FLAG_NEGOTIATE_CLIENT_CERT);
    ~ssl_conf();

    ssl_conf& operator = (const ssl_conf& other);
    ssl_conf& operator = (ssl_conf&& other);

    inline unsigned short port() const { return _port; }
    inline const GUID& app_id() const { return _app_id; }
    inline const std::vector<unsigned char>& cert_hash() const { return _cert_hash; }
    inline const std::wstring& cert_store_name() const { return _cert_store_name; }
    inline unsigned long cert_check_mode() const { return _cert_check_mode; }
    inline unsigned long flags() const { return _flags; }
    inline GUID& app_id() { return _app_id; }
    inline std::vector<unsigned char>& cert_hash() { return _cert_hash; }
    inline std::wstring& cert_store_name() { return _cert_store_name; }

private:
    unsigned short              _port;
    GUID                        _app_id;
    std::vector<unsigned char>  _cert_hash;
    std::wstring                _cert_store_name;
    unsigned long               _cert_check_mode;
    unsigned long               _flags;
};



ssl_conf query_ssl_conf(unsigned short port);
std::vector<ssl_conf> query_ssl_conf_all();
std::vector<ssl_conf> query_ssl_conf_by_hash(const std::wstring& hash);
std::vector<ssl_conf> query_ssl_conf_by_hash(const std::vector<unsigned char>& hash);
std::vector<ssl_conf> query_ssl_conf_by_app_id(const GUID& id);


void set_ssl_conf(const ssl_conf& conf);
void remove_ssl_conf(unsigned short port);


}
}



#endif  // __NUDF_HTTP_SERVICE_HPP__