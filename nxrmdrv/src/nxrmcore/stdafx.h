// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <limits.h>
#include <assert.h>
#include <winternl.h>
#include <stdio.h>
#include <stdlib.h>
#include <winioctl.h>
#include <Ole2.h>
#include <OCIdl.h>
#include <stdexcpt.h>
#include <nudf\shared\listentry.h>
#include <nudf\shared\listhlp.h>
#include <nudf\shared\obdef.h>
#include <nudf\shared\fltdef.h>
#include <nudf\shared\rightsdef.h>
#include <nudf\shared\obutil.h>
#include <nudf\nxlutil.hpp>
#include <nudf\string.hpp>
#include "nxrmdrv.h"
#include "nxrmdrvman.h"
#include "nxrmcorehlp.h"
#include "nxrmcore.h"

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif

// TODO: reference additional headers your program requires here
