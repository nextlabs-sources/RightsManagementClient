#include "stdafx.h"
#include "excelevents.h"
#include "nxrmcoreglobal.h"
#include "office.h"
#include "screenoverlay.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA			Global;

	extern SETWINDOWDISPLAYAFFINITY	g_fnorg_SetWindowDisplayAffinity;

	extern GETCLIENTRECT			g_fnorg_GetClientRect;

	extern INVALIDATERECT			g_fnorg_InvalidateRect;

	extern GETGUITHREADINFO			g_fnorg_GetGUIThreadInfo;

	extern SYSFREESTRING			g_fnorg_SysFreeString;

	extern SYSALLOCSTRING			g_fnorg_SysAllocString;

	extern nudf::util::CObligations *query_engine(const WCHAR *FileName, ULONGLONG EvaluationId);

	extern BOOL						send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

	extern BOOL						Office_Update_WaterMarked_Document_List(HWND hWnd);

	extern BOOL						Office_Refresh_And_Remove_WaterMarked_Document_List(void);

	const VARIANT vIgnore = {0};

#ifdef __cplusplus
}
#endif

ExcelEventListener::ExcelEventListener()
{
	m_uRefCount = 0;
};

ExcelEventListener::~ExcelEventListener()
{

}

HRESULT STDMETHODCALLTYPE ExcelEventListener::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */ 
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppvObject = NULL;

	do 
	{
		if (__uuidof(Excel::AppEvents) == riid)
		{
			punk = (Excel::AppEvents*)this;
		}
		else if (IID_IUnknown == riid)
		{
			punk = (IUnknown*)this;
		}
		else if (IID_IDispatch == riid)
		{
			punk = (IDispatch*)this;
		}
		else
		{
			hRet = E_NOINTERFACE;
			break;
		}

		AddRef();

		*ppvObject = punk;

	} while (FALSE);

	return hRet;
}

ULONG STDMETHODCALLTYPE ExcelEventListener::AddRef(void)
{
	m_uRefCount++;

	return m_uRefCount;
}

ULONG STDMETHODCALLTYPE ExcelEventListener::Release(void)
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount)
	{
		delete this;
	}

	return uCount;
}

HRESULT STDMETHODCALLTYPE ExcelEventListener::GetTypeInfoCount( 
	/* [out] */ __RPC__out UINT *pctinfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ExcelEventListener::GetTypeInfo( 
	/* [in] */ UINT iTInfo,
	/* [in] */ LCID lcid,
	/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ExcelEventListener::GetIDsOfNames( 
	/* [in] */ __RPC__in REFIID riid,
	/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
	/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
	/* [in] */ LCID lcid,
	/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE ExcelEventListener::Invoke( 
	/* [annotation][in] */ 
	_In_  DISPID dispIdMember,
	/* [annotation][in] */ 
	_In_  REFIID riid,
	/* [annotation][in] */ 
	_In_  LCID lcid,
	/* [annotation][in] */ 
	_In_  WORD wFlags,
	/* [annotation][out][in] */ 
	_In_  DISPPARAMS *pDispParams,
	/* [annotation][out] */ 
	_Out_opt_  VARIANT *pVarResult,
	/* [annotation][out] */ 
	_Out_opt_  EXCEPINFO *pExcepInfo,
	/* [annotation][out] */ 
	_Out_opt_  UINT *puArgErr)
{
	HRESULT hr = DISP_E_MEMBERNOTFOUND;

	void *Wb = NULL;
	void *Wn = NULL;
	void *PvWindow = NULL;

	VARIANT_BOOL *Cancel = NULL;
	VARIANT_BOOL SaveAsUI = VARIANT_TRUE;

	switch (dispIdMember)
	{
	case WindowActivate_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Wb = (void*)pDispParams->rgvarg[i].pdispVal;
				}

				if (pDispParams->rgdispidNamedArgs[i] == 1)
				{
					Wn = (void*)pDispParams->rgvarg[i].pdispVal;
				}
			}

		}
		else
		{
			// (parameters are on stack, thus in reverse order)
			Wb = (void*)pDispParams->rgvarg[1].pdispVal;
			Wn = (void *)pDispParams->rgvarg[0].pdispVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WindowActivate((Excel2010::_Workbook*)Wb, (Excel2010::Window*)Wn);
		}
		else
		{
			hr = WindowActivate((Excel::_Workbook*)Wb, (Excel::Window*)Wn);
		}

		break;
	case WindowDeactivate_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Wb = (void*)pDispParams->rgvarg[i].pdispVal;
				}

				if (pDispParams->rgdispidNamedArgs[i] == 1)
				{
					Wn = (void*)pDispParams->rgvarg[i].pdispVal;
				}
			}

		}
		else
		{
			// (parameters are on stack, thus in reverse order)
			Wb = (void*)pDispParams->rgvarg[1].pdispVal;
			Wn = (void*)pDispParams->rgvarg[0].pdispVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WindowDeactivate((Excel2010::_Workbook*)Wb, (Excel2010::Window*)Wn);
		}
		else
		{
			hr = WindowDeactivate((Excel::_Workbook*)Wb, (Excel::Window*)Wn);
		}

		break;

	case WorkbookBeforeSave_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 3); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Wb = (void*)pDispParams->rgvarg[i].pdispVal;
				}

				if (pDispParams->rgdispidNamedArgs[i] == 1)
				{
					SaveAsUI = pDispParams->rgvarg[i].boolVal;
				}

				if (pDispParams->rgdispidNamedArgs[i] == 2)
				{
					Cancel = pDispParams->rgvarg[i].pboolVal;
				}
			}

		}
		else
		{
			// (parameters are on stack, thus in reverse order)
			Wb = (void*)pDispParams->rgvarg[2].pdispVal;
			SaveAsUI = pDispParams->rgvarg[1].boolVal;
			Cancel = pDispParams->rgvarg[0].pboolVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WorkbookBeforeSave((Excel2010::_Workbook*)Wb, SaveAsUI, Cancel);
		}
		else
		{
			hr = WorkbookBeforeSave((Excel::_Workbook*)Wb, SaveAsUI, Cancel);
		}

		break;

	case WorkbookBeforePrint_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Wb = (void*)pDispParams->rgvarg[i].pdispVal;
				}

				if (pDispParams->rgdispidNamedArgs[i] == 1)
				{
					Cancel = pDispParams->rgvarg[i].pboolVal;
				}
			}

		}
		else
		{
			// (parameters are on stack, thus in reverse order)
			Wb = (void*)pDispParams->rgvarg[1].pdispVal;
			Cancel = pDispParams->rgvarg[0].pboolVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WorkbookBeforePrint((Excel2010::_Workbook*)Wb, Cancel);
		}
		else
		{
			hr = WorkbookBeforePrint((Excel::_Workbook*)Wb, Cancel);
		}

		break;

	case ProtectedViewWindowActivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = ProtectedViewWindowActivate((Excel2010::ProtectedViewWindow*)PvWindow);
		}
		else
		{
			hr = ProtectedViewWindowActivate((Excel::ProtectedViewWindow*)PvWindow);
		}

		break;

	case ProtectedViewWindowDeactivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = ProtectedViewWindowDeactivate((Excel2010::ProtectedViewWindow*)PvWindow);
		}
		else
		{
			hr = ProtectedViewWindowDeactivate((Excel::ProtectedViewWindow*)PvWindow);
		}
	
		break;

	case WorkbookBeforeClose_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Wb = (void*)pDispParams->rgvarg[i].pdispVal;
				}

				if (pDispParams->rgdispidNamedArgs[i] == 1)
				{
					Cancel = pDispParams->rgvarg[i].pboolVal;
				}
			}

		}
		else
		{
			// (parameters are on stack, thus in reverse order)
			Wb = (void*)pDispParams->rgvarg[1].pdispVal;
			Cancel = pDispParams->rgvarg[0].pboolVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WorkbookBeforeClose((Excel2010::_Workbook*)Wb, Cancel);
		}
		else
		{
			hr = WorkbookBeforeClose((Excel::_Workbook*)Wb, Cancel);
		}

		break;

	default:
		break;
	}

	return hr;
}


STDMETHODIMP ExcelEventListener::WorkbookBeforeSave (
	/*[in]*/ Excel::_Workbook * Wb,
	/*[in]*/ VARIANT_BOOL SaveAsUI,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvluationId = 0;
	do 
	{
		hr = Wb->get_FullName(0x0409/*US English*/, &DocFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!DocFullName)
		{
			break;
		}

		if (!init_rm_section_safe())
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

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(DocFullName);

				DocFullName = NULL;

				DocFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvluationId);

		if (FAILED(hr))
		{
			break;
		}

		//
		// don't have edit right and it's SaveAs
		//
		if (!(RightsMask & BUILTIN_RIGHT_EDIT) && (SaveAsUI == VARIANT_FALSE))
		{
			*Cancel = VARIANT_TRUE;
		}
		else if (!(RightsMask & BUILTIN_RIGHT_SAVEAS) && (SaveAsUI == VARIANT_TRUE))
		{
			*Cancel = VARIANT_TRUE;
		}

		if (*Cancel == VARIANT_TRUE)
		{
			send_block_notification(DocFullName, NxrmdrvSaveFileBlocked);
		}

		//
		// driver makes sure Office can't save file if user does not have edit right
		//
	} while (FALSE);

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

STDMETHODIMP ExcelEventListener::WorkbookBeforePrint (
	/*[in]*/ Excel::_Workbook * Wb,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (Global.DisablePrinting)
		{
			Wb->get_FullName(0x0409/*US English*/, &DocFullName);

			//
			// send_block_notification takes care of NULL pointer
			//
			send_block_notification(DocFullName, NXrmdrvPrintingBlocked);

			*Cancel = -1;
		}

	} while (FALSE);

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

STDMETHODIMP ExcelEventListener::WindowActivate (
	/*[in]*/ Excel::_Workbook * Wb,
	/*[in]*/ Excel::Window * Wn )
{
	HRESULT hr = S_OK;

	Excel::_Application	*pAppObj = NULL;

	long hWnd = 0;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	BOOL ClearGlobalActiveWnd = TRUE;

	VARIANT vTrue = {0};

	BOOL EnableDragDropCells = TRUE;

	vTrue.vt = VT_BOOL;
	vTrue.boolVal = VARIANT_TRUE;

	do 
	{
		hr = Global.ExcelApp->QueryInterface(__uuidof(Excel::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = Wb->get_FullName(0x0409/*US English*/, &WbFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!WbFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   WbFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(WbFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		Office_Refresh_And_Remove_WaterMarked_Document_List();

		if (!init_rm_section_safe())
		{
			break;
		}

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(WbFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(WbFullName);

				WbFullName = NULL;

				WbFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!WbFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   WbFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(WbFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(WbFullName, &RightsMask, &CustomRightsMask, &EvlauationId);

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

			pAppObj->put_CellDragAndDrop(0x0409/*US English*/, VARIANT_FALSE);

			EnableDragDropCells = FALSE;
		}
		else
		{
			Global.DisableClipboard = FALSE;
		}

		//if (!(RightsMask & BUILTIN_RIGHT_EDIT))
		//{
		//	Excel::Sheets *pSheets = NULL;
		//	Excel::_Worksheet *pWorkSheet = NULL;

		//	VARIANT password = {0};

		//	VARIANT_BOOL Saved = VARIANT_FALSE;

		//	BOOL bRestoreSaved = FALSE;

		//	if (S_OK == Wb->get_Saved(0x0409/*US English*/, &Saved))
		//	{
		//		bRestoreSaved = TRUE;
		//	}

		//	password.vt	= VT_BSTR;
		//	password.bstrVal = g_fnorg_SysAllocString(L"{B7FF7B6F-A283-4870-B689-822E60D72FF6}");

		//	Wb->Protect(password, vTrue, vTrue);

		//	pAppObj->put_CellDragAndDrop(0x0409/*US English*/, VARIANT_FALSE);

		//	EnableDragDropCells = FALSE;

		//	Wb->get_Worksheets(&pSheets);

		//	if (pSheets)
		//	{
		//		long count = 0;

		//		pSheets->get_Count(&count);

		//		for(long i = 0; i < count; i++)
		//		{
		//			VARIANT index;
		//			index.vt	= VT_I4;
		//			index.lVal	= i+1;

		//			pSheets->get_Item(index, (IDispatch **)&pWorkSheet);

		//			if (pWorkSheet)
		//			{
		//				password.vt	= VT_BSTR;
		//				password.bstrVal = g_fnorg_SysAllocString(L"{B7FF7B6F-A283-4870-B689-822E60D72FF6}");

		//				pWorkSheet->Protect(password, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore);

		//				pWorkSheet->Release();
		//				
		//				pWorkSheet = NULL;
		//			}
		//		}

		//		pSheets->Release();
		//		pSheets = NULL;
		//	}

		//	if (bRestoreSaved && Wb)
		//	{
		//		Wb->put_Saved(0x0409/*US English*/, Saved);
		//	}
		//}

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
		Obligations = query_engine(WbFullName, EvlauationId);

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

	if (EnableDragDropCells && pAppObj)
	{
		VARIANT_BOOL DragDropCellsStatus = VARIANT_TRUE;

		hr = pAppObj->get_CellDragAndDrop(0x0409/*US English*/, &DragDropCellsStatus);

		if (SUCCEEDED(hr) && DragDropCellsStatus == VARIANT_FALSE)
		{
			pAppObj->put_CellDragAndDrop(0x0409/*US English*/, VARIANT_TRUE);
		}
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

	if (WbFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(WbFullName);
		WbFullName = NULL;
	}

	return hr;

}

STDMETHODIMP ExcelEventListener::WindowDeactivate (
	/*[in]*/ Excel::_Workbook * Wb,
	/*[in]*/ Excel::Window * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}


STDMETHODIMP ExcelEventListener::ProtectedViewWindowActivate (
	/*[in]*/ Excel::ProtectedViewWindow * Pvw )
{
	HRESULT hr = S_OK;

	Excel::_Application	*pAppObj = NULL;
	Excel::_Workbook *Wb = NULL;
	Excel::IProtectedViewWindow *pProtectVw = NULL;

	long hWnd = 0;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	BOOL ClearGlobalActiveWnd = TRUE;

	do 
	{
		hr = Global.ExcelApp->QueryInterface(__uuidof(Excel::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		pProtectVw = (Excel::IProtectedViewWindow *)Pvw;

		hr = pProtectVw->get_Workbook(&Wb);

		if (FAILED(hr) || Wb == NULL)
		{
			break;
		}

		hr = Wb->get_FullName(0x0409/*US English*/, &WbFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!WbFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   WbFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(WbFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(WbFullName, &RightsMask, &CustomRightsMask, &EvlauationId);

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
		Obligations = query_engine(WbFullName, EvlauationId);

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

	if (Wb)
	{
		Wb->Release();
		Wb = NULL;
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

	if (WbFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(WbFullName);
		WbFullName = NULL;
	}

	return hr;
}

STDMETHODIMP ExcelEventListener::ProtectedViewWindowDeactivate (
	/*[in]*/ Excel::ProtectedViewWindow * Pvw )
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP ExcelEventListener::WorkbookBeforeClose (
	/*[in]*/ Excel::_Workbook * Wb,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	if (Global.OfficeMode == OFFICE_MODE_DDE)
	{
		Global.OfficeMode = OFFICE_MODE_NORMAL;

		EnterCriticalSection(&Global.DDEDocFileNameLock);

		memset(Global.DDEDocFileName, 0 , sizeof(Global.DDEDocFileName));

		LeaveCriticalSection(&Global.DDEDocFileNameLock);
	}

	return hr;
}

STDMETHODIMP ExcelEventListener::WorkbookBeforeSave (
	/*[in]*/ Excel2010::_Workbook * Wb,
	/*[in]*/ VARIANT_BOOL SaveAsUI,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		hr = Wb->get_FullName(0x0409/*US English*/, &DocFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!DocFullName)
		{
			break;
		}

		if (!init_rm_section_safe())
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

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(DocFullName);

				DocFullName = NULL;

				DocFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		//
		// don't have edit right and it's SaveAs
		//
		if (!(RightsMask & BUILTIN_RIGHT_EDIT) && (SaveAsUI == VARIANT_FALSE))
		{
			*Cancel = VARIANT_TRUE;
		}
		else if (!(RightsMask & BUILTIN_RIGHT_SAVEAS) && (SaveAsUI == VARIANT_TRUE))
		{
			*Cancel = VARIANT_TRUE;
		}

		if (*Cancel == VARIANT_TRUE)
		{
			send_block_notification(DocFullName, NxrmdrvSaveFileBlocked);
		}
		//
		// driver makes sure Office can't save file if user does not have edit right
		//
	} while (FALSE);

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

STDMETHODIMP ExcelEventListener::WorkbookBeforePrint (
	/*[in]*/ Excel2010::_Workbook * Wb,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;
	
	BSTR DocFullName = NULL;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (Global.DisablePrinting)
		{
			Wb->get_FullName(0x0409/*US English*/, &DocFullName);
			
			send_block_notification(DocFullName, NXrmdrvPrintingBlocked);

			*Cancel = -1;
		}

	} while (FALSE);

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}


STDMETHODIMP ExcelEventListener::WindowActivate (
	/*[in]*/ Excel2010::_Workbook * Wb,
	/*[in]*/ Excel2010::Window * Wn )
{
	HRESULT hr = S_OK;

	Excel2010::_Application	*pAppObj = NULL;

	HWND hWnd = NULL;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	GUITHREADINFO GuiThreadInfo = {0};

	BOOL EnableDragDropCells = TRUE;

	VARIANT vTrue = {0};

	vTrue.vt = VT_BOOL;
	vTrue.boolVal = VARIANT_TRUE;

	do 
	{
		hr = Global.ExcelApp->QueryInterface(__uuidof(Excel2010::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = Wb->get_FullName(0x0409/*US English*/, &WbFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!WbFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   WbFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(WbFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		Office_Refresh_And_Remove_WaterMarked_Document_List();

		if (!init_rm_section_safe())
		{
			break;
		}

		GuiThreadInfo.cbSize = sizeof(GuiThreadInfo);

		if (!g_fnorg_GetGUIThreadInfo(GetCurrentThreadId(), &GuiThreadInfo))
		{
			break;
		}

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(WbFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(WbFullName);

				WbFullName = NULL;

				WbFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!WbFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   WbFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(WbFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(WbFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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

			pAppObj->put_CellDragAndDrop(0x0409/*US English*/, VARIANT_FALSE);
			EnableDragDropCells = FALSE;
		}
		else
		{
			Global.DisableClipboard = FALSE;
		}

		//if (!(RightsMask & BUILTIN_RIGHT_EDIT))
		//{
		//	Excel2010::Sheets *pSheets = NULL;
		//	Excel2010::_Worksheet *pWorkSheet = NULL;

		//	VARIANT password = {0};

		//	VARIANT_BOOL Saved = VARIANT_FALSE;

		//	BOOL bRestoreSaved = FALSE;

		//	if (S_OK == Wb->get_Saved(0x0409/*US English*/, &Saved))
		//	{
		//		bRestoreSaved = TRUE;
		//	}

		//	password.vt	= VT_BSTR;
		//	password.bstrVal = g_fnorg_SysAllocString(L"{B7FF7B6F-A283-4870-B689-822E60D72FF6}");

		//	Wb->Protect(password, vTrue, vTrue);

		//	pAppObj->put_CellDragAndDrop(0x0409/*US English*/, VARIANT_FALSE);

		//	EnableDragDropCells = FALSE;

		//	Wb->get_Worksheets(&pSheets);

		//	if (pSheets)
		//	{
		//		long count = 0;

		//		pSheets->get_Count(&count);

		//		for(long i = 0; i < count; i++)
		//		{
		//			VARIANT index;
		//			index.vt	= VT_I4;
		//			index.lVal	= i+1;

		//			pSheets->get_Item(index, (IDispatch **)&pWorkSheet);

		//			if (pWorkSheet)
		//			{
		//				password.vt	= VT_BSTR;
		//				password.bstrVal = g_fnorg_SysAllocString(L"{B7FF7B6F-A283-4870-B689-822E60D72FF6}");

		//				pWorkSheet->Protect(password, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore, vIgnore);

		//				pWorkSheet->Release();

		//				pWorkSheet = NULL;
		//			}
		//		}

		//		pSheets->Release();
		//		pSheets = NULL;
		//	}

		//	if (bRestoreSaved && Wb)
		//	{
		//		Wb->put_Saved(0x0409/*US English*/, Saved);
		//	}
		//}

		if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
		{
			hr = pAppObj->get_Hwnd((long*)&hWnd);

			if (SUCCEEDED(hr))
			{
				if (!g_fnorg_SetWindowDisplayAffinity(hWnd, WDA_MONITOR))
				{

				}
			}
		}

		//
		// take care of obligations
		//
		Obligations = query_engine(WbFullName, EvaluationId);

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

		if (!hWnd)
		{
			hr = pAppObj->get_Hwnd((long*)&hWnd);
		}

		if (hWnd)
			Office_Update_WaterMarked_Document_List(hWnd);

		if (g_fnorg_InvalidateRect != NULL && 
			g_fnorg_GetClientRect != NULL)
		{
			RECT rc = { 0 };

			g_fnorg_GetClientRect(hWnd, &rc);
			g_fnorg_InvalidateRect(hWnd, &rc, TRUE);
		}

	} while (FALSE);

	if (EnableDragDropCells && pAppObj)
	{
		VARIANT_BOOL DragDropCellsStatus = VARIANT_TRUE;

		hr = pAppObj->get_CellDragAndDrop(0x0409/*US English*/, &DragDropCellsStatus);

		if (SUCCEEDED(hr) && DragDropCellsStatus == VARIANT_FALSE)
		{
			pAppObj->put_CellDragAndDrop(0x0409/*US English*/, VARIANT_TRUE);
		}
	}

	if (pAppObj)
	{
		pAppObj->Release();
	}

	if (Obligations)
	{
		delete Obligations;
	}

	if (WbFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(WbFullName);
		WbFullName = NULL;
	}

	return hr;
}

STDMETHODIMP ExcelEventListener::WindowDeactivate (
	/*[in]*/ Excel2010::_Workbook * Wb,
	/*[in]*/ Excel2010::Window * Wn )
{
	HRESULT hr = S_OK;

	//Excel2010::_Application	*pAppObj = NULL;

	//long hWnd = 0;

	//do 
	//{
	//	hr = Global.ExcelApp->QueryInterface(__uuidof(Excel2010::_Application), (void**)&pAppObj);

	//	if (FAILED(hr))
	//	{
	//		break;
	//	}

	//	hr = pAppObj->get_Hwnd(&hWnd);

	//	if (FAILED(hr))
	//	{
	//		break;
	//	}

	//} while (FALSE);

	//if (pAppObj)
	//{
	//	pAppObj->Release();
	//}

	return hr;
}

STDMETHODIMP ExcelEventListener::ProtectedViewWindowActivate (
	/*[in]*/ Excel2010::ProtectedViewWindow * Pvw )
{
	HRESULT hr = S_OK;

	Excel2010::_Application *pAppObj = NULL;
	Excel2010::IProtectedViewWindow *pProtectVw = NULL;
	Excel2010::_Workbook *Wb = NULL;

	HWND hWnd = NULL;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	GUITHREADINFO GuiThreadInfo = {0};

	do 
	{
		hr = Global.ExcelApp->QueryInterface(__uuidof(Excel2010::_Application), (void**)&pAppObj);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pProtectVw = (Excel2010::IProtectedViewWindow *)Pvw;

		hr = pProtectVw->get_Workbook(&Wb);

		if (FAILED(hr) || Wb == NULL)
		{
			break;
		}

		hr = Wb->get_FullName(0x0409/*US English*/, &WbFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!WbFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   WbFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(WbFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		Office_Refresh_And_Remove_WaterMarked_Document_List();

		if (!init_rm_section_safe())
		{
			break;
		}

		GuiThreadInfo.cbSize = sizeof(GuiThreadInfo);

		if (!g_fnorg_GetGUIThreadInfo(GetCurrentThreadId(), &GuiThreadInfo))
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(WbFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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

		if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
		{
			hr = pAppObj->get_Hwnd((long*)&hWnd);

			if (SUCCEEDED(hr))
			{
				if (!g_fnorg_SetWindowDisplayAffinity(hWnd, WDA_MONITOR))
				{

				}
			}
		}

		//
		// take care of obligations
		//
		Obligations = query_engine(WbFullName, EvaluationId);

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

		if (!hWnd)
			pAppObj->get_Hwnd((long*)&hWnd);
		
		if (hWnd)
			Office_Update_WaterMarked_Document_List(hWnd);
		
		if (g_fnorg_InvalidateRect != NULL &&
			g_fnorg_GetClientRect != NULL)
		{
			RECT rc = { 0 };

			g_fnorg_GetClientRect(hWnd, &rc);
			g_fnorg_InvalidateRect(hWnd, &rc, TRUE);
		}

	} while (FALSE);

	if (pAppObj)
	{
		pAppObj->Release();
	}

	if (Wb)
	{
		Wb->Release();
	}

	if (Obligations)
	{
		delete Obligations;
	}

	if (WbFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(WbFullName);
		WbFullName = NULL;
	}

	return hr;
}

STDMETHODIMP ExcelEventListener::ProtectedViewWindowDeactivate (
	/*[in]*/ Excel2010::ProtectedViewWindow * Pvw )
{
	HRESULT hr = S_OK;

	return hr;

}

STDMETHODIMP ExcelEventListener::WorkbookBeforeClose (
	/*[in]*/ Excel2010::_Workbook * Wb,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	if (Global.OfficeMode == OFFICE_MODE_DDE)
	{
		Global.OfficeMode = OFFICE_MODE_NORMAL;

		EnterCriticalSection(&Global.DDEDocFileNameLock);

		memset(Global.DDEDocFileName, 0 , sizeof(Global.DDEDocFileName));

		LeaveCriticalSection(&Global.DDEDocFileNameLock);
	}

	return hr;
}