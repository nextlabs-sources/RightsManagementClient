
#pragma once
#ifndef __NXRM_SVC_HPP__
#define __NXRM_SVC_HPP__


#include <nudf\service.hpp>

#include "session.hpp"




extern BOOL __stdcall check_log_accept(unsigned long level);
extern LONG __stdcall write_log(const wchar_t* msg);

class rmserv : public NX::win::service_instance
{
public:
    rmserv();
    virtual ~rmserv();

    virtual void on_start(int argc, const wchar_t** argv);
    virtual void on_stop() noexcept;
    virtual void on_pause();
    virtual void on_resume();
    virtual void on_preshutdown() noexcept;
    virtual void on_shutdown() noexcept;
    virtual long on_session_logon(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept;
    virtual long on_session_logoff(_In_ WTSSESSION_NOTIFICATION* wtsn) noexcept;

protected:
    void init_log();
    void log_init_info();

private:
    winsession_manager  _win_session_manager;
};

class rmserv_conf
{
public:
    rmserv_conf();
    rmserv_conf(const std::wstring& key_path);
    rmserv_conf(unsigned long log_level, unsigned long log_size, unsigned long delay_time = 0);
    ~rmserv_conf();

    inline unsigned long get_delay_seconds() const { return _delay_seconds; }
    inline unsigned long get_log_level() const { return _log_level; }
    inline unsigned long get_log_size() const { return _log_size; }

    inline void set_delay_seconds(unsigned long seconds) { _delay_seconds = seconds; }
    inline void set_log_level(unsigned long level) { _log_level = level; }
    inline void set_log_size(unsigned long size) { _log_size = size; }

    rmserv_conf& operator = (const rmserv_conf& other);
    
    void load(const std::wstring& key_path = std::wstring()) noexcept;
    void apply(const std::wstring& key_path = std::wstring());

private:
    unsigned long   _delay_seconds;
    unsigned long   _log_level;
    unsigned long   _log_size;
};


#endif