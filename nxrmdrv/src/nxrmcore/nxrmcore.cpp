// nxrmcore.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "nxrmcore.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"


#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA Global;

#ifdef __cplusplus
}
#endif

BOOL init_rm_section_safe(void)
{
	BOOL bRet = FALSE;

	do 
	{
		if(!Global.Section)
		{
			EnterCriticalSection(&Global.SectionLock);

			Global.Section = init_transporter_client();

			LeaveCriticalSection(&Global.SectionLock);
		}

		if(!Global.Section)
		{
			break;
		}

		bRet = is_transporter_enabled(Global.Section);

	} while (FALSE);

	return bRet;
}