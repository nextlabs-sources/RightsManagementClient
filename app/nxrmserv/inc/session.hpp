

#pragma once
#ifndef __NXRM_SESSION_HPP__
#define __NXRM_SESSION_HPP__

#include <map>
#include <memory>
#include <thread>
#include <boost/noncopyable.hpp>

#include "profile.hpp"


class rmappinstance : boost::noncopyable
{
public:
    rmappinstance();
    rmappinstance(const std::wstring& app_path);
    ~rmappinstance();

    virtual void run(unsigned long session_id);
    virtual void quit(unsigned long wait_time = 3000);  // by default, wait 3 seconds
    void kill();

    void attach(unsigned long process_id);
    void detach();

    inline bool is_running() const { return (NULL != _process_handle); }
    inline HANDLE get_process_handle() const { return _process_handle; }
    inline unsigned long get_process_id() const { return _process_id; }
    

private:
    std::wstring    _image_path;
    HANDLE          _process_handle;
    unsigned long   _process_id;
};

class rmappmanager
{
public:
    rmappmanager();
    ~rmappmanager();

    void start(unsigned long session_id, const std::wstring& app_path);
    void shutdown();

    inline const std::shared_ptr<rmappinstance>& get_instance() const { return _instance; }
    inline std::shared_ptr<rmappinstance>& get_instance() { return _instance; }
    inline unsigned long get_session_id() const { return _session_id; }
    inline HANDLE get_shutdown_event() const { return _shutdown_event; }

protected:
    static void daemon(rmappmanager* manager);
    
private:
    std::shared_ptr<rmappinstance> _instance;
    HANDLE          _shutdown_event;
    std::thread     _daemon_thread;
    unsigned long   _session_id;
};

class rmsession
{
public:
    rmsession();
    virtual ~rmsession();

    inline bool logged_on() const { return (!_profile.empty() && !_profile.expired()); }
    inline const rmprofile& get_profile() const { return _profile; }
    inline rmappmanager& get_app_manager() { return _appmanager; }

    void initialize(const std::wstring& list_file);
    void clear();


private:
    rmappmanager    _appmanager;
    rmprofile       _profile;
};

class winsession : boost::noncopyable
{
public:
    winsession();
    winsession(unsigned long session_id);
    virtual ~winsession();

private:
    unsigned long   _session_id;
    std::wstring    _user_name;
    std::wstring    _user_sid;
    std::wstring    _user_profile_dir;
    std::wstring    _user_rm_profile_dir;
    rmsession       _rm_session;
};

class winsession_manager : boost::noncopyable
{
public:
    winsession_manager();
    virtual ~winsession_manager();

    void add_session(unsigned long session_id);
    void remove_session(unsigned long session_id);
    std::shared_ptr<winsession> get_session(unsigned long session_id) const;

    void clear();
    bool empty();

    static std::vector<unsigned long> find_existing_session();

private:
    std::map<unsigned long, std::shared_ptr<winsession>>    _map;
    mutable CRITICAL_SECTION _lock;
};



#endif