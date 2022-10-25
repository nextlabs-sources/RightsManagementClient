#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "office.h"
#include "powerpointevents.h"
#include "wordevents.h"
#include "excelevents.h"
#include "filedialog.h"
#include "screenoverlay.h"
#include "dde.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA		Global;

	SETWINDOWDISPLAYAFFINITY	g_fnorg_SetWindowDisplayAffinity = NULL;

	GETWINDOWDISPLAYAFFINITY	g_fnorg_GetWindowDisplayAffinity = NULL;

	GETGUITHREADINFO			g_fnorg_GetGUIThreadInfo = NULL;

	SYSFREESTRING				g_fnorg_SysFreeString = NULL;

	SYSALLOCSTRING				g_fnorg_SysAllocString = NULL;

	extern GETDC				g_fnorg_GetDC;

	extern RELEASEDC			g_fnorg_ReleaseDC;

	extern GETCLIENTRECT		g_fnorg_GetClientRect;

	extern INVALIDATERECT		g_fnorg_InvalidateRect;

	extern ISWINDOWVISIBLE		g_fnorg_IsWindowVisible;

	extern ID2D1BitmapBrush*	build_bitmap_brush(ID2D1DCRenderTarget *pRT, const WCHAR *FileName);

	nudf::util::CObligations*	query_engine(const WCHAR *FileName, ULONGLONG EvaluationId);

	BOOL						send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

	void						print_guid(const GUID &id);

	BOOL						Office_Update_WaterMarked_Document_List(HWND hWnd);

	BOOL						Office_Refresh_And_Remove_WaterMarked_Document_List(void);

	BOOL						send_overlay_windows_info_to_engine(HWND hWnd, BOOL Add);

#ifdef __cplusplus
}
#endif

static OFFICE_COCREATEINSTANCE			g_fnorg_CoCreateInstance = NULL;
static OFFICE_COCREATEINSTANCE			g_fn_CoCreateInstance_trampoline = NULL;
static OFFICE_COCREATEINSTANCEEX		g_fnorg_CoCreateInstanceEx = NULL;
static OFFICE_COCREATEINSTANCEEX		g_fn_CoCreateInstanceEx_trampoline = NULL;
static D2D1CREATEFACTORY				g_fnorg_D2D1CreateFactory = NULL;
static ENDPAINT							g_fnorg_EndPaint = NULL;
static ENDPAINT							g_fn_EndPaint_trampoline = NULL;
static LPMAPISENDMAIL					g_fnorg_MAPISendMail = NULL;
static LPMAPISENDMAIL					g_fn_MAPISendMail_trampoline = NULL;
static LPMAPISENDMAILW					g_fnorg_MAPISendMailW = NULL;
static LPMAPISENDMAILW					g_fn_MAPISendMailW_trampoline = NULL;
static OFFICE_STGOPENSTORAGE			g_fnorg_StgOpenStorage = NULL;
static OFFICE_STGOPENSTORAGE			g_fn_StgOpenStorage_trampoline = NULL;
static OFFICE_GETFILESIZEEX				g_fnorg_GetFileSizeEx = NULL;
static OFFICE_GETFILESIZEEX				g_fn_GetFileSizeEx_trampoline = NULL;
static OFFICE_GETFILEATTRIBUTESW		g_fnorg_GetFileAttributesW = NULL;
static OFFICE_GETFILEATTRIBUTESW		g_fn_GetFileAttributesW_trampoline = NULL;
static OFFICE_UNPACKDDELPARAM			g_fnorg_UnpackDDElParam = NULL;
static OFFICE_UNPACKDDELPARAM			g_fn_UnpackDDElParam_trampoline = NULL;
static EXCEL2013_COPYWORKSHEET			g_fnorg_Excel2013_CopyWorkSheet = NULL;
static EXCEL2013_COPYWORKSHEET			g_fn_Excel2013_CopyWorkSheet_trampoline = NULL;

static const IID IID_IDTExtensibility2		= OFFICE_IDTEXTENSIBILITY2_IID;
static const ULONG_PTR COCREATEINSTANCEHOOKED	= 0x10101010;

DECLSPEC_CACHEALIGN	static ULONG g_uTLSIndex = TLS_OUT_OF_INDEXES;

static BOOL attach_to_powerpoint(void);
static BOOL attach_to_winword(void);
static BOOL attach_to_excel(void);

static BOOL takecare_excel_active_wb_if_any(void);
static BOOL takecare_excel2010_active_wb_if_any(void);

static ULONGLONG find_excel2013_copy_worksheet_function(PVOID pbase);
static ULONGLONG query_saved_excel2013_copy_worksheet_function(void);
static void update_excel2013_copy_worksheet_function(void);

static BOOL InitializeOfficeHookPreWin8(void);
static BOOL InitializeOfficeHookWin8(void);

extern "C" const struct IDTExtensibility2Vtbl	g_lpIDTExtensibility2Vtbl = {CoreIDTExtensibility2_QueryInterface,
																			 CoreIDTExtensibility2_AddRef,
																			 CoreIDTExtensibility2_Release,
																			 CoreIDTExtensibility2_GetTypeInfoCount,
																			 CoreIDTExtensibility2_GetTypeInfo,
																			 CoreIDTExtensibility2_GetIDsOfNames,
																			 CoreIDTExtensibility2_Invoke,
																			 CoreIDTExtensibility2_OnConnection,
																			 CoreIDTExtensibility2_OnDisconnection,
																			 CoreIDTExtensibility2_OnAddInsUpdate,
																			 CoreIDTExtensibility2_OnStartupComplete,
																			 CoreIDTExtensibility2_OnBeginShutdown};



static BOOL InitializeOfficeHookPreWin8(void)
{
	BOOL bRet = TRUE;

	HMODULE hUser32 = NULL;
	HMODULE hGDI32 = NULL;
	HMODULE hKernel32 = NULL;

	do 
	{
		if (Global.Ole32Handle == NULL)
		{
			Global.Ole32Handle = GetModuleHandleW(OLE32_MODULE_NAME);
		}

		if (Global.Ole32Handle == NULL)
		{
			bRet = FALSE;
			break;
		}

		if (Global.OfficeHooksInitialized)
		{
			break;
		}

		g_fnorg_CoCreateInstanceEx = (OFFICE_COCREATEINSTANCEEX)GetProcAddress(Global.Ole32Handle, OFFICE_COCREATEINSTANCEEX_PROC_NAME);

		if(g_fnorg_CoCreateInstanceEx)
		{
			if(!install_hook(g_fnorg_CoCreateInstanceEx, (PVOID*)&g_fn_CoCreateInstanceEx_trampoline, Core_Office_CoCreateInstanceEx))
			{
				bRet = FALSE;
				break;
			}
		}

#ifndef _AMD64_

		if (Global.IsOffice365AppV)
		{
			UCHAR first_instruction = 0;
			DWORD offset = 0;

			g_fnorg_CoCreateInstance = (OFFICE_COCREATEINSTANCE)GetProcAddress(Global.Ole32Handle, OFFICE_COCREATEINSTANCE_PROC_NAME);

			if(g_fnorg_CoCreateInstance)
			{
				first_instruction = *(UCHAR*)(g_fnorg_CoCreateInstance);

				if (first_instruction == 0xe9)
				{
					offset = *(DWORD*)((BYTE*)g_fnorg_CoCreateInstance + 1);

					g_fnorg_CoCreateInstance = (OFFICE_COCREATEINSTANCE)((UCHAR*)g_fnorg_CoCreateInstance + 5 + offset);
				}

				if(!install_hook(g_fnorg_CoCreateInstance, (PVOID*)&g_fn_CoCreateInstance_trampoline, Core_Office_CoCreateInstance))
				{
					bRet = FALSE;
					break;
				}
			}
		}
		else
#endif
		{
			g_fnorg_CoCreateInstance = (OFFICE_COCREATEINSTANCE)GetProcAddress(Global.Ole32Handle, OFFICE_COCREATEINSTANCE_PROC_NAME);

			if(g_fnorg_CoCreateInstance)
			{
				if(!install_hook(g_fnorg_CoCreateInstance, (PVOID*)&g_fn_CoCreateInstance_trampoline, Core_Office_CoCreateInstance))
				{
					bRet = FALSE;
					break;
				}
			}
		}

		//
		// allocate TLS index
		//
		g_uTLSIndex = TlsAlloc();

		if (Global.OfficeMode == OFFICE_MODE_COMPLUS)
		{
			g_fnorg_StgOpenStorage = (OFFICE_STGOPENSTORAGE)GetProcAddress(Global.Ole32Handle, OFFICE_STGOPENSTORAGE_PROC_NAME);

			if (g_fnorg_StgOpenStorage)
			{
				if (!install_hook(g_fnorg_StgOpenStorage, (PVOID*)&g_fn_StgOpenStorage_trampoline, Core_StgOpenStorage))
				{
					bRet = FALSE;
					break;
				}
			}

			if (Global.IsPowerPoint)
			{
				hKernel32 = GetModuleHandleW(KERNELBASE_MODULE_NAME);

				g_fnorg_GetFileSizeEx = (OFFICE_GETFILESIZEEX)GetProcAddress(hKernel32, OFFICE_GETFILESIZEEX_PROC_NAME);

				if (g_fnorg_GetFileSizeEx)
				{
					if (!install_hook(g_fnorg_GetFileSizeEx, (PVOID*)&g_fn_GetFileSizeEx_trampoline, Core_GetFileSizeEx))
					{
						bRet = FALSE;
						break;
					}
				}
			}
		}

		hKernel32 = GetModuleHandleW(KERNELBASE_MODULE_NAME);
		hUser32 = GetModuleHandleW(USER32_MODULE_NAME);
		hGDI32 = GetModuleHandleW(GDI32_MODULE_NAME);

		//
		// ole32 depends on user32, GDI32 and Ole32Aut
		// no way for hUser32 to be NULL
		//
		g_fnorg_SetWindowDisplayAffinity = (SETWINDOWDISPLAYAFFINITY)GetProcAddress(hUser32, NXRMCORE_SETWINDOWDISPLAYAFFINITY_PROC_NAME);

		g_fnorg_GetWindowDisplayAffinity = (GETWINDOWDISPLAYAFFINITY)GetProcAddress(hUser32, NXRMCORE_GETWINDOWDISPLAYAFFINITY_PROC_NAME);

		g_fnorg_GetGUIThreadInfo = (GETGUITHREADINFO)GetProcAddress(hUser32, NXRMCORE_GETGUITHREADINFO_PROC_NAME);

		//g_fnorg_GetFileAttributesW = (OFFICE_GETFILEATTRIBUTESW)GetProcAddress(hKernel32, OFFICE_GETFILEATTRIBUTESW_PROC_NAME);

		//if (g_fnorg_GetFileAttributesW)
		//{
		//	if (!install_hook(g_fnorg_GetFileAttributesW, (PVOID*)&g_fn_GetFileAttributesW_trampoline, Core_GetFileAttributesW))
		//	{
		//		bRet = FALSE;
		//		break;
		//	}
		//}

		Global.OfficeHooksInitialized = TRUE;

	} while(FALSE);

	return bRet;
}

static BOOL InitializeOfficeHookWin8(void)
{
	BOOL bRet = TRUE;
	HMODULE hUser32 = NULL;
	HMODULE hGDI32 = NULL;
	HMODULE hOle32 = NULL;
	HMODULE hKernel32 = NULL;

	do 
	{
		if (Global.CombaseHandle == NULL)
		{
			Global.CombaseHandle = GetModuleHandleW(COMBASE_MODULE_NAME);
		}

		if (Global.CombaseHandle == NULL)
		{
			bRet = FALSE;
			break;
		}

		if (Global.Ole32Handle == NULL)
		{
			Global.Ole32Handle = GetModuleHandleW(OLE32_MODULE_NAME);
		}

		if (Global.Ole32Handle == NULL)
		{
			bRet = FALSE;
			break;
		}

		if (Global.OfficeHooksInitialized)
		{
			break;
		}

		g_fnorg_CoCreateInstanceEx = (OFFICE_COCREATEINSTANCEEX)GetProcAddress(Global.CombaseHandle, OFFICE_COCREATEINSTANCEEX_PROC_NAME);


		if (g_fnorg_CoCreateInstanceEx)
		{
			if(!install_hook(g_fnorg_CoCreateInstanceEx, (PVOID*)&g_fn_CoCreateInstanceEx_trampoline, Core_Office_CoCreateInstanceEx))
			{
				bRet = FALSE;
				break;
			}
		}

		g_fnorg_CoCreateInstance = (OFFICE_COCREATEINSTANCE)GetProcAddress(Global.CombaseHandle, OFFICE_COCREATEINSTANCE_PROC_NAME);

		if (g_fnorg_CoCreateInstance)
		{
			if(!install_hook(g_fnorg_CoCreateInstance, (PVOID*)&g_fn_CoCreateInstance_trampoline, Core_Office_CoCreateInstance))
			{
				bRet = FALSE;
				break;
			}
		}

		//
		// allocate TLS index
		//
		g_uTLSIndex = TlsAlloc();

		hOle32 = GetModuleHandleW(OLE32_MODULE_NAME);

		if (Global.OfficeMode == OFFICE_MODE_COMPLUS && hOle32)
		{
			g_fnorg_StgOpenStorage = (OFFICE_STGOPENSTORAGE)GetProcAddress(hOle32, OFFICE_STGOPENSTORAGE_PROC_NAME);

			if (g_fnorg_StgOpenStorage)
			{
				if (!install_hook(g_fnorg_StgOpenStorage, (PVOID*)&g_fn_StgOpenStorage_trampoline, Core_StgOpenStorage))
				{
					bRet = FALSE;
					break;
				}
			}

			if (Global.IsPowerPoint)
			{
				hKernel32 = GetModuleHandleW(KERNELBASE_MODULE_NAME);

				g_fnorg_GetFileSizeEx = (OFFICE_GETFILESIZEEX)GetProcAddress(hKernel32, OFFICE_GETFILESIZEEX_PROC_NAME);

				if (g_fnorg_GetFileSizeEx)
				{
					if (!install_hook(g_fnorg_GetFileSizeEx, (PVOID*)&g_fn_GetFileSizeEx_trampoline, Core_GetFileSizeEx))
					{
						bRet = FALSE;
						break;
					}
				}
			}
		}
		
		hKernel32 = GetModuleHandleW(KERNELBASE_MODULE_NAME);
		hUser32 = GetModuleHandleW(USER32_MODULE_NAME);
		hGDI32 = GetModuleHandleW(GDI32_MODULE_NAME);

		//
		// ole32 depends on user32 and GDI32
		// no way for hUser32 to be NULL
		//
		g_fnorg_SetWindowDisplayAffinity = (SETWINDOWDISPLAYAFFINITY)GetProcAddress(hUser32, NXRMCORE_SETWINDOWDISPLAYAFFINITY_PROC_NAME);

		g_fnorg_GetWindowDisplayAffinity = (GETWINDOWDISPLAYAFFINITY)GetProcAddress(hUser32, NXRMCORE_GETWINDOWDISPLAYAFFINITY_PROC_NAME);

		g_fnorg_GetGUIThreadInfo = (GETGUITHREADINFO)GetProcAddress(hUser32, NXRMCORE_GETGUITHREADINFO_PROC_NAME);

		//g_fnorg_GetFileAttributesW = (OFFICE_GETFILEATTRIBUTESW)GetProcAddress(hKernel32, OFFICE_GETFILEATTRIBUTESW_PROC_NAME);

		//if (g_fnorg_GetFileAttributesW)
		//{
		//	if (!install_hook(g_fnorg_GetFileAttributesW, (PVOID*)&g_fn_GetFileAttributesW_trampoline, Core_GetFileAttributesW))
		//	{
		//		bRet = FALSE;
		//		break;
		//	}
		//}

		Global.OfficeHooksInitialized = TRUE;

	} while(FALSE);

	return bRet;
}

BOOL InitializeOfficeHook(void)
{
	if(Global.IsWin8AndAbove)
	{
		return InitializeOfficeHookWin8();
	}
	else
	{
		return InitializeOfficeHookPreWin8();
	}
}

void CleanupOfficeHook(void)
{
	if (g_fn_CoCreateInstanceEx_trampoline)
	{
		remove_hook(g_fn_CoCreateInstanceEx_trampoline);
		g_fn_CoCreateInstanceEx_trampoline = NULL;
	}

	if (g_fn_CoCreateInstance_trampoline)
	{
		remove_hook(g_fn_CoCreateInstance_trampoline);
		g_fn_CoCreateInstance_trampoline = NULL;
	}

	if (Global.IsOffice2010)
	{
		if (Global.D2D1Factory)
		{
			Global.D2D1Factory->Release();
			Global.D2D1Factory = NULL;
		}

		if (Global.D2D1Handle)
		{
			FreeLibrary(Global.D2D1Handle);
			Global.D2D1Handle = NULL;
		}

		if (g_fn_EndPaint_trampoline)
		{
			remove_hook(g_fn_EndPaint_trampoline);
			g_fn_EndPaint_trampoline = NULL;
		}
	}

	if (g_fn_StgOpenStorage_trampoline)
	{
		remove_hook(g_fn_StgOpenStorage_trampoline);
		g_fn_StgOpenStorage_trampoline = NULL;
	}

	if (g_fn_GetFileSizeEx_trampoline)
	{
		remove_hook(g_fn_GetFileSizeEx_trampoline);
		g_fn_GetFileSizeEx_trampoline = NULL;
	}

	if (g_fn_GetFileAttributesW_trampoline)
	{
		remove_hook(g_fn_GetFileAttributesW_trampoline);
		g_fn_GetFileAttributesW_trampoline = NULL;
	}

	if (g_fn_Excel2013_CopyWorkSheet_trampoline)
	{
		remove_hook(g_fn_Excel2013_CopyWorkSheet_trampoline);
		g_fn_Excel2013_CopyWorkSheet_trampoline = NULL;
	}

	Global.OfficeHooksInitialized = FALSE;
}

HRESULT WINAPI Core_Office_CoCreateInstance(
	IN REFCLSID		rclsid, 
	IN LPUNKNOWN	pUnkOuter,
	IN DWORD		dwClsContext, 
	IN REFIID		riid, 
	OUT LPVOID FAR* ppv)
{
	HRESULT hr = S_OK;

	if (g_uTLSIndex != TLS_OUT_OF_INDEXES)
	{
		TlsSetValue(g_uTLSIndex, (LPVOID)COCREATEINSTANCEHOOKED);
	}

	hr = g_fn_CoCreateInstance_trampoline(rclsid,pUnkOuter,dwClsContext,riid,ppv);

	if (g_uTLSIndex != TLS_OUT_OF_INDEXES)
	{
		TlsSetValue(g_uTLSIndex, NULL);
	}

	do 
	{
		//if (Global.OfficeMode == OFFICE_MODE_COMPLUS)
		//{
		//	break;
		//}

		if(riid == IID_IDTExtensibility2 && hr == S_OK) 
		{
			CoreIDTExtensibility2	*pIDTExtensibility2 = NULL;
			IDTExtensibility2		*pInstance = NULL;

			do 
			{

				pInstance = *(IDTExtensibility2 **)ppv;

				pIDTExtensibility2 = (CoreIDTExtensibility2*)malloc(sizeof(CoreIDTExtensibility2));

				if(!pIDTExtensibility2)
				{
					break;
				}

				pIDTExtensibility2->lpVtbl					= &g_lpIDTExtensibility2Vtbl;
				pIDTExtensibility2->m_pIDTExtensibility2	= pInstance;
				pIDTExtensibility2->m_uRefCount				= 0;

				pIDTExtensibility2->lpVtbl->AddRef((IDTExtensibility2*)pIDTExtensibility2);

				*ppv = (LPVOID)pIDTExtensibility2;

			} while (FALSE);
		}
		else if (riid == IID_IFileDialog && hr == S_OK)
		{
			CoreIFileDialog *pCoreIFileDialog = NULL;
			IFileDialog *pFileDialog = NULL;

			do 
			{
				pFileDialog = *(IFileDialog **)ppv;

				pCoreIFileDialog = new CoreIFileDialog(pFileDialog);

				*ppv = (LPVOID)pCoreIFileDialog;

			} while (FALSE);
		}

	} while (FALSE);

	return hr;
}

HRESULT WINAPI Core_Office_CoCreateInstanceEx(
	_In_ REFCLSID Clsid,
	_In_opt_ IUnknown * punkOuter,
	_In_ DWORD dwClsCtx,
	_In_opt_ COSERVERINFO * pServerInfo,
	_In_ DWORD dwCount,
	_Inout_updates_(dwCount) MULTI_QI * pResults
	)
{
	HRESULT hr = S_OK;

	IID	riid = { 0 };

	IUnknown* pv = NULL;

	BOOL bSkipProcessing = FALSE;

	hr = g_fn_CoCreateInstanceEx_trampoline(Clsid, punkOuter, dwClsCtx, pServerInfo, dwCount, pResults);

	do
	{
		if(g_uTLSIndex == TLS_OUT_OF_INDEXES)
		{
			break;
		}

		bSkipProcessing = (TlsGetValue(g_uTLSIndex) == (LPVOID)COCREATEINSTANCEHOOKED);

		if (bSkipProcessing)
		{
			break;
		}

		if (dwCount != 1 || pResults == NULL)
		{
			break;
		}

		//if (Global.OfficeMode == OFFICE_MODE_COMPLUS)
		//{
		//	break;
		//}

		riid = *(pResults->pIID);
		pv = pResults->pItf;

		if (riid == IID_IDTExtensibility2 && hr == S_OK)
		{
			CoreIDTExtensibility2	*pIDTExtensibility2 = NULL;
			IDTExtensibility2		*pInstance = NULL;

			do
			{

				pInstance = (IDTExtensibility2 *)pv;

				pIDTExtensibility2 = (CoreIDTExtensibility2*)malloc(sizeof(CoreIDTExtensibility2));

				if (!pIDTExtensibility2)
				{
					break;
				}

				pIDTExtensibility2->lpVtbl = &g_lpIDTExtensibility2Vtbl;
				pIDTExtensibility2->m_pIDTExtensibility2 = pInstance;
				pIDTExtensibility2->m_uRefCount = 0;

				pIDTExtensibility2->lpVtbl->AddRef((IDTExtensibility2*)pIDTExtensibility2);

				pResults->pItf = (IUnknown*)pIDTExtensibility2;

			} while (FALSE);
		}
		else if (riid == IID_IFileDialog && hr == S_OK)
		{
			CoreIFileDialog *pCoreIFileDialog = NULL;
			IFileDialog *pFileDialog = NULL;

			do
			{
				pFileDialog = (IFileDialog *)pv;

				pCoreIFileDialog = new CoreIFileDialog(pFileDialog);

				pResults->pItf = (IUnknown*)pCoreIFileDialog;

			} while (FALSE);
		}
	} while (FALSE);

	return hr;
}

void print_guid(const GUID &id)
{
	char buf[128];

	ULONG p1;
	ULONG p2;
	ULONG p3;

	ULONG p4[8];

	memset(buf,0,sizeof(buf));

	p1 = id.Data1;
	p2 = id.Data2;
	p3 = id.Data3;

	for(int i = 0; i < 8; i++)
	{
		p4[i] = id.Data4[i];
	}


	sprintf_s(buf,
			  sizeof(buf),
			  "%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x\n",
			  p1,
			  p2,
			  p3,
			  p4[0],
			  p4[1],
			  p4[2],
			  p4[3],
			  p4[4],
			  p4[5],
			  p4[6],
			  p4[7]);

	OutputDebugStringA(buf);
}

STDMETHODIMP CoreIDTExtensibility2_QueryInterface(IDTExtensibility2 * This, REFIID riid, void **ppvObject)
{
	HRESULT hRet = S_OK;

	IUnknown *punk = NULL;

	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	*ppvObject = NULL;

	do 
	{
		if((IID_IUnknown == riid) || (IID_IDTExtensibility2 == riid) || (IID_IDispatch == riid))
		{
			punk = (IUnknown *)This;
		}
		else
		{
			if(!p->m_pIDTExtensibility2)
			{
				hRet = E_NOINTERFACE;
				break;
			}

			hRet = p->m_pIDTExtensibility2->lpVtbl->QueryInterface(p->m_pIDTExtensibility2,riid,(void**)&punk);

			if(hRet != S_OK || punk == NULL)
			{
				hRet = E_NOINTERFACE;
				break;
			}
		}

		This->lpVtbl->AddRef(This);

		*ppvObject = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP_(ULONG) CoreIDTExtensibility2_AddRef(IDTExtensibility2 * This)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	p->m_uRefCount++;

	p->m_pIDTExtensibility2->lpVtbl->AddRef(p->m_pIDTExtensibility2);

	return p->m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreIDTExtensibility2_Release(IDTExtensibility2 * This)
{
	ULONG uCount = 0;

	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	p->m_pIDTExtensibility2->lpVtbl->Release(p->m_pIDTExtensibility2);

	if(p->m_uRefCount)
		p->m_uRefCount--;

	uCount = p->m_uRefCount;

	if(!uCount)
	{
		free(This);
	}

	return uCount;
}

STDMETHODIMP CoreIDTExtensibility2_GetTypeInfoCount(IDTExtensibility2 * This, UINT *pctinfo)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	return p->m_pIDTExtensibility2->lpVtbl->GetTypeInfoCount(p->m_pIDTExtensibility2,pctinfo);
}

STDMETHODIMP CoreIDTExtensibility2_GetTypeInfo(IDTExtensibility2 * This, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	return p->m_pIDTExtensibility2->lpVtbl->GetTypeInfo(p->m_pIDTExtensibility2,iTInfo,lcid,ppTInfo);
}

STDMETHODIMP CoreIDTExtensibility2_GetIDsOfNames(IDTExtensibility2 * This, REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	return p->m_pIDTExtensibility2->lpVtbl->GetIDsOfNames(p->m_pIDTExtensibility2,riid,rgszNames,cNames,lcid,rgDispId);
}

STDMETHODIMP CoreIDTExtensibility2_Invoke(IDTExtensibility2 * This, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	return p->m_pIDTExtensibility2->lpVtbl->Invoke(p->m_pIDTExtensibility2,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr);
}

STDMETHODIMP CoreIDTExtensibility2_OnConnection(IDTExtensibility2 * This, IDispatch * Application, enum ext_ConnectMode ConnectMode, IDispatch * AddInInst, SAFEARRAY **custom)
{
	PVOID pOfficeApp = NULL;

	IWICImagingFactory *pWICFactory = NULL;

	HRESULT hr = S_OK;

	HMODULE hShell32 = NULL;

	COMMANDLINETOARGVW fn_CommandLineToArgvW = NULL;

	LPWSTR *Argv = NULL;
	int		Argc = 0;

	HMODULE	hOle32Aut = NULL;

	SYSALLOCSTRING	fn_SysAllocString = NULL;
	SYSFREESTRING	fn_SysFreeString = NULL;

	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	if (Global.IsOffice2010)
	{
		if (!Global.D2D1Handle)
		{
			Global.D2D1Handle = LoadLibraryW(D2D1_MODULE_NAME);

			if (Global.D2D1Handle)
			{
				g_fnorg_D2D1CreateFactory = (D2D1CREATEFACTORY)GetProcAddress(Global.D2D1Handle, NXRMCORE_D2D1CREATEFACTORY_PROC_NAME);
			}
		}

		if (g_fnorg_D2D1CreateFactory && Global.D2D1Factory == NULL)
		{
			hr = g_fnorg_D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), NULL, reinterpret_cast<void**>(&Global.D2D1Factory));
		}
	}

	if (!Global.WICFactory)
	{
		hr = g_fnorg_CoCreateInstance(CLSID_WICImagingFactory,
									  NULL,
									  CLSCTX_INPROC_SERVER,
									  IID_IWICImagingFactory,
									  reinterpret_cast<void**>(&pWICFactory));

		if (SUCCEEDED(hr))
		{
			if (InterlockedCompareExchangePointer((PVOID*)&Global.WICFactory, pWICFactory, NULL))
			{
				//
				// other thread set Global.WICFactory already
				//
				pWICFactory->Release();
				pWICFactory = NULL;
			}
		}
	}

	hOle32Aut = GetModuleHandleW(OLEAUT32_MODULE_NAME);

	if (hOle32Aut)
	{
		fn_SysFreeString = (SYSFREESTRING)GetProcAddress(hOle32Aut, OFFICE_SYSFREESTRING_PROC_NAME);

		fn_SysAllocString = (SYSALLOCSTRING)GetProcAddress(hOle32Aut, OFFICE_SYSALLOCSTRING_PROC_NAME);

		InterlockedCompareExchangePointer((PVOID*)&g_fnorg_SysFreeString, fn_SysFreeString, NULL);

		InterlockedCompareExchangePointer((PVOID*)&g_fnorg_SysAllocString, fn_SysAllocString, NULL);
	}

	if (Global.IsPowerPoint)
	{
		pOfficeApp = InterlockedCompareExchangePointer((PVOID*)&Global.PowerPointApp, Application, NULL);

		if(!pOfficeApp)
		{
			do 
			{
				hShell32 = GetModuleHandleW(SHELL32_MODULE_NAME);

				if (!hShell32)
				{
					break;
				}

				fn_CommandLineToArgvW = (COMMANDLINETOARGVW)GetProcAddress(hShell32, NXRMCORE_COMMANDLINETOARGVW_PROC_NAME);

				if (fn_CommandLineToArgvW)
				{
					Argv = fn_CommandLineToArgvW(Global.OfficeCommandLine, &Argc);

					if (!Argv || Argc != 3)
					{
						break;
					}

					if (_wcsicmp(Argv[1], L"/p") == 0)
					{
						//
						// take care of print
						//
						do 
						{
							ULONGLONG RightsMask = 0;
							ULONGLONG CustomRightsMask = 0;
							ULONGLONG EvaluationId = 0;

							if (!init_rm_section_safe())
							{
								break;
							}

							//
							// take care of rights
							//
							hr = nudf::util::nxl::NxrmCheckRights(Argv[2], &RightsMask, &CustomRightsMask, &EvaluationId);

							if (FAILED(hr))
							{
								Global.DisablePrinting = FALSE;
								Global.DisableSendMail = FALSE;
								Global.DisableClipboard = FALSE;
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

							if (!(RightsMask & BUILTIN_RIGHT_SEND))
							{
								Global.DisableSendMail = TRUE;
							}
							else
							{
								Global.DisableSendMail = FALSE;
							}

							if (!(RightsMask & BUILTIN_RIGHT_CLIPBOARD))
							{
								Global.DisableClipboard = TRUE;
							}
							else
							{
								Global.DisableClipboard = FALSE;
							}

							EnterCriticalSection(&Global.ActiveDocFileNameLock);

							memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

							memcpy(Global.ActiveDocFileName,
								   Argv[2],
								   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(Argv[2])*sizeof(WCHAR)));

							LeaveCriticalSection(&Global.ActiveDocFileNameLock);

						} while (FALSE);
						 
					}

					if (_wcsicmp(Argv[1], L"/n"))
					{
						break;
					}

					Global.OfficeMode = OFFICE_MODE_DDE;

					EnterCriticalSection(&Global.DDEDocFileNameLock);

					memcpy(Global.DDEDocFileName,
						   Argv[2],
						   min(sizeof(Global.DDEDocFileName) - sizeof(WCHAR), wcslen(Argv[2])*sizeof(WCHAR)));

					LeaveCriticalSection(&Global.DDEDocFileNameLock);
				}

			} while (FALSE);

			if (Argv)
			{
				LocalFree(Argv);
				Argv = NULL;
			}

			//
			// Attach to office application here
			//
			attach_to_powerpoint();
		}
	}

	if (Global.IsWinWord)
	{
		pOfficeApp = InterlockedCompareExchangePointer((PVOID*)&Global.WordApp, Application, NULL);

		if (!pOfficeApp)
		{
			do 
			{
				hShell32 = GetModuleHandleW(SHELL32_MODULE_NAME);

				if (!hShell32)
				{
					break;
				}

				fn_CommandLineToArgvW = (COMMANDLINETOARGVW)GetProcAddress(hShell32, NXRMCORE_COMMANDLINETOARGVW_PROC_NAME);

				if (fn_CommandLineToArgvW)
				{
					Argv = fn_CommandLineToArgvW(Global.OfficeCommandLine, &Argc);

					if (!Argv || Argc != 4)
					{
						break;
					}

					if (_wcsicmp(Argv[1], L"/n") || _wcsicmp(Argv[2], L"/f"))
					{
						break;
					}

					Global.OfficeMode = OFFICE_MODE_DDE;

					EnterCriticalSection(&Global.DDEDocFileNameLock);

					memcpy(Global.DDEDocFileName,
						   Argv[3],
						   min(sizeof(Global.DDEDocFileName) - sizeof(WCHAR), wcslen(Argv[3])*sizeof(WCHAR)));

					LeaveCriticalSection(&Global.DDEDocFileNameLock);
				}

			} while (FALSE);

			if (Argv)
			{
				LocalFree(Argv);
				Argv = NULL;
			}

			//
			// Attach to office application here
			//
			attach_to_winword();
		}
	}

	if (Global.IsExcel)
	{
		pOfficeApp = InterlockedCompareExchangePointer((PVOID*)&Global.ExcelApp, Application, NULL);

		if (!pOfficeApp)
		{
			if (Global.IsOffice2010)
			{
				takecare_excel2010_active_wb_if_any();
			}
			else
			{
				takecare_excel_active_wb_if_any();
			}

			//
			// Attach to office application here
			//
			attach_to_excel();
		}

#ifndef _AMD64_
		//
		// find Excel 2013 CopyWorkSheetOffset
		//
		if (!Global.IsOffice2010 && Global.Excel2013CopyWorkSheetOffset == 0ULL)
		{
			if (init_rm_section_safe())
			{
				Global.Excel2013CopyWorkSheetOffset = query_saved_excel2013_copy_worksheet_function();

				if (!Global.Excel2013CopyWorkSheetOffset)
				{
					Global.Excel2013CopyWorkSheetOffset = find_excel2013_copy_worksheet_function(GetModuleHandleW(NULL));

					if (Global.Excel2013CopyWorkSheetOffset)
					{
						update_excel2013_copy_worksheet_function();
					}
				}
			}
			else
			{
				Global.Excel2013CopyWorkSheetOffset = find_excel2013_copy_worksheet_function(GetModuleHandleW(NULL));
			}

			if (Global.Excel2013CopyWorkSheetOffset)
			{
				g_fnorg_Excel2013_CopyWorkSheet = (EXCEL2013_COPYWORKSHEET)((ULONG_PTR)GetModuleHandleW(NULL) + Global.Excel2013CopyWorkSheetOffset);

				install_hook(g_fnorg_Excel2013_CopyWorkSheet, (PVOID*)&g_fn_Excel2013_CopyWorkSheet_trampoline, Core_Excel2013_CopyWorkSheet);
			}
		}
#endif
	}

	return p->m_pIDTExtensibility2->lpVtbl->OnConnection(p->m_pIDTExtensibility2,Application,ConnectMode,AddInInst,custom);
}

STDMETHODIMP CoreIDTExtensibility2_OnDisconnection(IDTExtensibility2 * This, enum ext_DisconnectMode RemoveMode, SAFEARRAY **custom)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	if (Global.WICFactory)
	{
		Global.WICFactory->Release();
		Global.WICFactory = NULL;
	}

	return p->m_pIDTExtensibility2->lpVtbl->OnDisconnection(p->m_pIDTExtensibility2,RemoveMode,custom);
}

STDMETHODIMP CoreIDTExtensibility2_OnAddInsUpdate(IDTExtensibility2 * This, SAFEARRAY **custom)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	return p->m_pIDTExtensibility2->lpVtbl->OnAddInsUpdate(p->m_pIDTExtensibility2,custom);
}

STDMETHODIMP CoreIDTExtensibility2_OnStartupComplete(IDTExtensibility2 * This, SAFEARRAY **custom)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	return p->m_pIDTExtensibility2->lpVtbl->OnStartupComplete(p->m_pIDTExtensibility2,custom);
}

STDMETHODIMP CoreIDTExtensibility2_OnBeginShutdown(IDTExtensibility2 * This, SAFEARRAY **custom)
{
	CoreIDTExtensibility2 *p = (CoreIDTExtensibility2 *)This;

	return p->m_pIDTExtensibility2->lpVtbl->OnBeginShutdown(p->m_pIDTExtensibility2,custom);
}

static BOOL attach_to_powerpoint(void)
{
	BOOL bRet = TRUE;

	HRESULT hr = S_OK;

	ULONG uCookie = 0;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;
	PowerPointEventListener		*pPowerPointEventListener = NULL;

	do 
	{
		try
		{
			pPowerPointEventListener = ::new PowerPointEventListener();
		}
		catch (std::bad_alloc exec)
		{
			pPowerPointEventListener = NULL;
		}

		if (!pPowerPointEventListener)
		{
			break;
		}

		hr = Global.PowerPointApp->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(PowerPoint::EApplication), &pConnectionPoint);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise((IUnknown*)pPowerPointEventListener, &uCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

	} while (FALSE);


	if (pConnectionPoint)
	{
		pConnectionPoint->Release();
	}

	if (pConnectionPointContainer)
	{
		pConnectionPointContainer->Release();
	}

	return bRet;
}

static BOOL attach_to_winword(void)
{
	BOOL bRet = TRUE;

	HRESULT hr = S_OK;

	ULONG uCookie = 0;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;
	WordEventListener			*pWordEventListener = NULL;

	do 
	{
		try
		{
			pWordEventListener = ::new WordEventListener();
		}
		catch (std::bad_alloc exec)
		{
			pWordEventListener = NULL;
		}

		if (!pWordEventListener)
		{
			break;
		}

		hr = Global.WordApp->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Word::ApplicationEvents4), &pConnectionPoint);
		
		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise((IUnknown*)pWordEventListener, &uCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

	} while (FALSE);


	if (pConnectionPoint)
	{
		pConnectionPoint->Release();
	}

	if (pConnectionPointContainer)
	{
		pConnectionPointContainer->Release();
	}

	return bRet;
}

static BOOL attach_to_excel(void)
{
	BOOL bRet = TRUE;

	HRESULT hr = S_OK;

	ULONG uCookie = 0;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;
	ExcelEventListener			*pExcelEventListener = NULL;

	do 
	{
		try
		{
			pExcelEventListener = ::new ExcelEventListener();
		}
		catch (std::bad_alloc exec)
		{
			pExcelEventListener = NULL;
		}

		if (!pExcelEventListener)
		{
			break;
		}

		hr = Global.ExcelApp->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (Global.IsOffice2010)
			hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Excel2010::AppEvents), &pConnectionPoint);
		else
			hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Excel::AppEvents), &pConnectionPoint);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise((IUnknown*)pExcelEventListener, &uCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

	} while (FALSE);


	if (pConnectionPoint)
	{
		pConnectionPoint->Release();
	}

	if (pConnectionPointContainer)
	{
		pConnectionPointContainer->Release();
	}

	return bRet;
}

nudf::util::CObligations *query_engine(const WCHAR *FileName, ULONGLONG EvaluationId)
{
	nudf::util::CObligations *Obs = NULL; 

	CHECK_OBLIGATION_REQUEST req = {0};

	UCHAR ObsBuf[4096] = {0};

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		req.ProcessId	= GetCurrentProcessId();
		req.ThreadId	= GetCurrentThreadId();

		GetTempPathW(sizeof(req.TempPath)/sizeof(WCHAR), req.TempPath);

		GetLongPathNameW(req.TempPath, req.TempPath, sizeof(req.TempPath)/sizeof(WCHAR));

		req.EvaluationId	= EvaluationId;

		memcpy(req.FileName, 
			   FileName, 
			   min(sizeof(req.FileName) - sizeof(WCHAR), wcslen(FileName)*sizeof(WCHAR)));

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_CHECKOBLIGATION, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, (PVOID)ObsBuf, sizeof(ObsBuf), &bytesret))
		{
			Ctx = NULL;

			break;
		}


		try
		{
			Obs = new nudf::util::CObligations;
		}
		catch (std::bad_alloc exec)
		{
			Obs = NULL;
		}

		if (Obs)
			Obs->FromBlob(ObsBuf, bytesret);

	} while (FALSE);

	return Obs;
}

BOOL send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type)
{
	BOOL bRet = TRUE;
	
	BLOCK_NOTIFICATION_REQUEST Req = {0};

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		Req.ProcessId	= GetCurrentProcessId();
		Req.ThreadId	= GetCurrentThreadId();
		Req.SessionId	= Global.SessionId;
		Req.LanguageId	= GetSystemDefaultLangID();
		Req.Type		= Type;

		if (FileName)
		{
			memcpy(Req.FileName,
				   FileName,
				   min(sizeof(Req.FileName) - sizeof(WCHAR), wcslen(FileName)*sizeof(WCHAR)));
		}
		else
		{
			memset(Req.FileName, 0, sizeof(Req.FileName));
		}

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_BLOCK_NOTIFICATION, &Req, sizeof(Req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, NULL, 0, &bytesret))
		{
			Ctx = NULL;
			bRet = FALSE;
			break;
		}

		Ctx = NULL;

	} while (FALSE);
	
	return bRet;
}

BOOL InitializeOfficeSendMailHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
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

		if (Global.OfficeSendMailHooksInitialized)
		{
			break;
		}

		g_fnorg_MAPISendMail = (LPMAPISENDMAIL)GetProcAddress(Global.Mapi32Handle, OFFICE_MAPISENDMAIL_PROC_NAME);

		if (g_fnorg_MAPISendMail)
		{
			if (!install_hook(g_fnorg_MAPISendMail, (PVOID*)&g_fn_MAPISendMail_trampoline, Core_MAPISendMail))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.OfficeSendMailHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;

}

void CleanupOfficeSendMailHook(void)
{
	if (g_fn_MAPISendMail_trampoline)
	{
		remove_hook(g_fn_MAPISendMail_trampoline);
		g_fn_MAPISendMail_trampoline = NULL;
	}
}


ULONG WINAPI Core_MAPISendMail(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessage lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	)
{
	ULONG ulRet = 0;

	BOOL SkipOrignal = FALSE;

	WCHAR ActiveDoc[MAX_PATH] = {0};

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (Global.DisableSendMail)
		{
			ulRet = MAPI_E_ACCESS_DENIED;
			SkipOrignal = TRUE;
			SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);

			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memcpy(ActiveDoc,
				   Global.ActiveDocFileName,
				   min(sizeof(ActiveDoc) - sizeof(WCHAR), sizeof(Global.ActiveDocFileName) - sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);

			send_block_notification(ActiveDoc, NxrmdrvSendMailBlocked);

			break;
		}

	} while (FALSE);

	if (!SkipOrignal)
	{
		ulRet = g_fn_MAPISendMail_trampoline(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
	}

	return ulRet;

}

BOOL InitializePowerpointSendMailHook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.MSMAPI32Handle)
		{
			Global.MSMAPI32Handle = GetModuleHandleW(MSMAPI32_MODULE_NAME);
		}

		if (!Global.MSMAPI32Handle)
		{
			bRet = FALSE;
			break;
		}

		//
		// reference MSMapi32 to prevent it from being unloaded
		//
		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, MSMAPI32_MODULE_NAME, &Global.MSMAPI32Handle))
		{
			Global.MSMAPI32Handle = NULL;
			bRet = FALSE;
			break;
		}

		if (Global.OfficePowerPointSendMailHooksInitialized)
		{
			break;
		}

		g_fnorg_MAPISendMailW = (LPMAPISENDMAILW)GetProcAddress(Global.MSMAPI32Handle, OFFICE_MAPISENDMAILW_PROC_NAME);

		if (g_fnorg_MAPISendMailW)
		{
			if (!install_hook(g_fnorg_MAPISendMailW, (PVOID*)&g_fn_MAPISendMailW_trampoline, Core_MAPISendMailW))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.OfficePowerPointSendMailHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;
 
}

void CleanupPowerpointSendMailHook(void)
{
	if (g_fn_MAPISendMailW_trampoline)
	{
		remove_hook(g_fn_MAPISendMailW_trampoline);
		g_fn_MAPISendMailW_trampoline = NULL;
	}

	Global.MSMAPI32Handle = NULL;

	Global.OfficePowerPointSendMailHooksInitialized = FALSE;
}

ULONG WINAPI Core_MAPISendMailW(
	LHANDLE lhSession,
	ULONG_PTR ulUIParam,
	lpMapiMessageW lpMessage,
	FLAGS flFlags,
	ULONG ulReserved
	)
{
	ULONG ulRet = 0;

	BOOL SkipOrignal = FALSE;

	WCHAR ActiveDoc[MAX_PATH] = {0};

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (Global.DisableSendMail)
		{
			ulRet = MAPI_E_ACCESS_DENIED;
			SkipOrignal = TRUE;
			SetLastError(ERROR_ACCESS_DISABLED_BY_POLICY);

			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memcpy(ActiveDoc,
				   Global.ActiveDocFileName,
				   min(sizeof(ActiveDoc) - sizeof(WCHAR), sizeof(Global.ActiveDocFileName) - sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);

			send_block_notification(ActiveDoc, NxrmdrvSendMailBlocked);

			break;
		}

	} while (FALSE);

	if (!SkipOrignal)
	{
		ulRet = g_fn_MAPISendMailW_trampoline(lhSession, ulUIParam, lpMessage, flFlags, ulReserved);
	}

	return ulRet;

}

BOOL Office_Update_WaterMarked_Document_List(HWND hWnd)
{
	BOOL bRet = TRUE;

	LIST_ENTRY *ite = NULL;

	OFFICE_WATERMARKED_DOCUMENT_NODE *pNode = NULL;

	BOOL bSendNotificationToEngine = FALSE;

	EnterCriticalSection(&Global.WaterMarkedDocumentListLock);

	do 
	{
		FOR_EACH_LIST(ite, &Global.WaterMarkedDocumentList)
		{
			pNode = CONTAINING_RECORD(ite, OFFICE_WATERMARKED_DOCUMENT_NODE, Link);

			if (pNode->hWnd == hWnd)
			{
				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		if (pNode)
		{
			break;
		}

		pNode = (OFFICE_WATERMARKED_DOCUMENT_NODE *)malloc(sizeof(OFFICE_WATERMARKED_DOCUMENT_NODE));

		if (!pNode)
		{
			bRet = FALSE;
			break;
		}

		memset(pNode, 0, sizeof(OFFICE_WATERMARKED_DOCUMENT_NODE));

		pNode->hWnd	= hWnd;

		InsertHeadList(&Global.WaterMarkedDocumentList, &pNode->Link);

		Global.TotalWaterMarkedDocuments++;

		bSendNotificationToEngine = TRUE;

	} while (FALSE);

	LeaveCriticalSection(&Global.WaterMarkedDocumentListLock);

	if (bSendNotificationToEngine)
	{
		send_overlay_windows_info_to_engine(hWnd, TRUE);
	}

	return TRUE;
}

BOOL Office_Refresh_And_Remove_WaterMarked_Document_List(void)
{
	BOOL bRet = TRUE;

	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	LIST_ENTRY LocalFreeList = { 0 };

	OFFICE_WATERMARKED_DOCUMENT_NODE *pNode = NULL;

	InitializeListHead(&LocalFreeList);

	EnterCriticalSection(&Global.WaterMarkedDocumentListLock);

	do 
	{
		if (!g_fnorg_IsWindowVisible)
		{
			bRet = FALSE;
			break;
		}

		FOR_EACH_LIST_SAFE(ite, tmp, &Global.WaterMarkedDocumentList)
		{
			pNode = CONTAINING_RECORD(ite, OFFICE_WATERMARKED_DOCUMENT_NODE, Link);

			if (!g_fnorg_IsWindowVisible(pNode->hWnd))
			{
				RemoveEntryList(&pNode->Link);

				InsertHeadList(&LocalFreeList, &pNode->Link);

				Global.TotalWaterMarkedDocuments--;
			}
		}

	} while (FALSE);

	LeaveCriticalSection(&Global.WaterMarkedDocumentListLock);

	//
	// free all record and notify engine
	//
	FOR_EACH_LIST_SAFE(ite, tmp, &LocalFreeList)
	{
		pNode = CONTAINING_RECORD(ite, OFFICE_WATERMARKED_DOCUMENT_NODE, Link);

		send_overlay_windows_info_to_engine(pNode->hWnd, FALSE);

		RemoveEntryList(ite);

		free(pNode);
	}

	return bRet;
}

HRESULT WINAPI Core_StgOpenStorage(
	_In_opt_ _Null_terminated_ const WCHAR* pwcsName,
	_In_opt_ IStorage* pstgPriority,
	_In_ DWORD grfMode,
	_In_opt_z_ SNB snbExclude,
	_In_ DWORD reserved,
	_Outptr_ IStorage** ppstgOpen)
{
	HRESULT hr = S_OK;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	IWICImagingFactory *pWICFactory = NULL;

	do 
	{
		if (Global.OfficeMode != OFFICE_MODE_COMPLUS)
		{
			break;
		}

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(pwcsName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			Global.DisablePrinting = FALSE;
			Global.DisableSendMail = FALSE;
			Global.DisableClipboard = FALSE;

			break;
		}

		if (Global.IsExcel)
		{
			do 
			{
				if (!wcsstr(Global.OfficeCommandLine, L"/dde /n"))
				{
					break;
				}

				//
				// change the mode to DDE mode to prevent StgOpenStorage from monitoring
				//
				Global.OfficeMode = OFFICE_MODE_DDE;

				EnterCriticalSection(&Global.DDEDocFileNameLock);

				memcpy(Global.DDEDocFileName,
					   pwcsName,
					   min(sizeof(Global.DDEDocFileName) - sizeof(WCHAR), wcslen(pwcsName)*sizeof(WCHAR)));

				LeaveCriticalSection(&Global.DDEDocFileNameLock);

			} while (FALSE);
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   pwcsName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(pwcsName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (!(RightsMask & BUILTIN_RIGHT_PRINT))
		{
			Global.DisablePrinting = TRUE;
		}
		else
		{
			Global.DisablePrinting = FALSE;
		}

		if (!(RightsMask & BUILTIN_RIGHT_SEND))
		{
			Global.DisableSendMail = TRUE;
		}
		else
		{
			Global.DisableSendMail = FALSE;
		}

		if (!(RightsMask & BUILTIN_RIGHT_CLIPBOARD))
		{
			Global.DisableClipboard = TRUE;
		}
		else
		{
			Global.DisableClipboard = FALSE;
		}
 
		//
		// take care of obligations
		//
		Obligations = query_engine(pwcsName, EvaluationId);

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

		//
		// initialize D2D1
		//
		if (Global.IsOffice2010)
		{
			//Office2010_Update_WaterMarked_Document_List(NULL, BitmapFileName);

			break;
		}

		InterlockedCompareExchange(&Global.TotalWaterMarkedDocuments, 1, 0);

	} while (FALSE);

	return g_fn_StgOpenStorage_trampoline(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstgOpen);
}

BOOL WINAPI	Core_GetFileSizeEx(
	_In_ HANDLE hFile,
	_Out_ PLARGE_INTEGER lpFileSize
	)
{
	HRESULT hr = S_OK;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	IWICImagingFactory *pWICFactory = NULL;

	WCHAR FullPathFileName[MAX_PATH] = {0};

	DWORD dwRet = 0;

	do 
	{
		if (Global.OfficeMode != OFFICE_MODE_COMPLUS)
		{
			break;
		}

		if (!init_rm_section_safe())
		{
			break;
		}

		dwRet = GetFinalPathNameByHandleW(hFile, 
										  FullPathFileName, 
										  sizeof(FullPathFileName)/sizeof(WCHAR), 
										  FILE_NAME_OPENED|VOLUME_NAME_DOS);

		if (dwRet == 0 || dwRet >= sizeof(FullPathFileName)/sizeof(WCHAR))
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(FullPathFileName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		//
		// take care of obligations
		//
		Obligations = query_engine(FullPathFileName, EvaluationId);

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

		//
		// initialize D2D1
		//
		if (Global.IsOffice2010)
		{
			//Office2010_Update_WaterMarked_Document_List(NULL, BitmapFileName);

			break;
		}

		InterlockedCompareExchange(&Global.TotalWaterMarkedDocuments, 1, 0);

	} while (FALSE);

	return g_fn_GetFileSizeEx_trampoline(hFile, lpFileSize);
}

static BOOL takecare_excel_active_wb_if_any(void)
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;

	Excel::_Application	*pAppObj = NULL;

	long hWnd = 0;

	Excel::_Workbook	*pWb = NULL;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	BOOL ClearGlobalActiveWnd = TRUE;

	do 
	{
		if (!Global.ExcelApp)
		{
			break;
		}

		hr = Global.ExcelApp->QueryInterface(__uuidof(Excel::_Application), (void**)&pAppObj);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pAppObj)
		{
			break;
		}

		hr = pAppObj->get_ActiveWorkbook(&pWb);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pWb)
		{
			break;
		}

		hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!DocFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   DocFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(DocFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			Global.DisablePrinting = FALSE;
			Global.DisableSendMail = FALSE;
			Global.DisableClipboard = FALSE;
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

		if (!(RightsMask & BUILTIN_RIGHT_SEND))
		{
			Global.DisableSendMail = TRUE;
		}
		else
		{
			Global.DisableSendMail = FALSE;
		}

		if (!(RightsMask & BUILTIN_RIGHT_CLIPBOARD))
		{
			Global.DisableClipboard = TRUE;
		}
		else
		{
			Global.DisableClipboard = FALSE;
		}

		hr = pAppObj->get_Hwnd(&hWnd);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
		{
			if (!g_fnorg_SetWindowDisplayAffinity((HWND)(ULONG_PTR)hWnd, WDA_MONITOR))
			{

			}
		}

		//
		// take care of obligations
		//
		Obligations = query_engine(DocFullName, EvaluationId);

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

		Global.ActiveWnd = (HWND)(ULONG_PTR)hWnd;
	
		ClearGlobalActiveWnd = FALSE;

		//
		// add watermark node
		//
		Office_Update_WaterMarked_Document_List((HWND)(ULONG_PTR)hWnd);

	} while (FALSE);

	if (ClearGlobalActiveWnd)
	{
		Global.ActiveWnd = NULL;
	}

	if (pAppObj)
	{
		pAppObj->Release();
		pAppObj = NULL;
	}

	if (Obligations)
	{
		delete Obligations;
		Obligations = NULL;
	}

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return bRet;
}

static BOOL takecare_excel2010_active_wb_if_any(void)
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;

	Excel2010::_Application	*pAppObj = NULL;

	long hWnd = 0;

	Excel2010::_Workbook	*pWb = NULL;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	BOOL ClearGlobalActiveWnd = TRUE;

	do 
	{
		if (!Global.ExcelApp)
		{
			break;
		}

		hr = Global.ExcelApp->QueryInterface(__uuidof(Excel2010::_Application), (void**)&pAppObj);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pAppObj)
		{
			break;
		}

		hr = pAppObj->get_ActiveWorkbook(&pWb);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pWb)
		{
			break;
		}

		hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!DocFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   DocFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(DocFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			Global.DisablePrinting = FALSE;
			Global.DisableSendMail = FALSE;
			Global.DisableClipboard = FALSE;
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

		if (!(RightsMask & BUILTIN_RIGHT_SEND))
		{
			Global.DisableSendMail = TRUE;
		}
		else
		{
			Global.DisableSendMail = FALSE;
		}

		if (!(RightsMask & BUILTIN_RIGHT_CLIPBOARD))
		{
			Global.DisableClipboard = TRUE;
		}
		else
		{
			Global.DisableClipboard = FALSE;
		}

		hr = pAppObj->get_Hwnd(&hWnd);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
		{
			if (!g_fnorg_SetWindowDisplayAffinity((HWND)(ULONG_PTR)hWnd, WDA_MONITOR))
			{

			}
		}

		//
		// take care of obligations
		//
		Obligations = query_engine(DocFullName, EvaluationId);

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

		Global.ActiveWnd = (HWND)(ULONG_PTR)hWnd;

		ClearGlobalActiveWnd = FALSE;

		Office_Update_WaterMarked_Document_List((HWND)(ULONG_PTR)hWnd);

	} while (FALSE);

	if (ClearGlobalActiveWnd)
	{
		Global.ActiveWnd = NULL;
	}

	if (pAppObj)
	{
		pAppObj->Release();
		pAppObj = NULL;
	}

	if (Obligations)
	{
		delete Obligations;
		Obligations = NULL;
	}

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return bRet;
}

DWORD WINAPI Core_GetFileAttributesW(
	_In_ LPCWSTR lpFileName
	)
{
	WCHAR FileNameWithNXLExtension[MAX_PATH] = {0};

	DWORD FileNameLength = 0;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	WIN32_FIND_DATA FindFileData = {0};

	DWORD dwRet = g_fn_GetFileAttributesW_trampoline(lpFileName);

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (dwRet != INVALID_FILE_ATTRIBUTES)
		{
			break;
		}

		if (GetLastError() != ERROR_FILE_NOT_FOUND)
		{
			break;
		}

		FileNameLength = (DWORD)wcslen(lpFileName);

		if (FileNameLength * sizeof(WCHAR) + sizeof(L".nxl") > MAX_PATH)
		{
			break;
		}

		swprintf_s(FileNameWithNXLExtension,
				   MAX_PATH,
				   L"%s.nxl",
				   lpFileName);

		hFile = FindFirstFileW(FileNameWithNXLExtension,
							   &FindFileData);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			FindClose(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

	} while (FALSE);

	return dwRet;
}

BOOL InitializeExcelDDEHook(void)
{
	BOOL bRet = TRUE;

	HANDLE hUser32 = NULL;

	do 
	{
		hUser32 = GetModuleHandleW(USER32_MODULE_NAME);

		if (!hUser32)
		{
			bRet = FALSE;
			break;
		}

		if (Global.ExcelDDEHookInitialized)
		{
			break;
		}

		g_fnorg_UnpackDDElParam = (OFFICE_UNPACKDDELPARAM)GetProcAddress(Global.user32Handle, OFFICE_UNPACKDDELPARAM_PROC_NAME);

		if (g_fnorg_UnpackDDElParam)
		{
			if (!install_hook(g_fnorg_UnpackDDElParam, (PVOID*)&g_fn_UnpackDDElParam_trampoline, Core_UnpackDDElParam))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.ExcelDDEHookInitialized = TRUE;

	} while (FALSE);

	return bRet;
}

void CleanupExcelDDEHook(void)
{
	if (g_fn_UnpackDDElParam_trampoline)
	{
		remove_hook(g_fn_UnpackDDElParam_trampoline);
		g_fn_UnpackDDElParam_trampoline = NULL;
	}
}

BOOL WINAPI Core_UnpackDDElParam(
	_In_  UINT      msg,
	_In_  LPARAM    lParam,
	_Out_ PUINT_PTR puiLo,
	_Out_ PUINT_PTR puiHi
	)
{
	WCHAR *DDECmd = NULL;

	WCHAR *b = NULL;
	WCHAR *e = NULL;

	WCHAR FileName[MAX_PATH] = { 0 };

	BOOL UnlockGlobalMemory = FALSE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	HRESULT hr = S_OK;

	do 
	{
		if (msg != WM_DDE_EXECUTE)
		{
			break;
		}

		if (!init_rm_section_safe())
		{
			break;
		}

		if (!lParam)
		{
			break;
		}

		DDECmd = (WCHAR *)GlobalLock((HGLOBAL)lParam);

		if (!DDECmd)
		{
			break;
		}

		UnlockGlobalMemory = TRUE;

		b = wcsstr(DDECmd, OFFICE_EXCEL_DDE_PATENT_BEGIN);

		e = wcsstr(DDECmd, OFFICE_EXCEL_DDE_PATENT_END);

		if (b == NULL || e == NULL)
		{
			break;
		}

		b += wcslen(OFFICE_EXCEL_DDE_PATENT_BEGIN);

		memcpy(FileName, 
			   b, 
			   min(sizeof(FileName) - sizeof(WCHAR), ((ULONG_PTR)e - (ULONG_PTR)b)));


		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(FileName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		//
		// OFFICE_EXCEL_DDE_EMPTY_NEW must shorter than what's in DDECmd
		// so, it's safe to copy without checking the length
		//
		memcpy(DDECmd, 
			   OFFICE_EXCEL_DDE_EMPTY_NEW, 
			   sizeof(OFFICE_EXCEL_DDE_EMPTY_NEW));

	} while (FALSE);
	
	if (UnlockGlobalMemory)
	{
		GlobalUnlock((HGLOBAL)lParam);
	}

	return g_fn_UnpackDDElParam_trampoline(msg, lParam, puiLo, puiHi);
}

static ULONGLONG find_excel2013_copy_worksheet_function(PVOID pbase)
{
	const UCHAR f[] = { 0x55, 0x8b, 0xec, 0x81, 0xec, 0x90, 0x06, 0x00, 0x00, 0xa1 };

	const UCHAR g[] = { 0x33, 0xc5, 0x89, 0x45, 0xfc, 0x8b, 0x45, 0x0c, 0x64, 0x8b,	\
						0x0d, 0x2c, 0x00, 0x00, 0x00, 0x8b, 0x55, 0x08, 0x89, 0x85,	\
						0x88, 0xf9, 0xff, 0xff, 0x8b, 0x45, 0x10, 0x53, 0x8b, 0x19,	\
						0x89, 0x85, 0x78, 0xf9, 0xff, 0xff, 0x8b, 0x45, 0x14, 0x56 };

	UCHAR *p = NULL;

	ULONGLONG offset = 0;

	do 
	{
		__try
		{
			if (pbase)
			{
				for (p = (UCHAR*)pbase; p < (UCHAR*)pbase + 5*1024*1024; p++)
				{
					if (*p != 0x55)
					{
						continue;
					}

					if (memcmp(p, f, sizeof(f)) != 0)
					{
						continue;
					}

					p += sizeof(f);

					//
					// we need to skip 4 bytes because these 4 bytes is an internal offset which changes version by version
					//
					p += 4;

					if (memcmp(p, g, sizeof(g)) != 0)
					{
						continue;
					}

					offset = (ULONGLONG)((ULONG_PTR)p - 4 - sizeof(f) - (ULONG_PTR)pbase);

					break;
				}

			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			offset = 0;
			break;
		}

	} while (FALSE);

	return offset;
}

static ULONGLONG query_saved_excel2013_copy_worksheet_function(void)
{
	ULONGLONG Offset = 0ULL;

	QUERY_CORE_CTX_REQUEST req = { 0 };

	QUERY_CORE_CTX_RESPONSE resp = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do
	{
		req.ProcessId		= GetCurrentProcessId();
		req.ThreadId		= GetCurrentThreadId();
		req.SessionId		= Global.SessionId;
		req.ModuleChecksum	= Global.ExcelModuleChecksum;

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

		if (resp.ModuleChecksum == Global.ExcelModuleChecksum)
		{
			Offset = resp.CtxData[0];
		}

	} while (FALSE);

	return Offset;
}

static void update_excel2013_copy_worksheet_function(void)
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
		req.ModuleChecksum	= Global.ExcelModuleChecksum;
		req.CtxData[0]		= Global.Excel2013CopyWorkSheetOffset;

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

ULONG WINAPI Core_Excel2013_CopyWorkSheet(PVOID P1, PVOID P2, const WCHAR *WorkSheetName, PVOID P4, PVOID P5, PVOID P6, PVOID P7)
{
	//
	// From observation, the parameters passed to this function under
	// various situations are the following:
	// 1. Copying SheetName from SrcFile.xlsx to DestFile.xlsx
	//    - WorkSheetName   = 0x????, "[DestFile.xlsx]SheetName", 0x0000
	//    - P4              = NULL
	//    - P5              = NULL
	// 2. Moving SheetName from SrcFile.xlsx to DestFile.xlsx
	//    - WorkSheetName   = 0x????, "[DestFile.xlsx]SheetName", 0x0000
	//    - P4              = NULL
	//    - P5              = NULL
	// 3. Renaming OldSheetName to NewSheetName in File.xlsx
	//    - WorksheetName   = 0x????, "NewSheetName", 0x0000
	//    - P4              = 0x????, "C:\Dir\File.xlsx", 0x0000
	//    - P5              = 0x????, "C:\Dir\File.xlsx", 0x0000
	//

	ULONG uRet = 1;

	BOOL bSkipOriginal = FALSE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	HRESULT hr = S_OK;

	WCHAR FileName[MAX_PATH] = { 0 };

	do 
	{
		//
		// If the operation is renaming sheet, just allow it without checking
		// for rights.  This is because the user won't be able to save the
		// changes anyway if there is no Edit right.  This is analogus to
		// modifying cell content in a file without Edit right.
		//
		if (P4 && P5)
		{
			break;
		}

		if (!init_rm_section_safe())
		{
			break;
		}

		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memcpy(FileName,
			   Global.ActiveDocFileName,
			   min(sizeof(FileName) - sizeof(WCHAR), sizeof(Global.ActiveDocFileName) - sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(FileName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		bSkipOriginal = TRUE;

	} while (FALSE);

	if (!bSkipOriginal)
	{
		return g_fn_Excel2013_CopyWorkSheet_trampoline(P1, P2, WorkSheetName, P4, P5, P6, P7);
	}
	else
	{
		return 0;
	}
}

void takecare_powerpoint_create_in_memory_content_from_nxl_file(void)
{
	WCHAR *b = NULL;
	WCHAR *e = NULL;

	WCHAR FileName[MAX_PATH] = { 0 };

	do
	{
		if (!Global.IsPowerPoint)
		{
			break;
		}

		b = wcsstr(Global.OfficeCommandLine, L"/n ");

		if (!b)
		{
			break;
		}

		b = wcsstr(b, L"\"");

		if (!b)
		{
			break;
		}

		b += 1;

		e = wcsstr(b, L"\"");		// file name does not allow L"\"" in it.

		if (!e)
		{
			break;
		}

		memcpy(FileName,
			   b,
			   min(sizeof(FileName) - sizeof(WCHAR), ((ULONG_PTR)e - (ULONG_PTR)b)));

		do
		{
			ULONGLONG RightsMask = 0;
			ULONGLONG CustomRightsMask = 0;
			ULONGLONG EvaluationId = 0;

			HRESULT hr = S_OK;

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
				break;
			}

			memset(b,
				   0,
				   ((ULONG_PTR)e - (ULONG_PTR)b));

			*b = L'\"';

		} while (FALSE);

	} while (FALSE);

	return;
}

void takecare_winword_create_in_memory_content_from_nxl_file(void)
{
	WCHAR *b = NULL;
	WCHAR *e = NULL;

	WCHAR FileName[MAX_PATH] = { 0 };

	do
	{
		if (!Global.IsWinWord)
		{
			break;
		}

		b = wcsstr(Global.OfficeCommandLine, L"/n /f");

		if (!b)
		{
			break;
		}

		b = wcsstr(b, L"\"");

		if (!b)
		{
			break;
		}

		b += 1;

		e = wcsstr(b, L"\"");		// file name does not allow L"\"" in it.

		if (!e)
		{
			break;
		}

		memcpy(FileName,
			   b,
			   min(sizeof(FileName) - sizeof(WCHAR), ((ULONG_PTR)e - (ULONG_PTR)b)));

		do
		{
			ULONGLONG RightsMask = 0;
			ULONGLONG CustomRightsMask = 0;
			ULONGLONG EvaluationId = 0;

			HRESULT hr = S_OK;

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
				break;
			}

			memset(b,
				   0,
				   ((ULONG_PTR)e - (ULONG_PTR)b));

			*b = L'\"';

		} while (FALSE);

	} while (FALSE);

	return;
}

BOOL send_overlay_windows_info_to_engine(HWND hWnd, BOOL Add)
{
	BOOL bRet = TRUE;

	UPDATE_OVERLAY_WINDOW_REQUEST req = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		req.ProcessId = GetCurrentProcessId();
		req.ThreadId  = GetCurrentThreadId();
		req.SessionId = Global.SessionId;

		req.Op = Add ? NXRMDRV_OVERLAY_WINDOW_ADD : NXRMDRV_OVERLAY_WINDOW_DELETE;

		req.hWnd = (ULONG)(ULONG_PTR)hWnd;
		
		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_UPDATE_OVERLAY_WINDOW, &req, sizeof(req));

		if (!Ctx)
		{
			bRet = FALSE;
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, NULL, 0, &bytesret))
		{
			bRet = FALSE;

			Ctx = NULL;
			break;
		}

	} while (FALSE);

	return bRet;
}
