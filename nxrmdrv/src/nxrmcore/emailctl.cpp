#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "emailctl.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA Global;

#ifdef __cplusplus
}
#endif

static LPMAPISENDMAIL					g_fnorg_MAPISendMail = NULL;
static LPMAPISENDMAIL					g_fn_MAPISendMail_trampoline = NULL;

typedef struct _MSG_RENAME_NODE
{
	LIST_ENTRY	Link;

	ULONG		Index;

	LPSTR		lpszOldFileName;

	char		FileName[MAX_PATH];

}MSG_RENAME_NODE, *PMSG_RENAME_NODE;

BOOL InitializeExplorerSendEmailHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		//
		// reuse office sendmail related handle and lock
		//
		if (!Global.Mapi32Handle)
		{
			Global.Mapi32Handle = GetModuleHandleW(MAPI32_MODULE_NAME);
		}

		if (!Global.Mapi32Handle)
		{
			bRet = FALSE;
			break;
		}

		//
		// reference Mapi32 to prevent it from being unloaded
		//
		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, MAPI32_MODULE_NAME, &Global.Mapi32Handle))
		{
			Global.Mapi32Handle = NULL;
			bRet = FALSE;
			break;
		}

		if (Global.ExplorerSendMailHooksInitialized)
		{
			break;
		}

		g_fnorg_MAPISendMail = (LPMAPISENDMAIL)GetProcAddress(Global.Mapi32Handle, EMAILCTL_EXPLORER_MAPISENDMAIL_PROC_NAME);

		if (g_fnorg_MAPISendMail)
		{
			if (!install_hook(g_fnorg_MAPISendMail, (PVOID*)&g_fn_MAPISendMail_trampoline, Core_Explorer_MAPISendMail))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.ExplorerSendMailHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;

}

void CleanupExplorerSendEmailHook(void)
{
	if (g_fn_MAPISendMail_trampoline)
	{
		remove_hook(g_fn_MAPISendMail_trampoline);
		g_fn_MAPISendMail_trampoline = NULL;
	}
}

ULONG WINAPI Core_Explorer_MAPISendMail(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	)
{
	ULONG ulRet = 0;

	ULONG i = 0;

	LIST_ENTRY DisplayNameList = {0};

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	MSG_RENAME_NODE *pNode = NULL;

	MapiFileDesc *pMsg = NULL;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	WCHAR FullPathFileName[MAX_PATH] = {0};

	DWORD dwRet = 0;

	HRESULT hr = S_OK;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		InitializeListHead(&DisplayNameList);

		if (!init_rm_section_safe())
		{
			ulRet = g_fn_MAPISendMail_trampoline(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
			break;
		}

		for (i = 0; i < lpMessage->nFileCount; i++)
		{
			pMsg = lpMessage->lpFiles + i;

			if (pMsg->lpszPathName && pMsg->lpszFileName)
			{
				hFile = CreateFileA(pMsg->lpszPathName,
									GENERIC_READ,
									FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL);

				if (hFile == INVALID_HANDLE_VALUE)
				{
					continue;
				}

				dwRet = GetFinalPathNameByHandleW(hFile, 
												  FullPathFileName, 
												  sizeof(FullPathFileName)/sizeof(WCHAR), 
												  FILE_NAME_OPENED|VOLUME_NAME_DOS);

				if (dwRet == 0 || dwRet >= sizeof(FullPathFileName)/sizeof(WCHAR))
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
					continue;
				}

				//
				// take care of rights
				//
				hr = nudf::util::nxl::NxrmCheckRights(FullPathFileName, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (FAILED(hr))
				{
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
					continue;
				}

				CloseHandle(hFile);
				hFile = INVALID_HANDLE_VALUE;

				pNode = (MSG_RENAME_NODE*)malloc(sizeof(MSG_RENAME_NODE));

				if (!pNode)
				{
					continue;
				}

				memset(pNode, 0, sizeof(MSG_RENAME_NODE));

				pNode->Index = i;
				pNode->lpszOldFileName = pMsg->lpszFileName;

				memcpy(pNode->FileName,
					   pMsg->lpszFileName,
					   min(strlen(pMsg->lpszFileName), sizeof(pNode->FileName) - sizeof(char)));

				memcpy(pNode->FileName + strlen(pNode->FileName),
					   ".nxl",
					   min(strlen(".nxl"), sizeof(pNode->FileName) - strlen(pNode->FileName) - sizeof(char)));

				InsertTailList(&DisplayNameList, &pNode->Link);

				pMsg->lpszFileName = pNode->FileName;
			}
		}

		ulRet = g_fn_MAPISendMail_trampoline(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);

	} while (FALSE);

	FOR_EACH_LIST_SAFE(ite, tmp, &DisplayNameList)
	{
		pNode = CONTAINING_RECORD(ite, MSG_RENAME_NODE, Link);

		RemoveEntryList(ite);

		pMsg = lpMessage->lpFiles + pNode->Index;

		pMsg->lpszFileName = pNode->lpszOldFileName;
		
		free(pNode);
	}

	return ulRet;
}

