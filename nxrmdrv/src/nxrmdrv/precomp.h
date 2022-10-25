
#pragma once

#include <ntdef.h>
#include <ntifs.h>
#include <ntimage.h>
#include <ntstrsafe.h>
#include <nudf\shared\rbtree.h>
#include <nudf\shared\listentry.h>


#ifdef _DEBUG
#define dprintf DbgPrint
#else
#define dprintf
#endif 

#define NXRMTAG		'IRAM'