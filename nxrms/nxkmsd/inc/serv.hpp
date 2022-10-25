
#pragma once
#ifndef __NXRMS_SERV_HPP__
#define __NXRMS_SERV_HPP__


#include <nudf\service.hpp>


class kms_serv : public NX::win::service_instance
{
public:
    kms_serv();
    virtual ~kms_serv();

    virtual void on_start(int argc, const wchar_t** argv);
    virtual void on_stop() noexcept;
    virtual void on_pause();
    virtual void on_resume();
    virtual void on_preshutdown() noexcept;
    virtual void on_shutdown() noexcept;

protected:
    rms_httpd   _httpd;
};

class kms_serv_conf
{
public:
    kms_serv_conf();
    kms_serv_conf(const std::wstring& key_path);
    kms_serv_conf(bool use_https, unsigned short server_port, unsigned short control_port, int log_level, bool use_remote_control = false, unsigned long delay_time = 0);
    ~kms_serv_conf();

    inline unsigned short get_server_port() const { return _server_port; }
    inline bool is_https_enabled() const { return _https_enabled; }
    inline unsigned short get_control_port() const { return _control_port; }
    inline bool is_remote_control_enabld() const { return _remote_control; }
    inline unsigned long get_delay_seconds() const { return _delay_seconds; }
    inline int get_log_level() const { return _log_level; }

    inline void set_server_port(unsigned short server_port) { _server_port = server_port; }
    inline void enable_https(bool enabled) { _https_enabled = enabled; }
    inline void set_control_port(unsigned short control_port) { _control_port = control_port; }
    inline void enable_remote_control(bool enabled) { _remote_control = enabled; }
    inline void set_delay_seconds(unsigned long seconds) { _delay_seconds = seconds; }
    inline void set_log_level(int level) { _log_level = level; }

    kms_serv_conf& operator = (const kms_serv_conf& other);

    std::wstring build_site_url() const;
    std::wstring build_crtl_url() const;

    void init(const std::wstring& key_path = std::wstring()) noexcept;
    void apply(const std::wstring& key_path = std::wstring());

private:
    unsigned short  _server_port;
    bool            _https_enabled;
    unsigned short  _control_port;
    bool            _remote_control;
    unsigned long   _delay_seconds;
    int             _log_level;
};


#endif