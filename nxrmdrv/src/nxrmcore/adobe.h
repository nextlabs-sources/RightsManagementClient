#pragma once

#define ADOBE_PLUGINMAIN_PROC_NAME			"PlugInMain"

#define ADOBE_COCREATEINSTANCE_PROC_NAME	"CoCreateInstance"

#define ADOBE_MAPIINITIALIZE_PROC_NAME		"MAPIInitialize"

#define ADOBE_ACROWINMAINSANDBOX_PROC_NAME	"AcroWinMainSandbox"

#define ADOBE_PROTECTEDCHILD_CMD_LINE		L" --channel="

#define ADOBE_IEMODE_CMD_LINE				L" /o /eo /l /b /id"
#define ADOBE_PRINT_CMD_LINE				L" /p "

#define ADOBE_MODE_PROTECTEDCHILD			0x40404040
#define ADOBE_MODE_PARENT					0x44444444

#define ASCallsHFT_VERSION_6			0x00060000
#define ASCallsHFT_VERSION_8			0x00080000
#define CosHFT_VERSION_6				0x00060000
#define CoreHFT_VERSION_2				0x00020000
#define CoreHFT_VERSION_4				0x00040000
#define CoreHFT_VERSION_5				0x00050000  /* note: version 5 is same as 4.  Provided to support old plugins that erroneously required 5 */
#define PDModelHFT_VERSION_6			0x00060000
#define AcroViewHFT_VERSION_6			0x00060000
#define ASExtraHFT_VERSION_6			0x00060000
#define PDFEditWriteHFT_VERSION_6		0x00060000
#define PDFEditReadHFT_VERSION_6		0x00060000
#define PDModelHFT_VERSION_2			0x00020000
#define PDSysFontHFT_VERSION_4			0x00040000
#define PagePDEContentHFT_VERSION_6		0x00060000
#define PDSWrite_VERSION_6				0x00060000
#define PDSRead_VERSION_6				0x00060000
#define WINHFT_VERSION_5				0x00050000
#define WINHFT_VERSION_8				0x00080000
#define PDMetadataHFT_VERSION_6			0x00060000
#define AcroViewHFT_VERSION_2			0x00020000

#define HANDSHAKE_V0200				((UINT)((2L<<16) + 0))

#define PI_CORE_VERSION				CoreHFT_VERSION_5
#define HANDSHAKE_VERSION			HANDSHAKE_V0200
#define PI_ACROSUPPORT_VERSION		ASCallsHFT_VERSION_8
#define PI_COS_VERSION				CosHFT_VERSION_6
#define PI_PDMODEL_VERSION			PDModelHFT_VERSION_6
#define PI_ACROVIEW_VERSION			AcroViewHFT_VERSION_6
#define PI_ASEXTRA_VERSION			ASExtraHFT_VERSION_6
#define PI_PDFEDIT_WRITE_VERSION	PDFEditWriteHFT_VERSION_6
#define PI_PDFEDIT_READ_VERSION		PDFEditReadHFT_VERSION_6
#define PI_PDSYSFONT_VERSION		PDSysFontHFT_VERSION_4
#define PI_PAGE_PDE_CONTENT_VERSION	PagePDEContentHFT_VERSION_6
#define PI_PDSEDIT_WRITE_VERSION	PDSWrite_VERSION_6
#define PI_PDSEDIT_READ_VERSION		PDSRead_VERSION_6
#define PI_WIN_VERSION				WINHFT_VERSION_5
#define PI_PDMETADATA_VERSION       PDMetadataHFT_VERSION_6

#define HFT_ERROR_NO_VERSION (0xFFFFFFFF)

#pragma pack(push, 8)

typedef struct _t_ASExtension *ASExtension;
typedef struct _t_AVDoc *AVDoc;
typedef struct _t_AVPageView *AVPageView;
typedef struct _t_PDDoc *PDDoc;
typedef	struct _t_ASFile *ASFile;
typedef struct _t_ASFileSysRec *ASFileSys;
typedef struct _t_ASPathNameRec *ASPathName;
typedef struct _t_ASPlatformPath *ASPlatformPath;
typedef struct _t_AVWindow *AVWindow;

typedef ASExtension ExtensionID ;

typedef void *HFTEntry;

typedef HFTEntry *HFT;

typedef struct V0200_DATA_t_ {
	
	HWND 		hWnd;
	
	HINSTANCE	hInstance;

} V0200_DATA;

typedef struct {
	
	UINT				handshakeVersion;	/* IN  - Will always be HANDSHAKE_VERSION_V0200 */

	ExtensionID			extensionID;		/* IN  - Opaque to extensions, used to identify the Extension */

	HFT					coreHFT;			/* IN  - Host Function Table for "core" functions */

	void				*handshakeCallback;	/* OUT - Address of PIHandshake() */

} PISDKData_V0200;

typedef struct {
	
	UINT				handshakeVersion;					/* IN  - Will always be HANDSHAKE_V0200 */
	
	USHORT				appName;							/* IN  - Name of host application */
	
	USHORT				extensionName;						/* OUT - Name of the plug-in */
	
	void				*exportHFTsCallback;				/* OUT - Routine to register HFTs this plug-in is providing */
	
	void				*importReplaceAndRegisterCallback;	/* OUT - Routine to import other plug-in's HFTs, replace HFT functions, and register notifications */
	
	void				*initCallback;						/* OUT - Routine for plug-in to initialize itself */
	
	void				*unloadCallback;					/* OUT - Routine to unreplace, etc. the plug-in */

} PIHandshakeData_V0200;

typedef struct _t_WinPort {

	/** */
	HWND	hWnd;

	/** */
	HDC		hDC;
} WinPortRec, *WinPort;

typedef struct AVRect {

	/** */
	int left;

	/** */
	int top;

	/** */
	int right;

	/** */
	int bottom;

} AVRect, *AVRectP;

typedef AVRect AVDevRect;

enum {
	AVAppDidInitializeSEL,
	AVAppWillQuitSEL,
	AVAppFrontDocDidChangeSEL,
	AVDocWillOpenFromFileSEL,
	AVDocDidOpenSEL,
	AVDocDidActivateSEL,
	AVDocDidDeactivateSEL,
	AVDocWillCloseSEL,
	AVDocDidCloseSEL,
	AVDocWillClearSelectionSEL,
	AVDocDidSetSelectionSEL,
	AVDocWillPerformActionSEL,
	AVDocDidPerformActionSEL,
	AVPageViewDidDrawSEL,
	AVPageViewDidChangeSEL,
	PDDocWillInsertPagesSEL,
	PDDocDidInsertPagesSEL,
	PDDocWillReplacePagesSEL,
	PDDocDidReplacePagesSEL,
	PDDocWillMovePagesSEL,
	PDDocDidMovePagesSEL,
	PDDocWillDeletePagesSEL,
	PDDocDidDeletePagesSEL,
	PDDocWillChangePagesSEL,
	PDDocDidChangePagesSEL,
	PDDocDidChangeThumbsSEL,
	PDDocWillPrintPagesSEL,
	PDDocDidPrintPagesSEL,
	PDDocWillPrintPageSEL,
	PDDocDidPrintPageSEL,
	PDDocDidAddThreadSEL,
	PDDocDidRemoveThreadSEL,
	PDDocWillSaveSEL,
	PDDocDidSaveSEL,
	PDPageContentsDidChangeSEL,
	PDPageWillAddAnnotSEL,
	PDPageDidAddAnnotSEL,
	PDPageWillRemoveAnnotSEL,
	PDPageDidRemoveAnnotSEL,
	PDAnnotWasCreatedSEL,
	PDAnnotWillChangeSEL,
	PDAnnotDidChangeSEL,
	PDThreadDidChangeSEL,
	PDBookmarkWasCreatedSEL,
	PDBookmarkDidChangePositionSEL,
	PDBookmarkWillChangeSEL,
	PDBookmarkDidChangeSEL,
	PDBookmarkWillDestroySEL,
	PDBookmarkDidDestroySEL,
	PDDocWillRemoveThreadSEL,
	PDPageContentsDidChangeExSEL,
	AVDocWillOpenFromPDDocSEL,
	AVDocDidAddToSelectionSEL,
	AVDocDidRemoveFromSelectionSEL,
	AVDocWantsToDieSEL,
	PDDocWillPrintDocSEL,
	PDDocWillSaveExSEL,
	AVDocWillPrintSEL,
	AVDocDidPrintSEL,
	PDDocWillCloseSEL,
	PDDocPageLabelDidChangeSEL,
	AVDocWindowWasAddedSEL = 119
};

typedef struct _PAGEVIEWCTX
{
	AVDevRect				LastRect;

	HWND					hWnd;

	HWND					hCompositionWnd;

	HWND					hOverlayWnd;

	AVDoc					Doc;

	LIST_ENTRY				Link;

	ULONG					InvalidCount;

}PAGEVIEWCTX, *PPAGEVIEWCTX;

#pragma pack(pop)

typedef struct {const char* name; UINT ver; UINT* retver; HFT* hft; USHORT optional; } HFTINFO;

//////////////////////////////////////////////////////////////////////////
//
// AS function table
//
//////////////////////////////////////////////////////////////////////////

#define ASExtensionMgrGetHFTSEL		10
#define ASAtomFromStringSEL			5
#define HFTGetVersionSEL			103

typedef HFT (__cdecl *ASExtensionMgrGetHFTSELPROTO)(USHORT name, UINT version);
typedef USHORT (__cdecl *ASAtomFromStringSELPROTO)(const char *nameStr);
typedef UINT (__cdecl *HFTGetVersionSELPROTO)(HFT hft);

#define ASExtensionMgrGetHFT (_ASSERT(g_CoreVersion >= CoreHFT_VERSION_2), *((ASExtensionMgrGetHFTSELPROTO)(g_CoreHFT[ASExtensionMgrGetHFTSEL])))
#define ASAtomFromString (_ASSERT(g_CoreVersion >= CoreHFT_VERSION_2), *((ASAtomFromStringSELPROTO)(g_CoreHFT[ASAtomFromStringSEL])))

#define HFTGetVersion (_ASSERT(ASCallsHFT_VERSION_6), *((HFTGetVersionSELPROTO)(g_AcroSupportHFT[HFTGetVersionSEL])))

//////////////////////////////////////////////////////////////////////////
//
// AV function table
//
//////////////////////////////////////////////////////////////////////////

#define AVAppGetActiveDocSEL					12
#define AVAppRegisterNotificationSEL			41
#define AVAppUnregisterNotificationSEL			42
#define AVDocGetPDDocSEL						48
#define AVDocGetPageViewSEL						49
#define AVPageViewGetAVDocSEL					107
#define AVPageViewAcquireMachinePortSEL			148
#define AVPageViewReleaseMachinePortSEL			149
#define AVWindowShowSEL							187
#define AVWindowGetPlatformThingSEL				190
#define AVAppRegisterForPageViewDrawingSEL		397
#define AVPageViewGetApertureSEL				406
#define AVAppUnregisterForPageViewDrawingExSEL	545

/* declare a routine name to require version >= level.  Routine is indexed from HFT */
#define AVROUTINE(level, name) (_ASSERT(g_AcroViewVersion >=level), *((name##SELPROTO)(g_AcroViewHFT[name##SEL])))

typedef void (__cdecl *AVPageViewDrawProc)(AVPageView pageView, AVDevRect* updateRect, void*data);

typedef void (__cdecl *AVAppRegisterNotificationSELPROTO)(int nsel, ASExtension owner, void *proc, void *clientData);

typedef void (__cdecl *AVAppUnregisterNotificationSELPROTO)(int nsel, ASExtension owner, void *proc, void *clientData);

typedef AVPageView (__cdecl *AVDocGetPageViewSELPROTO)(AVDoc doc);

typedef AVDoc(__cdecl *AVPageViewGetAVDocSELPROTO)(AVPageView pageView);

typedef void* (__cdecl *AVPageViewAcquireMachinePortSELPROTO)(AVPageView pageView);

typedef void (__cdecl *AVPageViewReleaseMachinePortSELPROTO)(AVPageView pageView, void* port); 

typedef AVDoc (__cdecl *AVAppGetActiveDocSELPROTO)(void);

typedef PDDoc (__cdecl *AVDocGetPDDocSELPROTO)(AVDoc avDoc);

typedef void (__cdecl *AVAppRegisterForPageViewDrawingSELPROTO)(AVPageViewDrawProc proc, void* data);

typedef void (__cdecl *AVAppUnregisterForPageViewDrawingExSELPROTO)(AVPageViewDrawProc proc, void* data);

typedef void (__cdecl *AVPageViewGetApertureSELPROTO)(AVPageView pageView, AVDevRect *rect);

typedef HWND (__cdecl *AVWindowGetPlatformThingSELPROTO)(AVWindow win);

typedef void (__cdecl *AVWindowShowSELPROTO)(AVWindow win);

#define AVAppRegisterNotification AVROUTINE(AcroViewHFT_VERSION_2, AVAppRegisterNotification)
#define AVAppUnregisterNotification AVROUTINE(AcroViewHFT_VERSION_2, AVAppUnregisterNotification)
#define AVDocGetPageView AVROUTINE(AcroViewHFT_VERSION_2, AVDocGetPageView)
#define AVPageViewGetAVDoc AVROUTINE(AcroViewHFT_VERSION_2, AVPageViewGetAVDoc)
#define AVPageViewAcquireMachinePort AVROUTINE(AcroViewHFT_VERSION_2, AVPageViewAcquireMachinePort)
#define AVPageViewReleaseMachinePort AVROUTINE(AcroViewHFT_VERSION_2, AVPageViewReleaseMachinePort)
#define AVAppGetActiveDoc AVROUTINE(AcroViewHFT_VERSION_2, AVAppGetActiveDoc)
#define AVAppRegisterForPageViewDrawing AVROUTINE(AcroViewHFT_VERSION_6, AVAppRegisterForPageViewDrawing)
#define AVAppUnregisterForPageViewDrawingEx AVROUTINE(AcroViewHFT_VERSION_6, AVAppUnregisterForPageViewDrawingEx)
#define AVDocGetPDDoc AVROUTINE(AcroViewHFT_VERSION_2, AVDocGetPDDoc)
#define AVPageViewGetAperture AVROUTINE(AcroViewHFT_VERSION_6, AVPageViewGetAperture)
#define AVWindowGetPlatformThing AVROUTINE(AcroViewHFT_VERSION_2, AVWindowGetPlatformThing)
#define AVWindowShow AVROUTINE(AcroViewHFT_VERSION_2, AVWindowShow)

//////////////////////////////////////////////////////////////////////////
//
// PD Layer
//
//////////////////////////////////////////////////////////////////////////

#define PDDocGetFileSEL			79

typedef ASFile (__cdecl *PDDocGetFileSELPROTO)(PDDoc doc);

#define PDDocGetFile (_ASSERT(g_PDModelVersion >= PDModelHFT_VERSION_2), *((PDDocGetFileSELPROTO)(g_PDModelHFT[PDDocGetFileSEL])))

//////////////////////////////////////////////////////////////////////////
//
// AS Layer
//
//////////////////////////////////////////////////////////////////////////

#define ASFileSysReleasePathSEL			25
#define ASFileAcquirePathNameSEL		37
#define ASFileGetFileSysSEL				38
#define ASFileSysAcquirePlatformPathSEL	90
#define ASFileSysReleasePlatformPathSEL	91
#define ASPlatformPathGetCstringPtrSEL	92
#define ASGetDefaultUnicodeFileSysSEL	120

typedef ASFileSys (__cdecl *ASFileGetFileSysSELPROTO)(ASFile aFile);
typedef ASPathName (__cdecl *ASFileAcquirePathNameSELPROTO)(ASFile aFile);
typedef void (__cdecl *ASFileSysReleasePathSELPROTO)(ASFileSys fileSys, ASPathName pathName);
typedef int (__cdecl *ASFileSysAcquirePlatformPathSELPROTO)(ASFileSys fileSys, ASPathName path, USHORT platformPathType, ASPlatformPath *platformPath);
typedef void (__cdecl *ASFileSysReleasePlatformPathSELPROTO)(ASFileSys fileSys, ASPlatformPath platformPath);
typedef ASFileSys (__cdecl *ASGetDefaultUnicodeFileSysSELPROTO)(void);
typedef WCHAR * (__cdecl *ASPlatformPathGetCstringPtrSELPROTO)(ASPlatformPath path);
#define ASFileGetFileSys (_ASSERT(g_AcroSupportVersion >= 0), *((ASFileGetFileSysSELPROTO)(g_AcroSupportHFT[ASFileGetFileSysSEL])))
#define ASFileAcquirePathName (_ASSERT(g_AcroSupportVersion >= 0), *((ASFileAcquirePathNameSELPROTO)(g_AcroSupportHFT[ASFileAcquirePathNameSEL])))
#define ASFileSysReleasePath (_ASSERT(g_AcroSupportVersion >= 0), *((ASFileSysReleasePathSELPROTO)(g_AcroSupportHFT[ASFileSysReleasePathSEL])))
#define ASFileSysAcquirePlatformPath (_ASSERT(g_AcroSupportVersion >= ASCallsHFT_VERSION_6), *((ASFileSysAcquirePlatformPathSELPROTO)(g_AcroSupportHFT[ASFileSysAcquirePlatformPathSEL])))
#define ASFileSysReleasePlatformPath (_ASSERT(g_AcroSupportVersion >= ASCallsHFT_VERSION_6), *((ASFileSysReleasePlatformPathSELPROTO)(g_AcroSupportHFT[ASFileSysReleasePlatformPathSEL])))
#define ASGetDefaultUnicodeFileSys (_ASSERT(g_AcroSupportVersion >= ASCallsHFT_VERSION_8), *((ASGetDefaultUnicodeFileSysSELPROTO)(g_AcroSupportHFT[ASGetDefaultUnicodeFileSysSEL])))
#define ASPlatformPathGetCstringPtr (_ASSERT(g_AcroSupportVersion >= ASCallsHFT_VERSION_6),*((ASPlatformPathGetCstringPtrSELPROTO)(g_AcroSupportHFT[ASPlatformPathGetCstringPtrSEL])))

//////////////////////////////////////////////////////////////////////////
//
// Win function table
//
//////////////////////////////////////////////////////////////////////////

#define WinAppGetModelessParentSEL 9

#define WINROUTINE(level, name) (_ASSERT(g_AcroViewVersion >=level), *((name##SELPROTO)(g_WinHFT[name##SEL])))

typedef HWND (__cdecl *WinAppGetModelessParentSELPROTO)(void);

#define WinAppGetModelessParent WINROUTINE(WINHFT_VERSION_8, WinAppGetModelessParent)


//////////////////////////////////////////////////////////////////////////
//
// Init callback
//
//////////////////////////////////////////////////////////////////////////

typedef USHORT (__cdecl *PISetupSDKProcType)(UINT handshakeVersion, void *sdkData);
typedef USHORT (__cdecl *PIHandshakeProcType)(UINT handshakeVersion, void *handshakeData);
typedef USHORT (__cdecl *PIExportHFTsProcType)(void);
typedef USHORT (__cdecl *PIImportReplaceAndRegisterProcType)(void);
typedef USHORT (__cdecl *PIInitProcType)(void);
typedef USHORT (__cdecl *PIUnloadProcType)(void);

typedef USHORT (WINAPI *PLUGINMAIN)(UINT appHandshakeVersion, UINT *handshakeVersion, PISetupSDKProcType *setupProc, void *windowsData);

//////////////////////////////////////////////////////////////////////////
//
// Event callback
//
//////////////////////////////////////////////////////////////////////////

void __cdecl Core_OnAVAppFrontDocDidChange(AVDoc doc, void *clientData);

void __cdecl Core_OnAVDocDidOpen(AVDoc doc, int error, void *clientdata);

void __cdecl Core_OnAVPageViewDraw(AVPageView pageView, AVDevRect* updateRect, void*data);

void __cdecl Core_OnAVDocWillClose(AVDoc doc, void *clientData);

void __cdecl Core_OnAVDocWindowWasAdded(AVDoc doc, AVWindow window, void *clientData);

USHORT WINAPI Core_PlugInMain(UINT appHandshakeVersion, UINT *handshakeVersion, PISetupSDKProcType *setupProc, void *windowsData);

USHORT __cdecl Core_PISetupSDK(UINT handshakeVersion, void *sdkData);

USHORT __cdecl Core_PIHandshake(UINT handshakeVersion, void *handshakeData);

USHORT __cdecl Core_PluginInit(void);

//////////////////////////////////////////////////////////////////////////
//
// Other functions
//
//////////////////////////////////////////////////////////////////////////

typedef HRESULT (WINAPI *ADOBE_COCREATEINSTANCE)(
	_In_ REFCLSID																		rclsid,
	_In_opt_ LPUNKNOWN																	pUnkOuter,
	_In_ DWORD																			dwClsContext,
	_In_ REFIID																			riid,
	_COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID FAR	*ppv
	);

BOOL InitializeAdobeReaderHook(void);

void CleanupAdobeReaderHook(void);

typedef HRESULT (WINAPI *ADOBE_MAPIINITIALIZE)(
	LPVOID lpMapiInit
	);

HRESULT WINAPI Core_MAPIInitialize(LPVOID lpMapiInit);

typedef void (__cdecl *ADOBE_XI_SENDMAIL)(ULONG p1, ULONG p2);

void __cdecl Core_AdobeXISendMail(ULONG p1, ULONG p2);

BOOL InitializeAdobeSendMailHooks(void);
void CleanupAdobeSendMailHooks(void);

typedef int (WINAPI *ADOBE_ACROWINMAINSANDBOX)(PVOID p1, PVOID p2, PVOID p3, PVOID p4, PVOID p5);

int WINAPI Core_AcroWinMainSandbox(PVOID p1, PVOID p2, PVOID p3, PVOID p4, PVOID p5);

BOOL InitializeAdobeAcroWinMainSandboxHook(void);
void CleanupAdobeAcroWinMainSandboxHook(void);

HFT __cdecl Core_ASExtensionMgrGetHFT(USHORT name, UINT version);