#pragma once

#define MAX_RIBBON_ID_LENGTH	32

typedef struct _RIBBON_ID_INFO {

	WCHAR			RibbonId[MAX_RIBBON_ID_LENGTH];

	ULONGLONG		RightsMask;

	ULONGLONG		CustomRights;

} RIBBON_ID_INFO, *PRIBBON_ID_INFO;