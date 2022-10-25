

#include <Winsock2.h>
#include <Windows.h>
#include <http.h>


#include <algorithm>


#include <nudf\string.hpp>
#include <nudf\conversion.hpp>
#include <nudf\http_service.hpp>


using namespace NX;
using namespace NX::http;




ssl_conf::ssl_conf() :
    _port(0),
    _cert_store_name(L"MY"),
    _cert_check_mode(0),
    _flags(HTTP_SERVICE_CONFIG_SSL_FLAG_NEGOTIATE_CLIENT_CERT)
{
}

ssl_conf::ssl_conf(const ssl_conf& other)
{
    *this = other;
}

ssl_conf::ssl_conf(unsigned short ssl_port,
    const GUID& ssl_app_id,
    std::vector<unsigned char>& ssl_cert_hash,
    const wchar_t* ssl_cert_store_name,
    unsigned long ssl_cert_check_mode,
    unsigned long ssl_flags) : _port(ssl_port),
    _app_id(ssl_app_id),
    _cert_hash(ssl_cert_hash),
    _cert_store_name(ssl_cert_store_name ? ssl_cert_store_name : L"MY"),
    _cert_check_mode(ssl_cert_check_mode),
    _flags(ssl_flags)
{
}

ssl_conf::ssl_conf(unsigned short ssl_port,
    const std::wstring& ssl_app_id,
    std::wstring& ssl_cert_hash,
    const wchar_t* ssl_cert_store_name,
    unsigned long ssl_cert_check_mode,
    unsigned long ssl_flags) : _port(ssl_port),
    _cert_store_name(ssl_cert_store_name ? ssl_cert_store_name : L"MY"),
    _cert_check_mode(ssl_cert_check_mode),
    _flags(ssl_flags)
{
}

ssl_conf::~ssl_conf()
{
}

ssl_conf& ssl_conf::operator = (const ssl_conf& other)
{
    if (this != &other) {
        _port = other.port();
        _app_id = other.app_id();
        _cert_hash = other.cert_hash();
        _cert_store_name = other.cert_store_name();
        _cert_check_mode = other.cert_check_mode();
        _flags = other.flags();
    }
    return *this;
}

ssl_conf& ssl_conf::operator = (ssl_conf&& other)
{
    if (this != &other) {
        _port = other.port();
        _app_id = std::move(other.app_id());
        _cert_hash = std::move(other.cert_hash());
        _cert_store_name = std::move(other.cert_store_name());
        _cert_check_mode = other.cert_check_mode();
        _flags = other.flags();
    }
    return *this;
}



ssl_conf NX::http::query_ssl_conf(unsigned short port)
{
    std::vector<unsigned char> buf;
    unsigned long result = 0;
    HTTP_SERVICE_CONFIG_SSL_QUERY query_data;
    sockaddr_in sock_addr;
    unsigned long bytes_required = 0;

    memset(&query_data, 0, sizeof(HTTP_SERVICE_CONFIG_SSL_QUERY));
    memset(&sock_addr, 0, sizeof(SOCKADDR));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    if (0 != InetPtonW(AF_INET, L"0.0.0.0", &sock_addr.sin_addr)) {
        throw std::exception("error");
    }

    query_data.QueryDesc = HttpServiceConfigQueryExact;
    query_data.KeyDesc.pIpPort = (PSOCKADDR)(&sock_addr);

    result = HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query_data, (ULONG)sizeof(HTTP_SERVICE_CONFIG_SSL_QUERY), NULL, 0, &bytes_required, NULL);
    if (ERROR_INSUFFICIENT_BUFFER != result) {
        throw std::exception("error");
    }

    buf.resize(bytes_required, 0);
    result = HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query_data, (ULONG)sizeof(HTTP_SERVICE_CONFIG_SSL_QUERY), buf.data(), (ULONG)buf.size(), &bytes_required, NULL);
    if (NO_ERROR != result) {
        throw std::exception("error");
    }

    const HTTP_SERVICE_CONFIG_SSL_SET* conf = reinterpret_cast<const HTTP_SERVICE_CONFIG_SSL_SET*>(buf.data());
    unsigned short ssl_port = ntohs(((sockaddr_in*)conf->KeyDesc.pIpPort)->sin_port);
    std::vector<unsigned char> ssl_cert_hash((unsigned char*)conf->ParamDesc.pSslHash, (unsigned char*)conf->ParamDesc.pSslHash + conf->ParamDesc.SslHashLength);
    std::wstring ssl_cert_store_name(conf->ParamDesc.pSslCertStoreName ? conf->ParamDesc.pSslCertStoreName : L"MY");

    return ssl_conf(ssl_port, conf->ParamDesc.AppId, ssl_cert_hash, conf->ParamDesc.pSslCertStoreName, conf->ParamDesc.DefaultCertCheckMode, conf->ParamDesc.DefaultFlags);
}


std::vector<ssl_conf> NX::http::query_ssl_conf_all()
{
    std::vector<ssl_conf> confs;
    unsigned long result = 0;
    HTTP_SERVICE_CONFIG_SSL_QUERY query_data;

    memset(&query_data, 0, sizeof(query_data));
    query_data.QueryDesc = HttpServiceConfigQueryNext;

    do {

        std::vector<unsigned char> buf;
        unsigned long bytes_required = 0;

        result = HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query_data, (ULONG)sizeof(HTTP_SERVICE_CONFIG_SSL_QUERY), NULL, 0, &bytes_required, NULL);
        if (ERROR_INSUFFICIENT_BUFFER != result) {
            throw std::exception("error");
        }
        query_data.dwToken++;

        buf.resize(bytes_required, 0);
        result = HttpQueryServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &query_data, (ULONG)sizeof(HTTP_SERVICE_CONFIG_SSL_QUERY), buf.data(), (ULONG)buf.size(), &bytes_required, NULL);
        if (NO_ERROR == result) {

            const HTTP_SERVICE_CONFIG_SSL_SET* conf = (const HTTP_SERVICE_CONFIG_SSL_SET*)buf.data();
            confs.push_back(ssl_conf(ntohs(((sockaddr_in*)conf->KeyDesc.pIpPort)->sin_port),
                conf->ParamDesc.AppId,
                std::vector<unsigned char>((unsigned char*)conf->ParamDesc.pSslHash, (unsigned char*)conf->ParamDesc.pSslHash + conf->ParamDesc.SslHashLength),
                conf->ParamDesc.pSslCertStoreName,
                conf->ParamDesc.DefaultCertCheckMode,
                conf->ParamDesc.DefaultFlags));
        }

    } while (result != ERROR_NO_MORE_ITEMS);

    return std::move(confs);
}

std::vector<ssl_conf> NX::http::query_ssl_conf_by_hash(const std::wstring& hash)
{
    const std::vector<unsigned char>& buf = NX::utility::hex_string_to_buffer<wchar_t>(hash);
    return query_ssl_conf_by_hash(buf);
}

std::vector<ssl_conf> NX::http::query_ssl_conf_by_hash(const std::vector<unsigned char>& hash)
{
    std::vector<ssl_conf> output_confs;

    const std::vector<ssl_conf>& confs = query_ssl_conf_all();
    std::for_each(confs.begin(), confs.end(), [&](const ssl_conf& conf) {
        if (hash == conf.cert_hash()) {
            output_confs.push_back(conf);
        }
    });
    return std::move(output_confs);
}

std::vector<ssl_conf> NX::http::query_ssl_conf_by_app_id(const GUID& id)
{
    std::vector<ssl_conf> output_confs;

    const std::vector<ssl_conf>& confs = query_ssl_conf_all();
    std::for_each(confs.begin(), confs.end(), [&](const ssl_conf& conf) {
        if (id == conf.app_id()) {
            output_confs.push_back(conf);
        }
    });
    return std::move(output_confs);
}

void NX::http::set_ssl_conf(const ssl_conf& conf)
{
    std::vector<unsigned char> buf;
    unsigned long result = 0;
    HTTP_SERVICE_CONFIG_SSL_SET ssl_set;
    sockaddr_in sock_addr;

    memset(&ssl_set, 0, sizeof(HTTP_SERVICE_CONFIG_SSL_SET));
    memset(&sock_addr, 0, sizeof(sockaddr_in));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(conf.port());
    if (0 != InetPtonW(AF_INET, L"0.0.0.0", &sock_addr.sin_addr)) {
        throw std::exception("error");
    }

    ssl_set.KeyDesc.pIpPort = (PSOCKADDR)(&sock_addr);
    ssl_set.ParamDesc.SslHashLength = (ULONG)conf.cert_hash().size();
    ssl_set.ParamDesc.pSslHash = (PVOID)conf.cert_hash().data();
    ssl_set.ParamDesc.AppId = conf.app_id();
    ssl_set.ParamDesc.pSslCertStoreName = (PWSTR)conf.cert_store_name().c_str();
    ssl_set.ParamDesc.DefaultCertCheckMode = conf.cert_check_mode();
    ssl_set.ParamDesc.DefaultRevocationFreshnessTime = 0;
    ssl_set.ParamDesc.DefaultRevocationUrlRetrievalTimeout = 0;
    ssl_set.ParamDesc.pDefaultSslCtlIdentifier = NULL;
    ssl_set.ParamDesc.pDefaultSslCtlStoreName = NULL;
    ssl_set.ParamDesc.DefaultFlags = conf.flags();


    result = HttpSetServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, &ssl_set, (ULONG )sizeof(HTTP_SERVICE_CONFIG_SSL_SET), NULL);
    if (NO_ERROR != result) {
        throw std::exception("error");
    }
}

void NX::http::remove_ssl_conf(unsigned short port)
{
    std::vector<unsigned char> buf;
    unsigned long result = 0;
    HTTP_SERVICE_CONFIG_SSL_SET ssl_set;
    sockaddr_in sock_addr;

    memset(&ssl_set, 0, sizeof(HTTP_SERVICE_CONFIG_SSL_SET));
    memset(&sock_addr, 0, sizeof(sockaddr_in));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    if (0 != InetPtonW(AF_INET, L"0.0.0.0", &sock_addr.sin_addr)) {
        throw std::exception("error");
    }

    ssl_set.KeyDesc.pIpPort = (PSOCKADDR)(&sock_addr);
    result = HttpDeleteServiceConfiguration(NULL, HttpServiceConfigSSLCertInfo, buf.data(), (ULONG)buf.size(), NULL);
    if (NO_ERROR != result) {
        throw std::exception("error");
    }
}