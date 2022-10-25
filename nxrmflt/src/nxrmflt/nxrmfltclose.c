#include "nxrmfltdef.h"
#include "nxrmfltclose.h"

extern DECLSPEC_CACHEALIGN ULONG				gTraceFlags;
extern DECLSPEC_CACHEALIGN NXRMFLT_GLOBAL_DATA	Global;

FLT_PREOP_CALLBACK_STATUS
nxrmfltPreClose(
_Inout_ PFLT_CALLBACK_DATA				Data,
_In_ PCFLT_RELATED_OBJECTS				FltObjects,
_Flt_CompletionContext_Outptr_ PVOID	*CompletionContext
)
{
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}