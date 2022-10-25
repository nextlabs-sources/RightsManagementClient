#pragma once

#include <MAPI.h>

#define	EMAILCTL_EXPLORER_MAPISENDMAIL_PROC_NAME	"MAPISendMail"

ULONG WINAPI Core_Explorer_MAPISendMail(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	);

BOOL InitializeExplorerSendEmailHook(void);
void CleanupExplorerSendEmailHook(void);