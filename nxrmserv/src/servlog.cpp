
#include <Windows.h>
#include <assert.h>
#include <Wtsapi32.h>
#include <Sddl.h>

#include <nudf\exception.hpp>
#include <nudf\registry.hpp>
#include <nudf\shared\enginectl.h>
#include <nudf\shared\enginectl.h>

#include "nxrmserv.h"
#include "servlog.hpp"


using namespace NX;


//
//  class CLogServ
//

NX::log::log()
{
}

NX::log::~log()
{
}

void NX::log::init()
{
    SetLogDir(GLOBAL.dir_dbglogs().c_str());
    SetLogName(L"DebugDump");
    SetRotation(10);
    SetFileSizeLimit(0xF00000);   // 15*1024*1024 = 0xF00000 = 15728640 = 15MB per log file

    nudf::win::CRegLocalMachine lm;
    nudf::win::CRegKey rkserv;
    LOGLEVEL LogLevel = LOGDEBUG;
    ULONG    LogQueue = 1024;

    if(rkserv.Open(lm, L"SYSTEM\\CurrentControlSet\\services\\nxrmserv", KEY_READ)) {
        ULONG value = 0;
        if(rkserv.GetValue(L"LogLevel", &value)) {
            value = (value < LOGERROR) ? LOGERROR : value;
            LogLevel = (LOGLEVEL)value;
        }
        if(rkserv.GetValue(L"LogQueue", &value)) {
            LogQueue = (value < 512) ? 512 : ((value > 4096) ? 4096 : value);
        }
    }

    SetAcceptLevel(LogLevel);
    SetQueueSize(LogQueue);
}