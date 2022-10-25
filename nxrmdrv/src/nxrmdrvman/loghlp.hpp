

#ifndef _FLTMAN_LOG_HLP_HPP__
#define _FLTMAN_LOG_HLP_HPP__


#include <nudf\exception.hpp>
#include <nudf\log.hpp>
#include <nudf\shared\moddef.h>


//
//  Handy Log Macro
//

#define LOGMAN(MGR, LEVEL, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LEVEL)) { \
        nudf::util::log::CLogEntry le(LEVEL, NXMODNAME(), INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGMANEX(MGR, LEVEL, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LEVEL)) { \
        nudf::util::log::CLogEntry le(LEVEL, NXMODNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGDRV(MGR, LEVEL, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LEVEL)) { \
        nudf::util::log::CLogEntry le(LEVEL, NXDRVNAME(), INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGDRVEX(MGR, LEVEL, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LEVEL)) { \
        nudf::util::log::CLogEntry le(LEVEL, NXDRVNAME(), CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

//
//  Detail Output
//

#define LOGMAN_ASSERT(MGR, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGCRITICAL)) { \
        nudf::util::log::CLogEntry le(LOGCRITICAL, NXMODNAME(), (LONG)CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGMAN_ERROR(MGR, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGERROR)) { \
        nudf::util::log::CLogEntry le(LOGERROR, NXMODNAME(), (LONG)CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGMAN_WARN(MGR, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGWARNING)) { \
        nudf::util::log::CLogEntry le(LOGWARNING, NXMODNAME(), (LONG)0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGMAN_INFO(MGR, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGINFO)) { \
        nudf::util::log::CLogEntry le(LOGINFO, NXMODNAME(), INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGMAN_DBG(MGR, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGDEBUG)) { \
        nudf::util::log::CLogEntry le(LOGDEBUG, NXMODNAME(), INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGMAN_DBG2(MGR, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGDEBUG)) { \
        nudf::util::log::CLogEntry le(LOGDEBUG, NXMODNAME(), (LONG)0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }


#define LOGDRV_ASSERT(MGR, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGCRITICAL)) { \
        nudf::util::log::CLogEntry le(LOGCRITICAL, NXDRVNAME(), (LONG)CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGDRV_ERROR(MGR, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGERROR)) { \
        nudf::util::log::CLogEntry le(LOGERROR, NXDRVNAME(), (LONG)CODE, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGDRV_WARN(MGR, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGWARNING)) { \
        nudf::util::log::CLogEntry le(LOGWARNING, NXDRVNAME(), (LONG)0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGDRV_INFO(MGR, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGINFO)) { \
        nudf::util::log::CLogEntry le(LOGINFO, NXDRVNAME(), INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGDRV_DBG(MGR, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGDEBUG)) { \
        nudf::util::log::CLogEntry le(LOGDEBUG, NXDRVNAME(), INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define LOGDRV_DBG2(MGR, CODE, INFO, ...) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGDEBUG)) { \
        nudf::util::log::CLogEntry le(LOGDEBUG, NXDRVNAME(), (LONG)0, __FILE__, __FUNCTION__, __LINE__, INFO, __VA_ARGS__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

//
//  VOID
//  LOG_EXCEPTION (
//                 _In_ const nudf::CException& e
//                 );
//
#define LOG_EXCEPTION(MGR, EXCEPTION) \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGERROR)) { \
        nudf::util::log::CLogEntry le(LOGERROR, NXMODNAME(), e.GetCode(), e.GetFile(), e.GetFunction(), e.GetLine(), L"Catch Exception. "); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define FUNCTION_WIND() \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGERROR)) { \
        nudf::util::log::CLogEntry le(LOGDEBUG, NXMODNAME(), L"Function Wind (%s, %s, %d)", __FUNCTION__, __FILE__, __LINE__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }

#define FUNCTION_UNWIND() \
    if(NULL!=MGR && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback && NULL!=((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback && ((PNXRMDRV_MANAGER)MGR)->DebugDumpCheckLevelCallback(LOGERROR)) { \
        nudf::util::log::CLogEntry le(LOGDEBUG, NXMODNAME(), L"Function Unwind (%s, %s, %d)", __FUNCTION__, __FILE__, __LINE__); \
        ((PNXRMDRV_MANAGER)MGR)->DebugDumpCallback(le.GetData().c_str()); \
    }


#endif