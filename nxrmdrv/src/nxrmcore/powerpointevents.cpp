#include "stdafx.h"
#include "mso.h"
#include "powerpoint.h"
#include "powerpointevents.h"
#include "nxrmcoreglobal.h"
#include "office.h"
#include "screenoverlay.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA			Global;

	extern SETWINDOWDISPLAYAFFINITY	g_fnorg_SetWindowDisplayAffinity;

	extern LOADIMAGEW				g_fnorg_LoadImageW;

	extern CREATECOMPATIBLEDC		g_fnorg_CreateCompatibleDC;

	extern SELECTOBJECT				g_fnorg_SelectObject;

	extern DELETEDC					g_fnorg_DeleteDC;

	extern DELETEOBJECT				g_fnorg_DeleteObject;

	extern GETGUITHREADINFO			g_fnorg_GetGUIThreadInfo;

	extern SYSFREESTRING			g_fnorg_SysFreeString;

	extern SYSALLOCSTRING			g_fnorg_SysAllocString;

	extern nudf::util::CObligations *query_engine(const WCHAR *FileName, ULONGLONG EvaluationId);

	extern LRESULT CALLBACK			Office2010WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR	uIdSubclass, DWORD_PTR dwRefData);

	extern 	ID2D1BitmapBrush*		build_bitmap_brush(ID2D1DCRenderTarget *pRT, const WCHAR *FileName);
	
	extern BOOL						send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

	extern BOOL						Office_Update_WaterMarked_Document_List(HWND hWnd);

	extern BOOL						Office_Refresh_And_Remove_WaterMarked_Document_List(void);

#ifdef __cplusplus
}
#endif


PowerPointEventListener::PowerPointEventListener()
{
	m_uRefCount = 0;
}

PowerPointEventListener::~PowerPointEventListener()
{

}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */ 
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppvObject = NULL;

	do 
	{
		if (__uuidof(PowerPoint::EApplication) == riid)
		{
			punk = (PowerPoint::EApplication*)this;
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

ULONG STDMETHODCALLTYPE PowerPointEventListener::AddRef(void)
{
	m_uRefCount++;

	return m_uRefCount;
}

ULONG STDMETHODCALLTYPE PowerPointEventListener::Release(void)
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

HRESULT STDMETHODCALLTYPE PowerPointEventListener::GetTypeInfoCount( 
	/* [out] */ __RPC__out UINT *pctinfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::GetTypeInfo( 
	/* [in] */ UINT iTInfo,
	/* [in] */ LCID lcid,
	/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::GetIDsOfNames( 
	/* [in] */ __RPC__in REFIID riid,
	/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
	/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
	/* [in] */ LCID lcid,
	/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::Invoke( 
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

	void *Pres = NULL;
	void *Wn = NULL;
	void *PvWindow = NULL;

	VARIANT_BOOL *Cancel = NULL;

	switch (dispIdMember)
	{
	case WindowActivate_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Pres = (void*)pDispParams->rgvarg[i].pdispVal;
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
			Pres = (void*)pDispParams->rgvarg[1].pdispVal;
			Wn = (void *)pDispParams->rgvarg[0].pdispVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WindowActivate((PowerPoint2010::_Presentation*)Pres, (PowerPoint2010::DocumentWindow *)Wn);
		}
		else
		{
			hr = WindowActivate((PowerPoint::_Presentation*)Pres, (PowerPoint::DocumentWindow *)Wn);
		}

		break;
	case WindowDeactivate_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Pres = (void*)pDispParams->rgvarg[i].pdispVal;
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
			Pres = (void*)pDispParams->rgvarg[1].pdispVal;
			Wn = (void *)pDispParams->rgvarg[0].pdispVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WindowDeactivate((PowerPoint2010::_Presentation*)Pres, (PowerPoint2010::DocumentWindow *)Wn);
		}
		else
		{
			hr = WindowDeactivate((PowerPoint::_Presentation*)Pres, (PowerPoint::DocumentWindow *)Wn);
		}

		break;


	case PresentationSave_Id:
		
		Pres = (void*)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = PresentationSave((PowerPoint2010::_Presentation *)Pres);
		}
		else
		{
			hr = PresentationSave((PowerPoint::_Presentation *)Pres);
		}

		break;

	case PresentationBeforeSave_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Pres = (void*)pDispParams->rgvarg[i].pdispVal;
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
			Pres = (void*)pDispParams->rgvarg[1].pdispVal;
			Cancel = pDispParams->rgvarg[0].pboolVal;
		}

		if (Global.IsOffice2010)
		{
			hr = PresentationBeforeSave((PowerPoint2010::_Presentation *)Pres, Cancel);
		}
		else
		{
			hr = PresentationBeforeSave((PowerPoint::_Presentation *)Pres, Cancel);
		}

		break;

	case ProtectedViewWindowActivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = ProtectedViewWindowActivate((PowerPoint2010::ProtectedViewWindow *)PvWindow);
		}
		else
		{
			hr = ProtectedViewWindowActivate((PowerPoint::ProtectedViewWindow *)PvWindow);
		}

		break;

	case ProtectedViewWindowDeactivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = ProtectedViewWindowDeactivate((PowerPoint2010::ProtectedViewWindow *)PvWindow);
		}
		else
		{
			hr = ProtectedViewWindowDeactivate((PowerPoint::ProtectedViewWindow *)PvWindow);
		}

		break;

	case PresentationClose_Id:

		Pres = (void*)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = PresentationClose((PowerPoint2010::_Presentation *)Pres);
		}
		else
		{
			hr = PresentationClose((PowerPoint::_Presentation *)Pres);
		}

		break;

	case AfterNewPresentation_Id:

		Pres = (void*)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = AfterNewPresentation((PowerPoint2010::_Presentation *)Pres);
		}
		else
		{
			hr = AfterNewPresentation((PowerPoint::_Presentation *)Pres);
		}

		break;
	
	case SlideShowBegin_Id:

		Wn = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = SlideShowBegin((PowerPoint2010::SlideShowWindow *)Wn);
		}
		else
		{
			hr = SlideShowBegin((PowerPoint::SlideShowWindow *)Wn);
		}

		break;

	default:
		break;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowSelectionChange (
/*[in]*/ struct PowerPoint::Selection * Sel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowBeforeRightClick (
/*[in]*/ struct PowerPoint::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowBeforeDoubleClick (
/*[in]*/ struct PowerPoint::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationClose (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	PowerPoint::_Application *pAppObj = NULL;

	long hWnd = 0;

	do 
	{
		if (Global.OfficeMode == OFFICE_MODE_DDE)
		{
			Global.OfficeMode = OFFICE_MODE_NORMAL;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			memset(Global.DDEDocFileName, 0 , sizeof(Global.DDEDocFileName));

			LeaveCriticalSection(&Global.DDEDocFileNameLock);
		}

		hr = Global.PowerPointApp->QueryInterface(__uuidof(PowerPoint::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = pAppObj->get_HWND(&hWnd);

		if (FAILED(hr))
		{
			break;
		}

		if (!g_fnorg_SetWindowDisplayAffinity((HWND)(ULONG_PTR)hWnd, WDA_NONE))
		{

		}

	} while (FALSE);

	if (pAppObj)
	{
		pAppObj->Release();
		pAppObj = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationSave (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationOpen (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::NewPresentation (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationNewSlide (
/*[in]*/ struct PowerPoint::_Slide * Sld )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowActivate (
/*[in]*/ struct PowerPoint::_Presentation * Pres,
/*[in]*/ struct PowerPoint::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	PowerPoint::_Application	*pAppObj = NULL;

	long hWnd = 0;

	BSTR PptFullName = NULL;

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
		hr = Global.PowerPointApp->QueryInterface(__uuidof(PowerPoint::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!PptFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   PptFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		Office_Refresh_And_Remove_WaterMarked_Document_List();

		if (!init_rm_section_safe())
		{
			break;
		}

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(PptFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(PptFullName);

				PptFullName = NULL;

				PptFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!PptFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   PptFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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

		hr = pAppObj->get_HWND(&hWnd);

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
		Obligations = query_engine(PptFullName, EvaluationId);

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
	}

	if (Obligations)
	{
		delete Obligations;
	}

	if (PptFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(PptFullName);
		PptFullName = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowDeactivate (
/*[in]*/ struct PowerPoint::_Presentation * Pres,
/*[in]*/ struct PowerPoint::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	//long hDocWnd = 0;

	//do 
	//{
	//	hr = Wn->get_HWND(&hDocWnd);

	//	if (FAILED(hr))
	//	{
	//		break;
	//	}

	//} while (FALSE);

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowBegin (
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	PowerPoint::_Presentation * Pres = NULL;

	long hWnd = 0;

	HWND hDocWnd = NULL;

	BSTR PptFullName = NULL;

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
		hr = Wn->get_Presentation(&Pres);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!Pres)
		{
			break;
		}

		Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!PptFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   PptFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (!init_rm_section_safe())
		{
			break;
		}

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(PptFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(PptFullName);

				PptFullName = NULL;

				PptFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!PptFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   PptFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		hr = Wn->get_HWND(&hWnd);

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
		Obligations = query_engine(PptFullName, EvaluationId);

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

	if (Obligations)
	{
		delete Obligations;
	}

	if (Pres)
	{
		Pres->Release();
		Pres = NULL;
	}

	if (PptFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(PptFullName);
		PptFullName = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextBuild (
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextSlide (
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}
HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowEnd (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}
HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationPrint (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideSelectionChanged (
/*[in]*/ struct PowerPoint::SlideRange * SldRange )
{
	HRESULT hr = S_OK;

	return hr;
}
HRESULT STDMETHODCALLTYPE PowerPointEventListener::ColorSchemeChanged (
/*[in]*/ struct PowerPoint::SlideRange * SldRange )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationBeforeSave (
/*[in]*/ struct PowerPoint::_Presentation * Pres,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		hr = Pres->get_FullName(&DocFullName);

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
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   DocFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(DocFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		//
		// don't have extract right and it's SaveAs
		//
		if (!(RightsMask & BUILTIN_RIGHT_SAVEAS) && !(RightsMask & BUILTIN_RIGHT_EDIT))
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

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextClick (
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn,
/*[in]*/ struct PowerPoint::Effect * nEffect )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterNewPresentation (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterPresentationOpen (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationSync (
/*[in]*/ struct PowerPoint::_Presentation * Pres,
	/*[in]*/ enum Office::MsoSyncEventType SyncEventType )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowOnNext (
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowOnPrevious (
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationBeforeClose (
/*[in]*/ struct PowerPoint::_Presentation * Pres,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowOpen (
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowBeforeEdit (
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowBeforeClose (
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow,
	/*[in]*/ enum PowerPoint::PpProtectedViewCloseReason ProtectedViewCloseReason,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowActivate (
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	PowerPoint::_Application	*pAppObj = NULL;

	long hWnd = 0;

	BSTR PptFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	BOOL ClearGlobalActiveWnd = TRUE;

	PowerPoint::_Presentation *Pres = NULL;

	do 
	{
		hr = Global.PowerPointApp->QueryInterface(__uuidof(PowerPoint::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = ProtViewWindow->get_Presentation(&Pres);

		if (!SUCCEEDED(hr) || Pres == NULL)
		{
			break;
		}

		hr = Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!PptFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   PptFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		if (!init_rm_section_safe())
		{
			break;
		}

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(PptFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(PptFullName);

				PptFullName = NULL;

				PptFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!PptFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   PptFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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

		hr = pAppObj->get_HWND(&hWnd);

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
		Obligations = query_engine(PptFullName, EvaluationId);

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
	}

	if (Obligations)
	{
		delete Obligations;
	}

	if (PptFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(PptFullName);
		PptFullName = NULL;
	}

	if (Pres)
	{
		Pres->Release();
		Pres = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowDeactivate (
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationCloseFinal (
/*[in]*/ struct PowerPoint::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterDragDropOnSlide (
/*[in]*/ struct PowerPoint::_Slide * Sld,
	/*[in]*/ float X,
	/*[in]*/ float Y )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterShapeSizeChange (
/*[in]*/ struct PowerPoint::Shape * shp )
{
	HRESULT hr = S_OK;

	return hr;
}

//////////////////////////////////////////////////////////////////////////
//
// Office 2010
//
//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowSelectionChange (
/*[in]*/ struct PowerPoint2010::Selection * Sel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowBeforeRightClick (
/*[in]*/ struct PowerPoint2010::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowBeforeDoubleClick (
/*[in]*/ struct PowerPoint2010::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationClose (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	PowerPoint2010::_Application	*pAppObj = NULL;

	long hWnd = 0;

	do 
	{
		if (Global.OfficeMode == OFFICE_MODE_DDE)
		{
			Global.OfficeMode = OFFICE_MODE_NORMAL;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			memset(Global.DDEDocFileName, 0 , sizeof(Global.DDEDocFileName));

			LeaveCriticalSection(&Global.DDEDocFileNameLock);
		}

		hr = Global.PowerPointApp->QueryInterface(__uuidof(PowerPoint2010::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = pAppObj->get_HWND(&hWnd);

		if (FAILED(hr))
		{
			break;
		}

		if (!g_fnorg_SetWindowDisplayAffinity((HWND)(ULONG_PTR)hWnd, WDA_NONE))
		{

		}

	} while (FALSE);

	if (pAppObj)
	{
		pAppObj->Release();
		pAppObj = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationSave (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationOpen (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::NewPresentation (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationNewSlide (
/*[in]*/ struct PowerPoint2010::_Slide * Sld )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowActivate (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
/*[in]*/ struct PowerPoint2010::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	PowerPoint2010::_Application	*pAppObj = NULL;

	long hWnd = 0;

	HWND hDocWnd = NULL;

	BSTR PptFullName = NULL;

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
		hr = Global.PowerPointApp->QueryInterface(__uuidof(PowerPoint2010::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!PptFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   PptFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

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

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(PptFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(PptFullName);

				PptFullName = NULL;

				PptFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!PptFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   PptFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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
			hr = pAppObj->get_HWND(&hWnd);

			if (SUCCEEDED(hr))
			{
				if (!g_fnorg_SetWindowDisplayAffinity((HWND)(ULONG_PTR)hWnd, WDA_MONITOR))
				{

				}
			}
		}

		//
		// take care of obligations
		//
		Obligations = query_engine(PptFullName, EvaluationId);

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

		hr = pAppObj->get_HWND(&hWnd);

		if (SUCCEEDED(hr))
		{
			Office_Update_WaterMarked_Document_List((HWND)(ULONG_PTR)hWnd);
		}

	} while (FALSE);

	if (pAppObj)
	{
		pAppObj->Release();
	}

	if (Obligations)
	{
		delete Obligations;
	}

	if (PptFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(PptFullName);
		PptFullName = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowDeactivate (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
/*[in]*/ struct PowerPoint2010::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowBegin (
/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	PowerPoint2010::_Presentation * Pres = NULL;

	long hWnd = 0;

	HWND hDocWnd = NULL;

	BSTR PptFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	GUITHREADINFO GuiThreadInfo = { 0 };

	do 
	{
		hr = Wn->get_Presentation(&Pres);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!Pres)
		{
			break;
		}

		Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!PptFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   PptFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

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

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(PptFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(PptFullName);

				PptFullName = NULL;

				PptFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!PptFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   PptFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		hr = Wn->get_HWND(&hWnd);

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
		Obligations = query_engine(PptFullName, EvaluationId);

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

		Office_Update_WaterMarked_Document_List((HWND)(ULONG_PTR)hWnd);

	} while (FALSE);

	if (Pres)
	{
		Pres->Release();
		Pres = NULL;
	}

	if (Obligations)
	{
		delete Obligations;
		Obligations = NULL;
	}

	if (PptFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(PptFullName);
		PptFullName = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextBuild (
/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextSlide (
/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowEnd (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationPrint (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideSelectionChanged (
/*[in]*/ struct PowerPoint2010::SlideRange * SldRange )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ColorSchemeChanged (
/*[in]*/ struct PowerPoint2010::SlideRange * SldRange )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationBeforeSave (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		hr = Pres->get_FullName(&DocFullName);

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
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   DocFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(DocFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		//
		// don't have extract right and it's SaveAs
		//
		if (!(RightsMask & BUILTIN_RIGHT_SAVEAS) && !(RightsMask & BUILTIN_RIGHT_EDIT))
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

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextClick (
/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn,
/*[in]*/ struct PowerPoint2010::Effect * nEffect )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterNewPresentation (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterPresentationOpen (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationSync (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
	/*[in]*/ enum Office::MsoSyncEventType SyncEventType )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowOnNext (
/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowOnPrevious (
/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationBeforeClose (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowOpen (
/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowBeforeEdit (
/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowBeforeClose (
/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow,
	/*[in]*/ enum PowerPoint2010::PpProtectedViewCloseReason ProtectedViewCloseReason,
	/*[in,out]*/ VARIANT_BOOL * Cancel )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowActivate (
/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	PowerPoint2010::_Application	*pAppObj = NULL;

	long hWnd = 0;

	HWND hDocWnd = NULL;

	BSTR PptFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	GUITHREADINFO GuiThreadInfo = {0};

	PowerPoint2010::_Presentation *Pres = NULL;

	do 
	{
		hr = Global.PowerPointApp->QueryInterface(__uuidof(PowerPoint2010::_Application), (void**)&pAppObj);

		if (FAILED(hr))
		{
			break;
		}

		hr = ProtViewWindow->get_Presentation(&Pres);

		if (!SUCCEEDED(hr) || Pres == NULL)
		{
			break;
		}

		hr = Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new PPT
		//
		if (!PptFullName)
		{
			break;
		}

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   PptFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

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

		if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(PptFullName) && 
			Global.OfficeMode == OFFICE_MODE_DDE &&
			g_fnorg_SysFreeString &&
			g_fnorg_SysAllocString)
		{
			BOOL bSkipUpdateActiveDoc = FALSE;

			EnterCriticalSection(&Global.DDEDocFileNameLock);

			if (wcslen(Global.DDEDocFileName))
			{
				g_fnorg_SysFreeString(PptFullName);

				PptFullName = NULL;

				PptFullName = g_fnorg_SysAllocString(Global.DDEDocFileName);

			}
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!PptFullName || bSkipUpdateActiveDoc)
			{
				break;
			}

			//
			// update ActiveDocFileName
			//
			EnterCriticalSection(&Global.ActiveDocFileNameLock);

			memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

			memcpy(Global.ActiveDocFileName,
				   PptFullName,
				   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(PptFullName)*sizeof(WCHAR)));

			LeaveCriticalSection(&Global.ActiveDocFileNameLock);
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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
			hr = pAppObj->get_HWND(&hWnd);

			if (SUCCEEDED(hr))
			{
				if (!g_fnorg_SetWindowDisplayAffinity((HWND)(ULONG_PTR)hWnd, WDA_MONITOR))
				{

				}
			}
		}

		//
		// take care of obligations
		//
		Obligations = query_engine(PptFullName, EvaluationId);

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

		hr = ProtViewWindow->get_HWND(&hWnd);

		if (!SUCCEEDED(hr))
		{
			break;
		}
		
		Office_Update_WaterMarked_Document_List((HWND)(ULONG_PTR)hWnd);

	} while (FALSE);

	if (pAppObj)
	{
		pAppObj->Release();
	}

	if (Obligations)
	{
		delete Obligations;
	}

	if (PptFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(PptFullName);
		PptFullName = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowDeactivate (
/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationCloseFinal (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterDragDropOnSlide (
/*[in]*/ struct PowerPoint2010::_Slide * Sld,
	/*[in]*/ float X,
	/*[in]*/ float Y )
{
	HRESULT hr = S_OK;

	return hr;
}
HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterShapeSizeChange (
/*[in]*/ struct PowerPoint2010::Shape * shp )
{
	HRESULT hr = S_OK;

	return hr;
}
