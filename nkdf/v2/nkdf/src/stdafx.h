


#pragma once
#ifndef __NKDF_STDAFX_H__
#define __NKDF_STDAFX_H__


#include <ntifs.h>
#include <fltkernel.h>
#include <Ntstrsafe.h>

#define try_return(S)   {S; goto try_exit;}
#define try_return2(S)   {S; goto try_exit2;}


#define TAG_TMP     '**kN'
#define TAG_KEY     'YKkN'





#endif