

#include <Windows.h>
#include <assert.h>

#include <iostream>

#include <cpprest\http_listener.h>

#include <nudf\winutil.hpp>
#include <nudf\string.hpp>

#include "nxkmsd.hpp"
#include "httpd.hpp"
#include "serv.hpp"



kms_serv::kms_serv() : NX::win::service_instance(NXRMS_SERVICE_NAME)
{
}

kms_serv::~kms_serv()
{
}

void kms_serv::on_start(int argc, const wchar_t** argv)
{
    std::cout << "on_start" << std::endl;

    kms_serv_conf conf(L"");
    _httpd.start(conf.build_site_url());
}

void kms_serv::on_stop() noexcept
{
    std::cout << "on_stop" << std::endl;
    _httpd.stop();
}

void kms_serv::on_pause()
{
}

void kms_serv::on_resume()
{
}

void kms_serv::on_preshutdown() noexcept
{
}

void kms_serv::on_shutdown() noexcept
{
}


kms_serv_conf::kms_serv_conf() : _server_port(8080), _https_enabled(false), _control_port(8081), _remote_control(false), _delay_seconds(0), _log_level(3)
{
}

kms_serv_conf::kms_serv_conf(const std::wstring& key_path) : _server_port(8080), _https_enabled(false), _control_port(8081), _remote_control(false), _delay_seconds(0), _log_level(3)
{
    init(key_path);
}

kms_serv_conf::kms_serv_conf(bool use_https, unsigned short server_port, unsigned short control_port, int log_level, bool use_remote_control, unsigned long delay_time)
    :_https_enabled(use_https), _server_port(server_port), _control_port(control_port), _remote_control(use_remote_control), _delay_seconds(delay_time), _log_level(log_level)
{
}

kms_serv_conf::~kms_serv_conf()
{
}

kms_serv_conf& kms_serv_conf::operator = (const kms_serv_conf& other)
{
    if (this != &other) {
        _https_enabled = other.is_https_enabled();
        _server_port = other.get_server_port();
        _control_port = other.get_control_port();
        _remote_control = other.is_remote_control_enabld();
        _delay_seconds = other.get_delay_seconds();
        _log_level = other.get_log_level();
    }
    return *this;
}

void kms_serv_conf::init(const std::wstring& key_path) noexcept
{
    try {

        unsigned long ul = 0;
        NX::win::reg_local_machine rgk;

        rgk.open(key_path.empty() ? NXRMS_SERVICE_KEY_PARAMETER : key_path, NX::win::reg_key::reg_wow64_64, true);

        try {
            ul = 0;
            rgk.read_value(KMS_CONF_HTTPS_ENABLED, &ul);
            _https_enabled = (0 == ul) ? false : true;
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }

        if (_https_enabled) {
            _server_port = 8443;
            _control_port = 8444;
        }

        try {
            rgk.read_value(KMS_CONF_PORT, &ul);
            if (0 != ul && ul < 0xFFFF) {
                _server_port = (unsigned short)ul;
            }
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }

        try {
            rgk.read_value(KMS_CONF_CONTROL_PORT, &ul);
            if (0 != ul && ul < 0xFFFF) {
                _control_port = (unsigned short)ul;
            }
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }

        try {
            ul = 0;
            rgk.read_value(KMS_CONF_CONTROL_REMOTE, &ul);
            _remote_control = (0 == ul) ? false : true;
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
        }

        try {
            rgk.read_value(KMS_CONF_DELAY_SECONDS, &_delay_seconds);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            _delay_seconds = 0;
        }

        try {
            rgk.read_value(KMS_CONF_LOG_LEVEL, (unsigned long*)&_log_level);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            _log_level = 3;
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        std::cout << "Fail to read registry: " << e.what() << std::endl;
    }
}

void kms_serv_conf::apply(const std::wstring& key_path)
{
    try {

        unsigned long ul = 0;
        NX::win::reg_local_machine rgk;

        rgk.create(key_path.empty() ? NXRMS_SERVICE_KEY_PARAMETER : key_path, NX::win::reg_key::reg_wow64_64);

        rgk.set_value(KMS_CONF_HTTPS_ENABLED, (ULONG)(is_https_enabled() ? 1 : 0));
        rgk.set_value(KMS_CONF_PORT, (ULONG)get_server_port());
        rgk.set_value(KMS_CONF_CONTROL_PORT, (ULONG)get_control_port());
        rgk.set_value(KMS_CONF_CONTROL_REMOTE, (ULONG)(is_remote_control_enabld() ? 1 : 0));
        rgk.set_value(KMS_CONF_DELAY_SECONDS, (ULONG)get_delay_seconds());
        rgk.set_value(KMS_CONF_LOG_LEVEL, (ULONG)get_log_level());
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        std::cout << "Fail to read registry: " << e.what() << std::endl;
        throw e;
    }
}

std::wstring kms_serv_conf::build_site_url() const
{
    return NX::string_formater(L"%s://*:%d", is_https_enabled() ? L"https" : L"http", get_server_port());
}

std::wstring kms_serv_conf::build_crtl_url() const
{
    return NX::string_formater(L"https://%s:%d", is_remote_control_enabld() ? L"127.0.0.1" : L"*", get_control_port());
}
