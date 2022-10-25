
#ifndef __NXSERV_LOG_HPP__
#define __NXSERV_LOG_HPP__

#include <nudf\exception.hpp>
#include <nudf\log.hpp>
#include <nudf\shared\engineenv.h>

namespace NX {
    

class log : public nudf::util::log::CFileLog
{
public:
    log();
    virtual ~log();    
    void init();
};

}   // namespace NX

//
//  Handy Macros
//
#define LOG(LEVEL, INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LEVEL)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LEVEL, NXMODNAME(), INFO, __VA_ARGS__)); \
    }

#define LOGEX(LEVEL, CODE, INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LEVEL)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LEVEL, NXMODNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGASSERT(INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGCRITICAL)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGCRITICAL, NXMODNAME(), 0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGERR(CODE, INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGERROR)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGERROR, NXMODNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGWARN(CODE, INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGWARNING)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGWARNING, NXMODNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGINF(INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGINFO)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGINFO, NXMODNAME(), INFO, __VA_ARGS__)); \
    }

#define LOGINF2(INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGINFO)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGINFO, NXMODNAME(), 0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGSYSINF(INFO, ...) \
    GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGINFO, NXMODNAME(), INFO, __VA_ARGS__))

#define LOGDBG(INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGDEBUG)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), INFO, __VA_ARGS__)); \
    }

#define LOGDBG2(INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGDEBUG)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), 0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGDMP(INFO, ...) \
    if(GLOBAL.log().AcceptLevel(LOGDETAIL)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGDETAIL, NXMODNAME(), INFO, __VA_ARGS__)); \
    }

//
//  VOID
//  LOG_EXCEPTION (
//                 _In_ const nudf::CException& e
//                 );
//
#define LOG_EXCEPTION(e) \
    if(GLOBAL.log().AcceptLevel(LOGERROR)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGERROR, NXMODNAME(), e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine(), L"Catch Exception. ")); \
    }

#define LOG_EXCEPTION2(e, info) \
    if(GLOBAL.log().AcceptLevel(LOGERROR)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGERROR, NXMODNAME(), e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine(), info)); \
    }

#define FUNCTION_WIND() \
    if(GLOBAL.log().AcceptLevel(LOGDEBUG)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"Function Wind (%s, %s, %d)", __FUNCTION__, __FILE__, __LINE__)); \
    }

#define FUNCTION_UNWIND() \
    if(GLOBAL.log().AcceptLevel(LOGDEBUG)) { \
        GLOBAL.log().Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"Function Unwind (%s, %s, %d)", __FUNCTION__, __FILE__, __LINE__)); \
    }


#endif  // #ifndef __NXRM_ENGINE_LOGSERV_HPP__