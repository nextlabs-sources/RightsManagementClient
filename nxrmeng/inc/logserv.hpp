
#ifndef __NXRM_ENGINE_LOGSERV_HPP__
#define __NXRM_ENGINE_LOGSERV_HPP__

#include <nudf\exception.hpp>
#include <nudf\log.hpp>
#include <nudf\shared\engineenv.h>

namespace nxrm {
namespace engine {



class CLogServ : public nudf::util::log::CFileLog
{
public:
    CLogServ();
    virtual ~CLogServ();
    
    VOID Initialize();
    VOID Cleanup();
};

}   // namespace engine
}   // namespace nxrm

//
//  Handy Macros
//
#define LOG(LEVEL, INFO, ...) \
    if(_LogServ.AcceptLevel(LEVEL)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LEVEL, NXMODNAME(), INFO, __VA_ARGS__)); \
    }

#define LOGEX(LEVEL, CODE, INFO, ...) \
    if(_LogServ.AcceptLevel(LEVEL)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LEVEL, NXMODNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGASSERT(INFO, ...) \
    if(_LogServ.AcceptLevel(LOGCRITICAL)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGCRITICAL, NXMODNAME(), 0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGERR(CODE, INFO, ...) \
    if(_LogServ.AcceptLevel(LOGERROR)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGERROR, NXMODNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGWARN(CODE, INFO, ...) \
    if(_LogServ.AcceptLevel(LOGWARNING)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGWARNING, NXMODNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGINF(INFO, ...) \
    if(_LogServ.AcceptLevel(LOGINFO)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGINFO, NXMODNAME(), INFO, __VA_ARGS__)); \
    }

#define LOGINF2(INFO, ...) \
    if(_LogServ.AcceptLevel(LOGINFO)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGINFO, NXMODNAME(), 0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

#define LOGSYSINF(INFO, ...) \
    _LogServ.Push(nudf::util::log::CLogEntry(LOGINFO, NXMODNAME(), INFO, __VA_ARGS__))

#define LOGDBG(INFO, ...) \
    if(_LogServ.AcceptLevel(LOGDEBUG)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), INFO, __VA_ARGS__)); \
    }

#define LOGDBG2(INFO, ...) \
    if(_LogServ.AcceptLevel(LOGDEBUG)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), 0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__)); \
    }

//
//  VOID
//  LOG_EXCEPTION (
//                 _In_ const nudf::CException& e
//                 );
//
#define LOG_EXCEPTION(e) \
    if(_LogServ.AcceptLevel(LOGERROR)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGERROR, NXMODNAME(), e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine(), L"Catch Exception. ")); \
    }

#define LOG_EXCEPTION2(e, info) \
    if(_LogServ.AcceptLevel(LOGERROR)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGERROR, NXMODNAME(), e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine(), info)); \
    }

#define FUNCTION_WIND() \
    if(_LogServ.AcceptLevel(LOGDEBUG)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"Function Wind (%s, %s, %d)", __FUNCTION__, __FILE__, __LINE__)); \
        (VOID)RmEngineLog(LogEntry.GetData().c_str()); \
    }

#define FUNCTION_UNWIND() \
    if(_LogServ.AcceptLevel(LOGDEBUG)) { \
        _LogServ.Push(nudf::util::log::CLogEntry(LOGDEBUG, NXMODNAME(), L"Function Unwind (%s, %s, %d)", __FUNCTION__, __FILE__, __LINE__)); \
        RmEngineLog(LogEntry.GetData().c_str()); \
    }


#endif  // #ifndef __NXRM_ENGINE_LOGSERV_HPP__