
#pragma once
#ifndef __NXLOGDB_DBGLOG_HPP__
#define __NXLOGDB_DBGLOG_HPP__


extern bool DBGLOGACCEPT(unsigned long level);
extern void DBGLOG(unsigned long level, const wchar_t* message, ...);


#endif