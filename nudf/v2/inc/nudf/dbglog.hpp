
#pragma once
#ifndef __NUDF_DEBUG_LOG_HPP__
#define __NUDF_DEBUG_LOG_HPP__

#include <string>
#include <queue>
#include <thread>


namespace NX {
namespace dbg {


typedef enum LOGLEVEL {
    LL_CRITICAL = 0,
    LL_ERROR,
    LL_WARNING,
    LL_INFO,
    LL_INFO_1,
    LL_INFO_2,
    LL_INFO_3,
    LL_INFO_4,
    LL_INFO_5,
    LL_INFO_6,
    LL_INFO_7,
    LL_INFO_8,
    LL_INFO_9,
    LL_DEBUG,
    LL_DETAIL,
    LL_ALL
} LOGLEVEL;

class log_item
{
public:
    log_item();
    log_item(const log_item& other);
    //explicit log_item(log_item&& other);
    explicit log_item(LOGLEVEL level, const std::wstring& msg);
    explicit log_item(LOGLEVEL level, const std::string& msg);
    virtual ~log_item();

    inline bool empty() const { return (0 == _timestamp.wYear); }
    inline SYSTEMTIME get_timestamp() const { return _timestamp; }
    inline const std::wstring& get_message() const { return _message; }
    inline LOGLEVEL get_log_level() const { return _log_level; }
    inline unsigned long get_thread_id() const { return _thread_id; }

    std::wstring serialize() const;
    log_item& operator = (const log_item& other);
    //log_item& operator = (log_item&& other);

private:
    SYSTEMTIME      _timestamp;
    std::wstring    _message;
    LOGLEVEL        _log_level;
    unsigned long   _thread_id;
};

class file_log
{
public:
    file_log();
    virtual ~file_log();

    inline const std::wstring& get_file_path() const { return _file; }
    inline unsigned long get_size_limit() const { return _size_limit; }
    inline unsigned long get_queue_limit() const { return _queue_limit; }
    inline unsigned long get_accepted_level() const { return _accept_level; }
    inline unsigned long get_rotate_count() const { return _rotate_count; }

    inline void set_size_limit(unsigned long size_limit) { _size_limit = size_limit; }
    inline void set_queue_limit(unsigned long queue_limit) { _queue_limit = queue_limit; }
    inline void set_accept_level(unsigned long accept_level) { _accept_level = (accept_level < LL_ALL ? accept_level : LL_ALL); }
    inline void set_rotate_count(unsigned long rotate_count) { _rotate_count = (rotate_count < 10 ? rotate_count : 10); }

    inline bool opened() const { return (_h != INVALID_HANDLE_VALUE); }

    void create(const std::wstring& file, LOGLEVEL accept_level, unsigned long size_limit, unsigned long rotate_count, unsigned long queue_limit);
    void close();

    void push(const log_item& item);
    log_item pop();

    static void writer(file_log* instance);

private:
    void rotate();
    void write(const log_item& item);
    void write_bom();
    void write_line_end();

private:
    HANDLE  _h;
    HANDLE  _events[2];
    std::wstring _file;
    unsigned long _accept_level;
    unsigned long _size_limit;
    unsigned long _queue_limit;
    unsigned long _rotate_count;
    std::queue<log_item> _queue;
    CRITICAL_SECTION _lock;
    std::thread _log_thread;
};

}
}



extern NX::dbg::file_log   NxGlobalLog;

#define DECLARE_GLOBAL_LOG_INSTANCE()     NX::dbg::file_log   NxGlobalLog;

#define GLOBALLOG       NxGlobalLog

#define GLOBAL_LOG_CREATE(FilePath, AcceptLevel, SizeLimit, RotateCount)    NxGlobalLog.create(FilePath, AcceptLevel, SizeLimit, RotateCount, 512)
#define GLOBAL_LOG_SHUTDOWN()   NxGlobalLog.close();
#define GLOBAL_LOG_ACCEPT(ll)   (ll <= NxGlobalLog.get_accepted_level())

#define LOGCRITICAL(s)  NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_CRITICAL, s));
#define LOGERROR(s)     if(NX::dbg::LL_ERROR <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_ERROR, s)); }
#define LOGWARNING(s)   if(NX::dbg::LL_WARNING <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_WARNING, s)); }
#define LOGINFO(s)      if(NX::dbg::LL_INFO <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO, s)); }
#define LOGINFO1(s)     if(NX::dbg::LL_INFO_1 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_1, s)); }
#define LOGINFO2(s)     if(NX::dbg::LL_INFO_2 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_2, s)); }
#define LOGINFO3(s)     if(NX::dbg::LL_INFO_3 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_3, s)); }
#define LOGINFO4(s)     if(NX::dbg::LL_INFO_4 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_4, s)); }
#define LOGINFO5(s)     if(NX::dbg::LL_INFO_5 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_5, s)); }
#define LOGINFO6(s)     if(NX::dbg::LL_INFO_6 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_6, s)); }
#define LOGINFO7(s)     if(NX::dbg::LL_INFO_7 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_7, s)); }
#define LOGINFO8(s)     if(NX::dbg::LL_INFO_8 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_8, s)); }
#define LOGINFO9(s)     if(NX::dbg::LL_INFO_9 <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_INFO_9, s)); }
#define LOGDEBUG(s)     if(NX::dbg::LL_DEBUG <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_DEBUG, s)); }
#define LOGDETAIL(s)    if(NX::dbg::LL_DETAIL <= NxGlobalLog.get_accepted_level()) { NxGlobalLog.push(NX::dbg::log_item(NX::dbg::LL_DETAIL, s)); }


typedef bool(__stdcall *NXLOG_ACCEPT)(unsigned long level);
typedef void(__stdcall *NXLOG_WRITE)(unsigned long level, const wchar_t* message);

#define DECLARE_LOG_CALLBACKS()     static __inline NX::dbg::LOGLEVEL NxGlobalLogLevelNormalize(unsigned long level) {return (NX::dbg::LOGLEVEL)(level > NX::dbg::LL_ALL ? NX::dbg::LL_ALL : level); }  \
                                    bool __stdcall NxGlobalLogAccept(unsigned long level)   \
                                    {   \
                                        return ((unsigned long)NxGlobalLogLevelNormalize(level) <= NxGlobalLog.get_accepted_level());   \
                                    }   \
                                    void __stdcall NxGlobalLogWrite(unsigned long level, const wchar_t* message)    \
                                    {   \
                                        if((unsigned long)NxGlobalLogLevelNormalize(level) <= NxGlobalLog.get_accepted_level()) {  \
                                            NxGlobalLog.push(NX::dbg::log_item(NxGlobalLogLevelNormalize(level), message));   \
                                        }   \
                                    }

#define LOG_CALLBACK_ACCEPT  NxGlobalLogAccept
#define LOG_CALLBACK_WRITE   NxGlobalLogWrite

extern bool __stdcall NxGlobalLogAccept(unsigned long level);
extern void __stdcall NxGlobalLogWrite(unsigned long level, const wchar_t* message);


#endif