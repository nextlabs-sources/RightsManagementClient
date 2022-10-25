#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "adobe.h"
#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

	extern	CORE_GLOBAL_DATA	Global;

	HWND						g_AdobeHwnd = NULL;

	HINSTANCE					g_AdobeInstance = NULL;

	HFT							g_CoreHFT = NULL;
	UINT						g_CoreVersion = 0;

	HFT							g_AcroSupportHFT = NULL;
	UINT						g_AcroSupportVersion =0;

	HFT							g_CosHFT = 0;
	UINT						g_CosVersion = 0;

	HFT							g_PDModelHFT = NULL;
	UINT						g_PDModelVersion = 0;

	HFT							g_PDFEditReadHFT = NULL;
	UINT						g_PDFEditReadVersion = 0;

	HFT							g_PDFEditWriteHFT = NULL;
	UINT						g_PDFEditWriteVersion = 0;

	HFT							g_PDSysFontHFT = NULL;
	UINT						g_PDSysFontVersion = 0;

	HFT							g_PagePDEContentHFT = NULL;
	UINT						g_PagePDEContentVersion = 0;

	HFT							g_AcroViewHFT = NULL;
	UINT						g_AcroViewVersion = 0;

	HFT							g_PDSWriteHFT = NULL;
	UINT						g_PDSWriteVersion = 0;

	HFT							g_PDSReadHFT = NULL;
	UINT						g_PDSReadVersion = 0;

	HFT							g_WinHFT = NULL;
	UINT						g_WinVersion = 0;

	HFT							g_ASExtraHFT = NULL;
	UINT						g_ASExtraVersion = 0;

	HFT							g_PDMetadataHFT = NULL;
	UINT						g_PDMetadataVersion = 0;

	ExtensionID					g_ExtensionID;

	ID2D1BitmapBrush* build_bitmap_brush(ID2D1DCRenderTarget *pRT, const WCHAR *FileName);

	extern SETWINDOWDISPLAYAFFINITY	g_fnorg_SetWindowDisplayAffinity;

	extern nudf::util::CObligations *query_engine(const WCHAR *FileName, ULONGLONG EvaluationId);

	extern BOOL	send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

	extern LONGLONG get_module_checksum(PVOID pBase);

	extern INVALIDATERECT		g_fnorg_InvalidateRect;

	extern 	BOOL send_overlay_windows_info_to_engine(HWND hWnd, BOOL Add);

#ifdef __cplusplus
}
#endif

static PLUGINMAIN				g_fnorg_PlugInMain = NULL;
static PLUGINMAIN				g_fn_PlugInMain_trampoline = NULL;
static PISetupSDKProcType		g_fnorg_PISetupSDK = NULL;
static PIHandshakeProcType		g_fnorg_PIHandshake = NULL;
static PIInitProcType			g_fnorg_PluginInit = NULL;
static GETPARENT				g_fnorg_GetParent = NULL;
static GETCLIENTRECT			g_fnorg_GetClientRect = NULL;
static D2D1CREATEFACTORY		g_fnorg_D2D1CreateFactory = NULL;
static ADOBE_COCREATEINSTANCE	g_fnorg_CoCreateInstance = NULL;
static ADOBE_MAPIINITIALIZE				g_fnorg_MAPIInitialize = NULL;
static ADOBE_MAPIINITIALIZE				g_fn_MAPIInitialize_trampoline = NULL;

static ADOBE_XI_SENDMAIL				g_fnorg_AdobeXISendMail = NULL;
static ADOBE_XI_SENDMAIL				g_fn_AdobeXISendMail_trampoline = NULL;

static ADOBE_ACROWINMAINSANDBOX			g_fnorg_AcroWinMainSandbox = NULL;
static ADOBE_ACROWINMAINSANDBOX			g_fn_AcroWinMainSandbox_trampoline = NULL;

static ASExtensionMgrGetHFTSELPROTO		g_fnorg_ASExtensionMgrGetHFT = NULL;
static ASExtensionMgrGetHFTSELPROTO		g_fn_ASExtensionMgrGetHFT_trampoline = NULL;
static ASAtomFromStringSELPROTO			g_fnorg_ASAtomFromString = NULL;

static BOOL InitializeAdobeSendMailHooksXI(void);
static BOOL InitializeAdobeSendMailHooksX(void);

static USHORT GetRequestedHFT(const char* table, UINT requiredVer, UINT *resultingVer, HFT *resultHFT);

static BOOL can_I_send_email(void);
static BOOL update_protected_app_info(const WCHAR *ActiveDocFileName, ULONGLONG RightsMask, ULONGLONG CustomRights, ULONGLONG EvaluationId);

static BOOL query_saved_adobe_hft_functions(PVOID *ppASExtensionMgrGetHFT, PVOID *ppASAtomFromString);
static void update_adobe_hft_functions(PVOID pASExtensionMgrGetHFT, PVOID pASAtomFromString);

BOOL InitializeAdobeReaderHook(void)
{
	BOOL bRet = TRUE;

	HMODULE hUser32 = NULL;
	HMODULE hOle32 = NULL;

	HMODULE hOtherPluginHandle = NULL;

	const WCHAR *Plugins[] = { ANNOTS_MODULE_NAME,			\
							   ACROFORM_MODULE_NAME,		\
							   PPKLITE_MODULE_NAME,			\
							   UPDATER_MODULE_NAME,			\
							   IA32_MODULE_NAME,			\
							   PDDOM_MODULE_NAME,			\
							   ACCESSIBILITY_MODULE_NAME,	\
							   ACROFORM_MODULE_NAME,		\
							   CHECKERS_MODULE_NAME,		\
							   DIGSIG_MODULE_NAME,			\
							   DVA_MODULE_NAME,				\
							   EBOOK_MODULE_NAME,			\
							   ESCRIPT_MODULE_NAME,			\
							   MAKEACCESSIBLE_MODULE_NAME,	\
							   MULTIMEDIA_MODULE_NAME,		\
							   READOUTLOUD_MODULE_NAME,		\
							   REFLOW_MODULE_NAME,			\
							   SAVEASRTF_MODULE_NAME,		\
							   SEARCH_MODULE_NAME,			\
							   SENDMAIL_MODULE_NAME,		\
							   SPELLING_MODULE_NAME,		\
							   WEBLINK_MODULE_NAME };

	do 
	{
		for (int i = 0; i < sizeof(Plugins)/sizeof(WCHAR*); i++)
		{
			hOtherPluginHandle = GetModuleHandleW(Plugins[i]);

			if (hOtherPluginHandle)
			{
				break;
			}
		}

		if(hOtherPluginHandle == NULL)
		{
			bRet = FALSE;
			break;
		}

		//
		// AcroRd32 depends on user32 and GDI32
		// no way for hUser32 to be NULL
		//
		hUser32 = GetModuleHandleW(USER32_MODULE_NAME);


		//
		// Annots depends on OLE32
		// no way for hOle32 to be NULL
		//
		hOle32 = GetModuleHandleW(OLE32_MODULE_NAME);

		EnterCriticalSection(&Global.AnnotsLock);

		do 
		{
			if (!Global.AdobeHooksInitialized)
			{
				g_fnorg_PlugInMain = (PLUGINMAIN)GetProcAddress(hOtherPluginHandle, ADOBE_PLUGINMAIN_PROC_NAME);

				if (g_fnorg_PlugInMain)
				{
					if (!install_hook(g_fnorg_PlugInMain, (PVOID*)&g_fn_PlugInMain_trampoline, Core_PlugInMain))
					{
						bRet = FALSE;
						break;
					}
				}

				g_fnorg_SetWindowDisplayAffinity = (SETWINDOWDISPLAYAFFINITY)GetProcAddress(hUser32, NXRMCORE_SETWINDOWDISPLAYAFFINITY_PROC_NAME);

				g_fnorg_GetParent = (GETPARENT)GetProcAddress(hUser32, NXRMCORE_GETPARENT_PROC_NAME);

				g_fnorg_GetClientRect = (GETCLIENTRECT)GetProcAddress(hUser32, NXRMCORE_GETCLIENTRECT_PROC_NAME);

				g_fnorg_CoCreateInstance = (ADOBE_COCREATEINSTANCE)GetProcAddress(hOle32, ADOBE_COCREATEINSTANCE_PROC_NAME);

				Global.AnnotsHandle = hOtherPluginHandle;

				Global.AdobeHooksInitialized = TRUE;
			}

		} while (FALSE);

		LeaveCriticalSection(&Global.AnnotsLock);

	} while (FALSE);

	return bRet;
}

void CleanupAdobeReaderHook(void)
{
	if (g_fn_PlugInMain_trampoline)
	{
		remove_hook(g_fn_PlugInMain_trampoline);
		g_fn_PlugInMain_trampoline = NULL;
	}

	if (Global.IsAdobeReader)
	{
		if (Global.D2D1Factory)
		{
			Global.D2D1Factory->Release();
			Global.D2D1Factory = NULL;
		}

		if (Global.WICFactory)
		{
			Global.WICFactory->Release();
			Global.WICFactory = NULL;
		}

		if (Global.D2D1Handle)
		{
			FreeLibrary(Global.D2D1Handle);
			Global.D2D1Handle = NULL;
		}
	}
}

USHORT WINAPI Core_PlugInMain(UINT appHandshakeVersion, UINT *handshakeVersion, PISetupSDKProcType *setupProc, void *windowsData)
{
	USHORT usRet = 0;

	V0200_DATA* dataPtr = (V0200_DATA*) windowsData;

	do 
	{
		usRet = g_fn_PlugInMain_trampoline(appHandshakeVersion, handshakeVersion, setupProc, windowsData);

		if (!usRet)
		{
			break;
		}

		g_AdobeHwnd		= dataPtr->hWnd;
		g_AdobeInstance	= dataPtr->hInstance;

		g_fnorg_PISetupSDK = *setupProc;

		*setupProc = Core_PISetupSDK;

	} while (FALSE);

	return usRet;
}

USHORT __cdecl Core_PISetupSDK(UINT handshakeVersion, void *sdkData)
{
	USHORT usRet = 0;
	size_t i = 0;
	USHORT bSuccess = 0;

	PISDKData_V0200 *data = NULL;

	HFTINFO hftInfo[] = {
	{"Core", PI_CORE_VERSION, &g_CoreVersion, &g_CoreHFT, false},
	{"Cos", PI_COS_VERSION, &g_CosVersion ,&g_CosHFT, false},
	{"PDModel", PI_PDMODEL_VERSION, &g_PDModelVersion, &g_PDModelHFT, false},
	{"PDFEditWrite", PI_PDFEDIT_WRITE_VERSION, &g_PDFEditWriteVersion,&g_PDFEditWriteHFT, false},
	{"PDFEditRead", PI_PDFEDIT_READ_VERSION, &g_PDFEditReadVersion,&g_PDFEditReadHFT, false},
	{"PDSysFont", PI_PDSYSFONT_VERSION, &g_PDSysFontVersion, &g_PDSysFontHFT, false},
	{"PagePDEContent", PI_PAGE_PDE_CONTENT_VERSION, &g_PagePDEContentVersion, &g_PagePDEContentHFT, false},
	{"AcroView", PI_ACROVIEW_VERSION, &g_AcroViewVersion, &g_AcroViewHFT, false},
	{"PDSWrite", PI_PDSEDIT_WRITE_VERSION, &g_PDSWriteVersion, &g_PDSWriteHFT, false},
	{"PDSRead", PI_PDSEDIT_READ_VERSION, &g_PDSReadVersion, &g_PDSReadHFT, false},
	{"Win", PI_WIN_VERSION, &g_WinVersion, &g_WinHFT, false},
	{"ASExtra", PI_ASEXTRA_VERSION, &g_ASExtraVersion, &g_ASExtraHFT, false},
	{"PDMetadata", PI_PDMETADATA_VERSION, &g_PDMetadataVersion, &g_PDMetadataHFT, false},
	};

	do 
	{
		usRet = g_fnorg_PISetupSDK(handshakeVersion, sdkData);

		if (!usRet)
		{
			break;
		}
	
		if (handshakeVersion != HANDSHAKE_V0200)
		{
			usRet = FALSE;
			break;
		}

		data = (PISDKData_V0200 *)sdkData;

		if (data->handshakeVersion != HANDSHAKE_V0200)
		{
			usRet = FALSE;
			break;
		}

		g_CoreHFT		= data->coreHFT;
		g_ExtensionID	= data->extensionID;
		g_CoreVersion	= 0x00020000;

		g_AcroSupportHFT = ASExtensionMgrGetHFT(ASAtomFromString("AcroSupport"), ASCallsHFT_VERSION_6); 

		if (!g_AcroSupportHFT)
		{
			break;
		}

		g_AcroSupportVersion = ASCallsHFT_VERSION_6;
		g_AcroSupportVersion = HFTGetVersion(g_AcroSupportHFT);

		bSuccess = GetRequestedHFT("AcroSupport", PI_ACROSUPPORT_VERSION, &g_AcroSupportVersion, &g_AcroSupportHFT);

		for (i=0; i<sizeof(hftInfo)/sizeof(hftInfo[0]); i++) 
		{
			bSuccess = GetRequestedHFT(hftInfo[i].name,hftInfo[i].ver,hftInfo[i].retver,hftInfo[i].hft);

			if (!bSuccess && !hftInfo[i].optional)
			{
				bSuccess = FALSE;
				break;
			}
		}

		if (!bSuccess)
		{
			break;
		}

		g_fnorg_PIHandshake = (PIHandshakeProcType)data->handshakeCallback;

		data->handshakeCallback = Core_PIHandshake;

		update_adobe_hft_functions((PVOID)ASExtensionMgrGetHFT, (PVOID)ASAtomFromString);

	} while (FALSE);

	return usRet;
}

USHORT Core_PIHandshake(UINT handshakeVersion, void *handshakeData)
{
	USHORT usRet = TRUE;

	PIHandshakeData_V0200 *hsData = (PIHandshakeData_V0200 *)handshakeData;

	do 
	{
		usRet = g_fnorg_PIHandshake(handshakeVersion, handshakeData);

		if (!usRet)
		{
			break;
		}

		if (handshakeVersion != HANDSHAKE_V0200)
		{
			usRet = FALSE;
			break;
		}

		g_fnorg_PluginInit = (PIInitProcType)hsData->initCallback;

		hsData->initCallback = Core_PluginInit;
					
	} while (FALSE);

	return usRet;
}

USHORT __cdecl Core_PluginInit(void)
{
	USHORT usRet = TRUE;

	HRESULT hr = S_OK;

	AVDoc ActiveDoc = NULL;
	AVPageView PageView = NULL;

	do 
	{
		if (g_fnorg_PluginInit)
		{
			usRet = g_fnorg_PluginInit();
		}

		ActiveDoc = AVAppGetActiveDoc();

		if (ActiveDoc)
		{
			Core_OnAVAppFrontDocDidChange(ActiveDoc, NULL);	
		}

		AVAppRegisterNotification(AVAppFrontDocDidChangeSEL, 0, Core_OnAVAppFrontDocDidChange, NULL);

		AVAppRegisterNotification(AVDocWindowWasAddedSEL, 0, Core_OnAVDocWindowWasAdded, NULL);

	} while (FALSE);

	return usRet;
}

static USHORT GetRequestedHFT(const char* table, UINT requiredVer, UINT *resultingVer, HFT *resultHFT)
{
	USHORT tablename = ASAtomFromString(table);
	UINT resultVer = HFT_ERROR_NO_VERSION;
	/* these are the versions of Acrobat where we did not support GetVersion of an HFT. */
	static const UINT versionLessVersions[] = {0x00050001, 0x00050000, 0x00040005, 0x00040000, 0x00020003, 0x00020002, 0x00020001};
	static const size_t kNUMVERSIONS = sizeof(versionLessVersions) / sizeof(UINT);

	size_t i = 0;
	HFT thft = NULL;

	if (g_AcroSupportVersion >= ASCallsHFT_VERSION_6) 
	{
		thft = ASExtensionMgrGetHFT(tablename, requiredVer);

		if (thft) 
		{
			resultVer = HFTGetVersion(thft);
		}
	}

	if (resultVer == HFT_ERROR_NO_VERSION)
	{
		for (i=0;i<kNUMVERSIONS;i++) 
		{
			if (versionLessVersions[i]<requiredVer)
				break;
			
			thft = ASExtensionMgrGetHFT(tablename, versionLessVersions[i]);
			
			if (thft)
				break;
		}

		if (thft) 
			resultVer = versionLessVersions[i];
		else 
			resultVer = 0;
	}

	*resultHFT = thft;
	*resultingVer = resultVer;

	return *resultingVer != 0;
}

void __cdecl Core_OnAVAppFrontDocDidChange(AVDoc doc, void *clientData)
{
	AVPageView pPageView = NULL;
	PDDoc pPDDoc = NULL;
	ASFile pASFile = NULL;
	ASPathName pASPathName = NULL;
	ASPlatformPath pPlatformPath = NULL;
	WCHAR *FileName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	WinPort WindowInfo = NULL;
	HWND hWnd = NULL;
	HWND hParentWnd = NULL;

	PAGEVIEWCTX *PageViewCtx = NULL;
	//RECT rc = {0};
	//D2D1_SIZE_U size = {0};
	AVDevRect PageViewRect = {0};

	HRESULT hr = S_OK;

	LIST_ENTRY *ite = NULL;

	BOOL bSendNotificationToEngine = FALSE;

	do 
	{
		if (!doc)
		{
			break;
		}

		pPDDoc = AVDocGetPDDoc(doc);

		if (!pPDDoc)
		{
			break;
		}

		pASFile = PDDocGetFile(pPDDoc);

		if (!pASFile)
		{
			break;
		}

		pASPathName = ASFileAcquirePathName(pASFile);

		if (!pASPathName)
		{
			break;
		}

		if (0 != ASFileSysAcquirePlatformPath(ASGetDefaultUnicodeFileSys(), pASPathName, ASAtomFromString("WinUnicodePath"), &pPlatformPath))
		{
			break;
		}

		FileName = ASPlatformPathGetCstringPtr(pPlatformPath);

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   FileName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(FileName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (Global.IsLaunchPrinting) {
			Global.IsLaunchPrinting = FALSE;// reset the flag. User has open a file 
		}

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(FileName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			Global.DisablePrinting = FALSE;
			Global.DisableClipboard = FALSE;
			Global.DisableSendMail = FALSE;

			//if (Global.AdobeMode == ADOBE_MODE_PROTECTEDCHILD)
			{
				update_protected_app_info(FileName, BUILTIN_RIGHT_ALL, 0, 0xFFFFFFFFFFFFFFFF);
			}

			break;
		}

		if (!(RightsMask & BUILTIN_RIGHT_PRINT))
		{
			Global.DisablePrinting = TRUE;
		}
		else
		{
			Global.DisablePrinting = FALSE;
		}
		
		if (!(RightsMask & BUILTIN_RIGHT_CLIPBOARD))
		{
			Global.DisableClipboard = TRUE;
		}
		else
		{
			Global.DisableClipboard = FALSE;
		}

		if (!(RightsMask & BUILTIN_RIGHT_SEND))
		{
			Global.DisableSendMail = TRUE;
		}
		else
		{
			Global.DisableSendMail = FALSE;
		}

		//if (Global.AdobeMode == ADOBE_MODE_PROTECTEDCHILD)
		{
			update_protected_app_info(FileName, RightsMask, CustomRightsMask, EvaluationId);
		}

		pPageView = AVDocGetPageView(doc);

		if (pPageView)
		{
			WindowInfo = (WinPort)AVPageViewAcquireMachinePort(pPageView);

			if (WindowInfo)
			{
				hWnd = WindowInfo->hWnd;

				hParentWnd = g_fnorg_GetParent(hWnd);

				while(hParentWnd)
				{
					hWnd = hParentWnd;

					hParentWnd = g_fnorg_GetParent(hWnd);
				}

				if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
				{
					g_fnorg_SetWindowDisplayAffinity(hWnd, WDA_MONITOR);
				}

				AVPageViewReleaseMachinePort(pPageView, (void*)WindowInfo);
			}
		}

		if (pPageView)
		{
			WindowInfo = (WinPort)AVPageViewAcquireMachinePort(pPageView);

			if (WindowInfo)
			{
				EnterCriticalSection(&Global.PageViewCtxListLock);

				FOR_EACH_LIST(ite, &Global.PageViewCtxList)
				{
					PageViewCtx = CONTAINING_RECORD(ite, PAGEVIEWCTX, Link);

					if (PageViewCtx->hWnd == WindowInfo->hWnd)
					{
						break;
					}
					else
					{
						PageViewCtx = NULL;
					}
				}

				if (!PageViewCtx)
				{
					do
					{
						//
						// take care of obligations
						//
						Obligations = query_engine(FileName, EvaluationId);

						if (!Obligations)
						{
							break;
						}

						if (Obligations->IsEmpty())
						{
							break;
						}

						OverlayOb = (Obligations->GetObligations())[OB_ID_OVERLAY];

						BitmapFileName = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_IMAGE];

						if (!BitmapFileName.length())
						{
							break;
						}

						Opacity = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_TRANSPARENCY];

						if (Opacity.length())
						{
							nudf::string::ToInt<wchar_t>(Opacity, &Global.Opacity);
						}
						else
						{
							Global.Opacity = 10;
						}

						PageViewCtx = (PAGEVIEWCTX*)malloc(sizeof(PAGEVIEWCTX));

						memset(PageViewCtx, 0, sizeof(PAGEVIEWCTX));

						if (PageViewCtx)
						{
							do 
							{
								PageViewCtx->hWnd = WindowInfo->hWnd;

								PageViewCtx->Doc = doc;

								PageViewCtx->hOverlayWnd = hWnd;

								bSendNotificationToEngine = TRUE;

								if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
								{
									PageViewCtx->hCompositionWnd = hWnd;
								}
								else
								{
									PageViewCtx->hCompositionWnd = NULL;
								}

								memcpy(&PageViewCtx->LastRect,
									   &PageViewRect,
									   min(sizeof(PageViewCtx->LastRect), sizeof(PageViewRect)));

								PageViewCtx->InvalidCount = 0;

								InsertHeadList(&Global.PageViewCtxList, &PageViewCtx->Link);

								AVPageViewGetAperture(pPageView, &PageViewRect);

								AVAppRegisterNotification(AVDocWillCloseSEL, 0, Core_OnAVDocWillClose, PageViewCtx);

							} while (FALSE);

							if (!SUCCEEDED(hr))
							{
								free(PageViewCtx);
								PageViewCtx = NULL;
							}

						}

					} while(FALSE);
				}
				
				LeaveCriticalSection(&Global.PageViewCtxListLock);

				AVPageViewReleaseMachinePort(pPageView, (void*)WindowInfo);
			}
		}

	} while (FALSE);

	if (bSendNotificationToEngine && hWnd)
	{
		send_overlay_windows_info_to_engine(hWnd, TRUE);
	}

	if (pASPathName)
	{
		ASFileSysReleasePath(NULL, pASPathName);
		pASPathName = NULL;
	}

	if (pPlatformPath)
	{
		ASFileSysReleasePlatformPath(NULL, pPlatformPath);
		pPlatformPath = NULL;
		FileName = NULL;
	}

	return;
}

void __cdecl Core_OnAVDocDidOpen(AVDoc doc, int error, void *clientdata)
{
	return;
}

void __cdecl Core_OnAVDocWillClose(AVDoc doc, void *clientData)
{
	PAGEVIEWCTX *Ctx = (PAGEVIEWCTX*)clientData;

	if (Ctx && Ctx->Doc == doc)
	{
		AVAppUnregisterNotification(AVDocWillCloseSEL, 0, Core_OnAVDocWillClose, Ctx);

		if (Ctx->hCompositionWnd)
		{
			g_fnorg_SetWindowDisplayAffinity(Ctx->hCompositionWnd, WDA_NONE);
		}

		if (Ctx->hOverlayWnd)
		{
			send_overlay_windows_info_to_engine(Ctx->hOverlayWnd, FALSE);
		}

		EnterCriticalSection(&Global.PageViewCtxListLock);

		RemoveEntryList(&Ctx->Link);

		LeaveCriticalSection(&Global.PageViewCtxListLock);
		
		memset(Ctx, 0, sizeof(PAGEVIEWCTX));

		free(Ctx);

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);
	}
}

HRESULT WINAPI Core_MAPIInitialize(LPVOID lpMapiInit)
{
	HRESULT hr = S_OK;

	WCHAR EmailFileName[MAX_PATH] = {0};

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (can_I_send_email())
		{
			break;
		}

		EnterCriticalSection(&Global.EmailDocFileNameLock);

		memcpy(EmailFileName,
			   Global.EmailDocFileName,
			   min(sizeof(EmailFileName) - sizeof(WCHAR), wcslen(Global.EmailDocFileName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.EmailDocFileNameLock);

		send_block_notification(EmailFileName, NxrmdrvSendMailBlocked);

		return S_FALSE;

	} while (FALSE);

	return g_fn_MAPIInitialize_trampoline(lpMapiInit);
}

BOOL InitializeAdobeSendMailHooks(void)
{
	if (Global.IsAdobeReaderXI)
	{
		return InitializeAdobeSendMailHooksXI();
	}
	else
	{
		return InitializeAdobeSendMailHooksX();
	}
}

void CleanupAdobeSendMailHooks(void)
{
	if (g_fn_MAPIInitialize_trampoline)
	{
		remove_hook(g_fn_MAPIInitialize_trampoline);
		g_fn_MAPIInitialize_trampoline = NULL;
	}

	if (g_fn_AdobeXISendMail_trampoline)
	{
		remove_hook(g_fn_AdobeXISendMail_trampoline);
		g_fn_AdobeXISendMail_trampoline = NULL;
	}
}

static BOOL can_I_send_email(void)
{
	BOOL bRet = TRUE;

	QUERY_PROTECTEDMODEAPPINFO_REQUEST req = {0};
	QUERY_PROTECTEDMODEAPPINFO_RESPONSE resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		if (Global.DisableSendMail)
		{
			EnterCriticalSection(&Global.EmailDocFileNameLock);

			memset(Global.EmailDocFileName, 0 , sizeof(Global.EmailDocFileName));

			memcpy(Global.EmailDocFileName,
				   Global.ActiveDocFileName,
				   min(sizeof(Global.EmailDocFileName) - sizeof(WCHAR), wcslen(Global.ActiveDocFileName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.EmailDocFileNameLock);

			bRet = FALSE;
			break;
		}

		if (!init_rm_section_safe())
		{
			break;
		}

		req.ProcessId	= GetCurrentProcessId();
		req.ThreadId	= GetCurrentThreadId();

		ProcessIdToSessionId(req.ProcessId, &req.SessionId);

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_QUERY_PROTECTEDMODEAPPINFO, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, (PVOID)&resp, sizeof(resp), &bytesret))
		{
			Ctx = NULL;

			break;
		}

		if (resp.Ack != 0)
		{
			break;
		}

		if (!(resp.RightsMask & BUILTIN_RIGHT_SEND))
		{
			bRet = FALSE;
		}

		EnterCriticalSection(&Global.EmailDocFileNameLock);

		memset(Global.EmailDocFileName, 0 , sizeof(Global.EmailDocFileName));

		memcpy(Global.EmailDocFileName,
			   resp.ActiveDocFileName,
			   min(sizeof(Global.EmailDocFileName) - sizeof(WCHAR), wcslen(resp.ActiveDocFileName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.EmailDocFileNameLock);

	} while (FALSE);

	return bRet;
}

static BOOL update_protected_app_info(const WCHAR *ActiveDocFileName, ULONGLONG RightsMask, ULONGLONG CustomRights, ULONGLONG EvaluationId)
{
	BOOL bRet = FALSE;

	UPDATE_PROTECTEDMODEAPPINFO_REQUEST req = {0};
	UPDATE_PROTECTEDMODEAPPINFO_RESPONSE resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		req.ProcessId	= GetCurrentProcessId();
		req.ThreadId	= GetCurrentThreadId();

		ProcessIdToSessionId(req.ProcessId, &req.SessionId);

		memcpy(req.ActiveDocFileName,
			   ActiveDocFileName,
			   min(sizeof(req.ActiveDocFileName) - sizeof(WCHAR), wcslen(ActiveDocFileName)*sizeof(WCHAR)));

		req.RightsMask		= RightsMask;
		req.CustomRights	= CustomRights;
		req.EvaluationId	= EvaluationId;

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_UPDATE_PROTECTEDMODEAPPINFO, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, (PVOID)&resp, sizeof(resp), &bytesret))
		{
			Ctx = NULL;

			break;
		}

		bRet = TRUE;

	} while (FALSE);

	return bRet;
}

void __cdecl Core_OnAVDocWindowWasAdded(AVDoc doc, AVWindow window, void *clientData)
{
	PDDoc pPDDoc = NULL;
	ASFile pASFile = NULL;
	ASPathName pASPathName = NULL;
	ASPlatformPath pPlatformPath = NULL;
	WCHAR *FileName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;


	HWND hWnd = NULL;


	PAGEVIEWCTX *PageViewCtx = NULL;

	AVDevRect PageViewRect = {0};

	HRESULT hr = S_OK;

	BOOL bSendNotificationToEngine = FALSE;

	do
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		hWnd = AVWindowGetPlatformThing(window);

		if (!hWnd)
		{
			break;
		}

		if (!doc)
		{
			break;
		}

		pPDDoc = AVDocGetPDDoc(doc);

		if (!pPDDoc)
		{
			break;
		}

		pASFile = PDDocGetFile(pPDDoc);

		if (!pASFile)
		{
			break;
		}

		pASPathName = ASFileAcquirePathName(pASFile);

		if (!pASPathName)
		{
			break;
		}

		if (0 != ASFileSysAcquirePlatformPath(ASGetDefaultUnicodeFileSys(), pASPathName, ASAtomFromString("WinUnicodePath"), &pPlatformPath))
		{
			break;
		}

		FileName = ASPlatformPathGetCstringPtr(pPlatformPath);

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(FileName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
		{
			g_fnorg_SetWindowDisplayAffinity(hWnd, WDA_MONITOR);
		}

		do
		{
			//
			// take care of obligations
			//
			Obligations = query_engine(FileName, EvaluationId);

			if (!Obligations)
			{
				break;
			}

			if (Obligations->IsEmpty() || Global.IsLaunchPrinting)//ignore the watermark if it is launch for printing.
			{
				break;
			}

			OverlayOb = (Obligations->GetObligations())[OB_ID_OVERLAY];

			BitmapFileName = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_IMAGE];

			if (!BitmapFileName.length())
			{
				break;
			}

			Opacity = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_TRANSPARENCY];

			if (Opacity.length())
			{
				nudf::string::ToInt<wchar_t>(Opacity, &Global.Opacity);
			}
			else
			{
				Global.Opacity = 10;
			}

			bSendNotificationToEngine = TRUE;

		} while (FALSE);

	} while (FALSE);

	if (bSendNotificationToEngine && hWnd)
	{
		send_overlay_windows_info_to_engine(hWnd, TRUE);
	}

	if (pASPathName)
	{
		ASFileSysReleasePath(NULL, pASPathName);
		pASPathName = NULL;
	}

	if (pPlatformPath)
	{
		ASFileSysReleasePlatformPath(NULL, pPlatformPath);
		pPlatformPath = NULL;
		FileName = NULL;
	}

	return;
}

static BOOL InitializeAdobeSendMailHooksXI(void)
{
	BOOL bRet = TRUE;

	HMODULE hSendMail = NULL;

	UCHAR *p = NULL;

	UCHAR *pSendExec = NULL;

	ADOBE_XI_SENDMAIL SendMail = NULL;

	const UCHAR f[]= {0x55, 0x8b, 0xec, 0x6a, 0x01, 0xff, 0x75, 0x08, 0xe8};

	do 
	{
		hSendMail = GetModuleHandleW(SENDMAIL_MODULE_NAME);

		if (!hSendMail)
		{
			bRet = FALSE;
			break;
		}

		pSendExec = (UCHAR*)GetProcAddress(hSendMail, "SendExec");

		__try
		{
			if (pSendExec)
			{
				for (p = pSendExec; p < pSendExec + 128; p++)
				{
					if (*p != 0x55)
					{
						continue;
					}

					if (memcmp(p, f, sizeof(f)) != 0)
					{
						continue;
					}

					SendMail = (ADOBE_XI_SENDMAIL)p;

					break;
				}

			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			bRet = FALSE;
			break;
		}

		if (!SendMail)
		{
			bRet = FALSE;
			break;
		}

		EnterCriticalSection(&Global.Mapi32Lock);

		do 
		{
			if (SendMail && (g_fnorg_AdobeXISendMail != SendMail))
			{
				if (!install_hook(SendMail, (PVOID*)&g_fn_AdobeXISendMail_trampoline, Core_AdobeXISendMail))
				{
					bRet = FALSE;
					break;
				}
			}

			g_fnorg_AdobeXISendMail = SendMail;

			//
			// CAN'T set AdobeSendMailHooksInitialized to TRUE to take care of re-base
			// 
			Global.AdobeSendMailHooksInitialized = FALSE;

		} while (FALSE);

		LeaveCriticalSection(&Global.Mapi32Lock);

	} while (FALSE);

	return bRet;
}

static BOOL InitializeAdobeSendMailHooksX(void)
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

		EnterCriticalSection(&Global.Mapi32Lock);

		do 
		{
			if (Global.AdobeSendMailHooksInitialized)
			{
				break;
			}

			g_fnorg_MAPIInitialize = (ADOBE_MAPIINITIALIZE)GetProcAddress(Global.Mapi32Handle, ADOBE_MAPIINITIALIZE_PROC_NAME);

			if (g_fnorg_MAPIInitialize)
			{
				if (!install_hook(g_fnorg_MAPIInitialize, (PVOID*)&g_fn_MAPIInitialize_trampoline, Core_MAPIInitialize))
				{
					bRet = FALSE;
					break;
				}
			}

			Global.AdobeSendMailHooksInitialized = TRUE;

		} while (FALSE);

		LeaveCriticalSection(&Global.Mapi32Lock);
		
	} while (FALSE);

	return bRet;
}

void __cdecl Core_AdobeXISendMail(ULONG p1, ULONG p2)
{
	WCHAR EmailFileName[MAX_PATH] = {0};

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (can_I_send_email())
		{
			break;
		}

		EnterCriticalSection(&Global.EmailDocFileNameLock);

		memcpy(EmailFileName,
			   Global.EmailDocFileName,
			   min(sizeof(EmailFileName) - sizeof(WCHAR), wcslen(Global.EmailDocFileName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.EmailDocFileNameLock);

		send_block_notification(EmailFileName, NxrmdrvSendMailBlocked);

		return;

	} while (FALSE);

	return g_fn_AdobeXISendMail_trampoline(p1, p2);
}

BOOL InitializeAdobeAcroWinMainSandboxHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.AcroRd32Handle)
		{
			Global.AcroRd32Handle = GetModuleHandleW(ACRORD32_MODULE_NAME);
		}

		if (!Global.AcroRd32Handle)
		{
			bRet = FALSE;
			break;
		}

		EnterCriticalSection(&Global.AcroRd32Lock);

		do 
		{
			if (Global.AcroWinMainSandboxHookInitialized)
			{
				break;
			}

			g_fnorg_AcroWinMainSandbox = (ADOBE_ACROWINMAINSANDBOX)GetProcAddress(Global.AcroRd32Handle, ADOBE_ACROWINMAINSANDBOX_PROC_NAME);

			if (g_fnorg_AcroWinMainSandbox)
			{
				if (!install_hook(g_fnorg_AcroWinMainSandbox, (PVOID*)&g_fn_AcroWinMainSandbox_trampoline, Core_AcroWinMainSandbox))
				{
					bRet = FALSE;
					break;
				}
			}

			Global.AcroWinMainSandboxHookInitialized = TRUE;

		} while (FALSE);

		LeaveCriticalSection(&Global.AcroRd32Lock);

	} while (FALSE);

	return bRet;
}

void CleanupAdobeAcroWinMainSandboxHook(void)
{
	if (g_fn_AcroWinMainSandbox_trampoline)
	{
		remove_hook(g_fn_AcroWinMainSandbox_trampoline);
		g_fn_AcroWinMainSandbox_trampoline = NULL;
	}
}

int WINAPI Core_AcroWinMainSandbox(PVOID p1, PVOID p2, PVOID p3, PVOID p4, PVOID p5)
{
	
	Global.AcroRd32ModuleChecksum = get_module_checksum(Global.AcroRd32Handle);

	ASExtensionMgrGetHFTSELPROTO	fn_ASExtensionMgrGetHFT = NULL;
	ASAtomFromStringSELPROTO		fn_ASAtomFromString = NULL;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (!query_saved_adobe_hft_functions((PVOID*)&fn_ASExtensionMgrGetHFT, (PVOID*)&fn_ASAtomFromString))
		{
			break;
		}

		g_fnorg_ASExtensionMgrGetHFT = fn_ASExtensionMgrGetHFT;
		g_fnorg_ASAtomFromString = fn_ASAtomFromString;

		if (g_fnorg_ASExtensionMgrGetHFT)
		{
			install_hook(g_fnorg_ASExtensionMgrGetHFT, (PVOID*)&g_fn_ASExtensionMgrGetHFT_trampoline, Core_ASExtensionMgrGetHFT);
		}

	} while (FALSE);
	
	return g_fn_AcroWinMainSandbox_trampoline(p1, p2, p3, p4, p5);
}

static BOOL query_saved_adobe_hft_functions(PVOID *ppASExtensionMgrGetHFT, PVOID *ppASAtomFromString)
{
	BOOL bRet = FALSE;

	ULONGLONG Offset1 = 0ULL;
	ULONGLONG Offset2 = 0ULL;

	QUERY_CORE_CTX_REQUEST req = { 0 };

	QUERY_CORE_CTX_RESPONSE resp = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do
	{
		req.ProcessId		= GetCurrentProcessId();
		req.ThreadId		= GetCurrentThreadId();
		req.SessionId		= Global.SessionId;
		req.ModuleChecksum	= Global.AcroRd32ModuleChecksum;

		memcpy(req.ModuleFullPath,
			   Global.ProcessName,
			   min(sizeof(req.ModuleFullPath) - sizeof(WCHAR), wcslen(Global.ProcessName) * sizeof(WCHAR)));

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_QUERY_CORE_CTX, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, (PVOID)&resp, sizeof(resp), &bytesret))
		{
			Ctx = NULL;

			break;
		}

		if (resp.ModuleChecksum == Global.AcroRd32ModuleChecksum)
		{
			Offset1 = resp.CtxData[0];
			Offset2 = resp.CtxData[1];
		}

		if (Offset1 && Offset2)
		{
			*ppASExtensionMgrGetHFT = (PVOID)((ULONG_PTR)Global.AcroRd32Handle + (ULONG_PTR)Offset1);
			*ppASAtomFromString = (PVOID)((ULONG_PTR)Global.AcroRd32Handle + (ULONG_PTR)Offset2);

			bRet = TRUE;
		}

	} while (FALSE);

	return bRet;
}

static void update_adobe_hft_functions(PVOID pASExtensionMgrGetHFT, PVOID pASAtomFromString)
{
	ULONGLONG Offset = 0ULL;

	UPDATE_CORE_CTX_REQUEST req = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do
	{
		req.ProcessId		= GetCurrentProcessId();
		req.ThreadId		= GetCurrentThreadId();
		req.SessionId		= Global.SessionId;
		req.ModuleChecksum	= Global.AcroRd32ModuleChecksum;
		req.CtxData[0]		= (ULONGLONG)((ULONG_PTR)pASExtensionMgrGetHFT - (ULONG_PTR)Global.AcroRd32Handle);
		req.CtxData[1]		= (ULONGLONG)((ULONG_PTR)pASAtomFromString - (ULONG_PTR)Global.AcroRd32Handle);

		memcpy(req.ModuleFullPath,
			   Global.ProcessName,
			   min(sizeof(req.ModuleFullPath) - sizeof(WCHAR), wcslen(Global.ProcessName) * sizeof(WCHAR)));

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_UPDATE_CORE_CTX, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, NULL, 0, &bytesret))
		{
			Ctx = NULL;

			break;
		}

	} while (FALSE);

	return;
}

HFT __cdecl Core_ASExtensionMgrGetHFT(USHORT name, UINT version)
{
	ULONG AcroHFTInitialized = 0;

	size_t i = 0;
	USHORT bSuccess = 0;

	HMODULE hUser32 = NULL;
	HMODULE hOle32 = NULL;

	HFTINFO hftInfo[] = {
		{ "Core", PI_CORE_VERSION, &g_CoreVersion, &g_CoreHFT, false },
		{ "PDModel", PI_PDMODEL_VERSION, &g_PDModelVersion, &g_PDModelHFT, false },
		{ "AcroView", PI_ACROVIEW_VERSION, &g_AcroViewVersion, &g_AcroViewHFT, false },
		{ "Win", PI_WIN_VERSION, &g_WinVersion, &g_WinHFT, false },
	};

	do 
	{
		AcroHFTInitialized = InterlockedCompareExchange(&Global.AcroHFTInitialized, 1, 0);

		if (AcroHFTInitialized)
		{
			break;
		}

		g_AcroSupportHFT	= g_fn_ASExtensionMgrGetHFT_trampoline(g_fnorg_ASAtomFromString("AcroSupport"), ASCallsHFT_VERSION_6);
		g_CoreHFT			= g_fn_ASExtensionMgrGetHFT_trampoline(g_fnorg_ASAtomFromString("Core"), PI_CORE_VERSION);

		if (!g_AcroSupportHFT ||
			!g_CoreHFT)
		{
			break;
		}
		
		g_CoreVersion = PI_CORE_VERSION;
		g_CoreVersion = HFTGetVersion(g_CoreHFT);

		g_AcroSupportVersion = ASCallsHFT_VERSION_6;
		g_AcroSupportVersion = HFTGetVersion(g_AcroSupportHFT);

		bSuccess = GetRequestedHFT("AcroSupport", PI_ACROSUPPORT_VERSION, &g_AcroSupportVersion, &g_AcroSupportHFT);

		for (i = 0; i < sizeof(hftInfo) / sizeof(hftInfo[0]); i++)
		{
			bSuccess = GetRequestedHFT(hftInfo[i].name, hftInfo[i].ver, hftInfo[i].retver, hftInfo[i].hft);

			if (!bSuccess && !hftInfo[i].optional)
			{
				bSuccess = FALSE;
				break;
			}
		}

		if (!bSuccess)
		{
			g_AcroSupportHFT = NULL;
			g_AcroSupportVersion = 0;

			for (i = 0; i < sizeof(hftInfo) / sizeof(hftInfo[0]); i++)
			{
				hftInfo[i].hft = NULL;
			}

			InterlockedCompareExchange(&Global.AcroHFTInitialized, 0, 1);
			break;
		}

		Global.AdobeHooksInitialized = TRUE;

		//
		// AcroRd32 depends on user32 and GDI32
		// no way for hUser32 to be NULL
		//
		hUser32 = GetModuleHandleW(USER32_MODULE_NAME);


		//
		// Annots depends on OLE32
		// no way for hOle32 to be NULL
		//
		hOle32 = GetModuleHandleW(OLE32_MODULE_NAME);

		g_fnorg_SetWindowDisplayAffinity = (SETWINDOWDISPLAYAFFINITY)GetProcAddress(hUser32, NXRMCORE_SETWINDOWDISPLAYAFFINITY_PROC_NAME);

		g_fnorg_GetParent = (GETPARENT)GetProcAddress(hUser32, NXRMCORE_GETPARENT_PROC_NAME);

		g_fnorg_GetClientRect = (GETCLIENTRECT)GetProcAddress(hUser32, NXRMCORE_GETCLIENTRECT_PROC_NAME);

		g_fnorg_CoCreateInstance = (ADOBE_COCREATEINSTANCE)GetProcAddress(hOle32, ADOBE_COCREATEINSTANCE_PROC_NAME);

		Core_PluginInit();

	} while (FALSE);

	return g_fn_ASExtensionMgrGetHFT_trampoline(name, version);
}