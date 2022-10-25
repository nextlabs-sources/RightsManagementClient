#include "stdafx.h"
#include "mso.h"
#include "word.h"
#include "wordevents.h"
#include "nxrmcoreglobal.h"
#include "office.h"
#include "screenoverlay.h"
#include "direct2d.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA			Global;

	extern SETWINDOWDISPLAYAFFINITY	g_fnorg_SetWindowDisplayAffinity;

	extern GETWINDOWDISPLAYAFFINITY	g_fnorg_GetWindowDisplayAffinity;

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

	extern ID2D1BitmapBrush*		build_bitmap_brush(ID2D1DCRenderTarget *pRT, const WCHAR *FileName);

	extern BOOL						send_block_notification(const WCHAR *FileName, BLOCK_NOTIFICATION_TYPE Type);

	extern BOOL						Office_Update_WaterMarked_Document_List(HWND hWnd);

	extern BOOL						Office_Refresh_And_Remove_WaterMarked_Document_List(void);

#ifdef __cplusplus
}
#endif

//BOOL IsChildOfActiveWindowNotSafe(HWND hWnd, OFFICE_CHILDWINDOWNODE *pNode);

WordEventListener::WordEventListener()
{
	m_uRefCount = 0;
};

WordEventListener::~WordEventListener()
{

}

HRESULT STDMETHODCALLTYPE WordEventListener::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */ 
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppvObject = NULL;

	do 
	{
		if (__uuidof(Word::ApplicationEvents4) == riid)
		{
			punk = (Word::ApplicationEvents4*)this;
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

ULONG STDMETHODCALLTYPE WordEventListener::AddRef(void)
{
	m_uRefCount++;

	return m_uRefCount;
}

ULONG STDMETHODCALLTYPE WordEventListener::Release(void)
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

HRESULT STDMETHODCALLTYPE WordEventListener::GetTypeInfoCount( 
	/* [out] */ __RPC__out UINT *pctinfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WordEventListener::GetTypeInfo( 
	/* [in] */ UINT iTInfo,
	/* [in] */ LCID lcid,
	/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WordEventListener::GetIDsOfNames( 
	/* [in] */ __RPC__in REFIID riid,
	/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
	/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
	/* [in] */ LCID lcid,
	/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE WordEventListener::Invoke( 
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

	void *Doc = NULL;
	void *Wn = NULL;
	void *PvWindow = NULL;

	VARIANT_BOOL *Cancel = NULL;
	VARIANT_BOOL *SaveAsUI = NULL;

	switch (dispIdMember)
	{
	case WindowActivate_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Doc = (void*)pDispParams->rgvarg[i].pdispVal;
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
			Doc = (void*)pDispParams->rgvarg[1].pdispVal;
			Wn = (void *)pDispParams->rgvarg[0].pdispVal;
		}

		if (Global.IsOffice2010)
		{
			hr = WindowActivate((Word2010::_Document*)Doc, (Word2010::Window*)Wn);
		}
		else
		{
			hr = WindowActivate((Word::_Document*)Doc, (Word::Window*)Wn);
		}

		break;
	case WindowDeactivate_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Doc = (void*)pDispParams->rgvarg[i].pdispVal;
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
			Doc = (void*)pDispParams->rgvarg[1].pdispVal;
			Wn = (void*)pDispParams->rgvarg[0].pdispVal;
		}
		
		if (Global.IsOffice2010)
		{
			hr = WindowDeactivate((Word2010::_Document*)Doc, (Word2010::Window*)Wn);
		}
		else
		{
			hr = WindowDeactivate((Word::_Document*)Doc, (Word::Window*)Wn);
		}

		break;

	case DocumentBeforeSave_Id:
		
		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 3); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Doc = (void*)pDispParams->rgvarg[i].pdispVal;
				}

				if (pDispParams->rgdispidNamedArgs[i] == 1)
				{
					SaveAsUI = pDispParams->rgvarg[i].pboolVal;
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
			Doc = (void*)pDispParams->rgvarg[2].pdispVal;
			SaveAsUI = pDispParams->rgvarg[1].pboolVal;
			Cancel = pDispParams->rgvarg[0].pboolVal;
		}

		if (Global.IsOffice2010)
		{
			hr = DocumentBeforeSave((Word2010::_Document*)Doc, SaveAsUI, Cancel);
		}
		else
		{
			hr = DocumentBeforeSave((Word::_Document*)Doc, SaveAsUI, Cancel);
		}

		break;

	case DocumentBeforePrint_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Doc = (void*)pDispParams->rgvarg[i].pdispVal;
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
			Doc = (void*)pDispParams->rgvarg[1].pdispVal;
			Cancel = pDispParams->rgvarg[0].pboolVal;
		}

		if (Global.IsOffice2010)
		{
			hr = DocumentBeforePrint((Word2010::_Document*)Doc, Cancel);
		}
		else
		{
			hr = DocumentBeforePrint((Word::_Document*)Doc, Cancel);
		}

		break;

	case ProtectedViewWindowActivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = ProtectedViewWindowActivate((Word2010::ProtectedViewWindow*)PvWindow);
		}
		else
		{
			hr = ProtectedViewWindowActivate((Word::ProtectedViewWindow*)PvWindow);
		}

		break;
	case ProtectedViewWindowDeactivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = ProtectedViewWindowDeactivate((Word2010::ProtectedViewWindow*)PvWindow);
		}
		else
		{
			hr = ProtectedViewWindowDeactivate((Word::ProtectedViewWindow*)PvWindow);
		}

		break;
	case DocumentBeforeClose_Id:

		if (pDispParams->rgdispidNamedArgs)
		{
			for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
			{
				if (pDispParams->rgdispidNamedArgs[i] == 0)
				{
					Doc = (void*)pDispParams->rgvarg[i].pdispVal;
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
			Doc = (void*)pDispParams->rgvarg[1].pdispVal;
			Cancel = pDispParams->rgvarg[0].pboolVal;
		}

		if (Global.IsOffice2010)
		{
			hr = DocumentBeforeClose((Word2010::_Document*)Doc, Cancel);
		}
		else
		{
			hr = DocumentBeforeClose((Word::_Document*)Doc, Cancel);
		}

		break;

	case NewDocument_Id:

		Doc = (void *)pDispParams->rgvarg[0].pdispVal;

		if (Global.IsOffice2010)
		{
			hr = NewDocument((Word2010::_Document*)Doc);
		}
		else
		{
			hr = NewDocument((Word::_Document*)Doc);
		}

		break;

	case DocumentChange_Id:

		hr = DocumentChange();

		break;

	default:
		break;
	}

	return hr;
}

HRESULT __stdcall WordEventListener::Startup()
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::Quit()
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentChange()
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentOpen(
/*[in]*/ struct Word::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentBeforeClose(
/*[in]*/ struct Word::_Document * Doc,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	Word::Window *Wn = NULL;

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

		hr = Doc->get_ActiveWindow(&Wn);

		if (FAILED(hr))
		{
			break;
		}
		
		hr = Wn->get_Hwnd(&hWnd);

		if (FAILED(hr))
		{
			break;
		}

		if (!g_fnorg_SetWindowDisplayAffinity((HWND)(ULONG_PTR)hWnd, WDA_NONE))
		{

		}

	} while (FALSE);

	if (Wn)
	{
		Wn->Release();
		Wn = NULL;
	}

	return hr;
}


HRESULT __stdcall WordEventListener::DocumentBeforePrint(
/*[in]*/ struct Word::_Document * Doc,
/*[in,out]*/ VARIANT_BOOL * Cancel)
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

			Doc->get_FullName(&DocFullName);

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

HRESULT __stdcall WordEventListener::DocumentBeforeSave(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ VARIANT_BOOL * SaveAsUI,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		hr = Doc->get_FullName(&DocFullName);

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
		// don't have edit right and it's SaveAs
		//
		if (!(RightsMask & BUILTIN_RIGHT_EDIT) && (*SaveAsUI == VARIANT_FALSE))
		{
			*Cancel = VARIANT_TRUE;
		}
		else if (!(RightsMask & BUILTIN_RIGHT_SAVEAS) && (*SaveAsUI == VARIANT_TRUE))
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

HRESULT __stdcall WordEventListener::NewDocument(
/*[in]*/ struct Word::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowActivate(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ struct Word::Window * Wn)
{
	HRESULT hr = S_OK;

	long hWnd = 0;

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
		hr = Doc->get_FullName(&DocFullName);

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

		//
		// update ActiveDocFileName
		//
		EnterCriticalSection(&Global.ActiveDocFileNameLock);

		memset(Global.ActiveDocFileName, 0, sizeof(Global.ActiveDocFileName));

		memcpy(Global.ActiveDocFileName,
			   DocFullName,
			   min(sizeof(Global.ActiveDocFileName) - sizeof(WCHAR), wcslen(DocFullName)*sizeof(WCHAR)));

		LeaveCriticalSection(&Global.ActiveDocFileNameLock);

		Office_Refresh_And_Remove_WaterMarked_Document_List();

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
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!DocFullName || bSkipUpdateActiveDoc)
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

		hr = Wn->get_Hwnd(&hWnd);

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

	if (Obligations)
	{
		delete Obligations;
	}

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

HRESULT __stdcall WordEventListener::WindowDeactivate(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ struct Word::Window * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowSelectionChange(
/*[in]*/ struct Word::Selection * Sel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowBeforeRightClick(
/*[in]*/ struct Word::Selection * Sel,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowBeforeDoubleClick(
/*[in]*/ struct Word::Selection * Sel,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::EPostagePropertyDialog(
/*[in]*/ struct Word::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::EPostageInsert(
/*[in]*/ struct Word::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeAfterMerge(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ struct Word::_Document * DocResult)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeAfterRecordMerge(
/*[in]*/ struct Word::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeBeforeMerge(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ long StartRecord,
/*[in]*/ long EndRecord,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeBeforeRecordMerge(
/*[in]*/ struct Word::_Document * Doc,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeDataSourceLoad(
/*[in]*/ struct Word::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeDataSourceValidate(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ VARIANT_BOOL * Handled)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeWizardSendToCustom(
/*[in]*/ struct Word::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeWizardStateChange(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ int * FromState,
/*[in]*/ int * ToState,
/*[in]*/ VARIANT_BOOL * Handled)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowSize(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ struct Word::Window * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::XMLSelectionChange(
/*[in]*/ struct Word::Selection * Sel,
/*[in]*/ struct Word::XMLNode * OldXMLNode,
/*[in]*/ struct Word::XMLNode * NewXMLNode,
/*[in]*/ long * Reason)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::XMLValidationError(
/*[in]*/ struct Word::XMLNode * XMLNode)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentSync(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ enum Office::MsoSyncEventType SyncEventType)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::EPostageInsertEx(
/*[in]*/ struct Word::_Document * Doc,
/*[in]*/ int cpDeliveryAddrStart,
/*[in]*/ int cpDeliveryAddrEnd,
/*[in]*/ int cpReturnAddrStart,
/*[in]*/ int cpReturnAddrEnd,
/*[in]*/ int xaWidth,
/*[in]*/ int yaHeight,
/*[in]*/ BSTR bstrPrinterName,
/*[in]*/ BSTR bstrPaperFeed,
/*[in]*/ VARIANT_BOOL fPrint,
/*[in,out]*/ VARIANT_BOOL * fCancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeDataSourceValidate2(
/*[in]*/ struct Word::_Document * Doc,
/*[in,out]*/ VARIANT_BOOL * Handled)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowOpen(
/*[in]*/ struct Word::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowBeforeEdit(
/*[in]*/ struct Word::ProtectedViewWindow * PvWindow,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowBeforeClose(
/*[in]*/ struct Word::ProtectedViewWindow * PvWindow,
/*[in]*/ int CloseReason,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowSize(
/*[in]*/ struct Word::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowActivate(
/*[in]*/ struct Word::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	long hWnd = 0;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	BOOL ClearGlobalActiveWnd = TRUE;

	Word::_Document *Doc = NULL;

	Word::Window *Wn = NULL;

	do 
	{
		hr = PvWindow->get_Document(&Doc);
		
		if (!SUCCEEDED(hr) || Doc == NULL)
		{
			break;
		}
		
		hr = Doc->get_ActiveWindow(&Wn);

		if (!SUCCEEDED(hr) || Wn == NULL)
		{
			break;
		}

		hr = Doc->get_FullName(&DocFullName);

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
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!DocFullName || bSkipUpdateActiveDoc)
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

		hr = Wn->get_Hwnd(&hWnd);

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

	if (Obligations)
	{
		delete Obligations;
	}

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowDeactivate(
/*[in]*/ struct Word::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentOpen(
/*[in]*/ struct Word2010::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentBeforeClose(
/*[in]*/ struct Word2010::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
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

HRESULT __stdcall WordEventListener::DocumentBeforePrint(
/*[in]*/ struct Word2010::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
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
			Doc->get_FullName(&DocFullName);

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

HRESULT __stdcall WordEventListener::DocumentBeforeSave(
/*[in]*/ struct Word2010::_Document * Doc,
	/*[in]*/ VARIANT_BOOL * SaveAsUI,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		hr = Doc->get_FullName(&DocFullName);

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
		// don't have edit right and it's SaveAs
		//
		if (!(RightsMask & BUILTIN_RIGHT_EDIT) && (*SaveAsUI == VARIANT_FALSE))
		{
			*Cancel = VARIANT_TRUE;
		}
		else if (!(RightsMask & BUILTIN_RIGHT_SAVEAS) && (*SaveAsUI == VARIANT_TRUE))
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

HRESULT __stdcall WordEventListener::NewDocument(
/*[in]*/ struct Word2010::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowActivate(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ struct Word2010::Window * Wn)
{
	HRESULT hr = S_OK;

	HWND hWnd = 0;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	GUITHREADINFO GuiThreadInfo = {0};

	LIST_ENTRY *ite = NULL;

	do 
	{
		hr = Doc->get_FullName(&DocFullName);

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

		GuiThreadInfo.cbSize = sizeof(GuiThreadInfo);

		if (!g_fnorg_GetGUIThreadInfo(GetCurrentThreadId(), &GuiThreadInfo))
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
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!DocFullName || bSkipUpdateActiveDoc)
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

		if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
		{
			g_fnorg_SetWindowDisplayAffinity(GuiThreadInfo.hwndActive, WDA_MONITOR);
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

		hWnd = GuiThreadInfo.hwndActive;

		Office_Update_WaterMarked_Document_List(hWnd);

	} while (FALSE);

	if (Obligations)
	{
		delete Obligations;
	}

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

HRESULT __stdcall WordEventListener::WindowDeactivate(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ struct Word2010::Window * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowSelectionChange(
/*[in]*/ struct Word2010::Selection * Sel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowBeforeRightClick(
/*[in]*/ struct Word2010::Selection * Sel,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}


HRESULT __stdcall WordEventListener::WindowBeforeDoubleClick(
/*[in]*/ struct Word2010::Selection * Sel,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::EPostagePropertyDialog(
/*[in]*/ struct Word2010::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::EPostageInsert(
/*[in]*/ struct Word2010::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeAfterMerge(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ struct Word2010::_Document * DocResult)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeAfterRecordMerge(
/*[in]*/ struct Word2010::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeBeforeMerge(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ long StartRecord,
/*[in]*/ long EndRecord,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeBeforeRecordMerge(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeDataSourceLoad(
/*[in]*/ struct Word2010::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeDataSourceValidate(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ VARIANT_BOOL * Handled)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeWizardSendToCustom(
/*[in]*/ struct Word2010::_Document * Doc)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::MailMergeWizardStateChange(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ int * FromState,
/*[in]*/ int * ToState,
/*[in]*/ VARIANT_BOOL * Handled)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowSize(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ struct Word2010::Window * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::XMLSelectionChange(
/*[in]*/ struct Word2010::Selection * Sel,
/*[in]*/ struct Word2010::XMLNode * OldXMLNode,
/*[in]*/ struct Word2010::XMLNode * NewXMLNode,
/*[in]*/ long * Reason)
{
	HRESULT hr = S_OK;

	return hr;
}


HRESULT __stdcall WordEventListener::XMLValidationError(
/*[in]*/ struct Word2010::XMLNode * XMLNode)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentSync(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ enum Office::MsoSyncEventType SyncEventType)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::EPostageInsertEx(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ int cpDeliveryAddrStart,
/*[in]*/ int cpDeliveryAddrEnd,
/*[in]*/ int cpReturnAddrStart,
/*[in]*/ int cpReturnAddrEnd,
/*[in]*/ int xaWidth,
/*[in]*/ int yaHeight,
/*[in]*/ BSTR bstrPrinterName,
/*[in]*/ BSTR bstrPaperFeed,
/*[in]*/ VARIANT_BOOL fPrint,
/*[in,out]*/ VARIANT_BOOL * fCancel)
{
	HRESULT hr = S_OK;

	return hr;
}


HRESULT __stdcall WordEventListener::MailMergeDataSourceValidate2(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in,out]*/ VARIANT_BOOL * Handled)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowOpen(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowBeforeEdit(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowBeforeClose(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow,
	/*[in]*/ int CloseReason,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowSize(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowActivate(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	HWND hWnd = 0;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	GUITHREADINFO GuiThreadInfo = {0};

	LIST_ENTRY *ite = NULL;

	Word2010::_Document *Doc = NULL;
	
	do 
	{
		hr = PvWindow->get_Document(&Doc);

		if (!SUCCEEDED(hr) || Doc == NULL)
		{
			break;
		}

		hr = Doc->get_FullName(&DocFullName);

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

		GuiThreadInfo.cbSize = sizeof(GuiThreadInfo);

		if (!g_fnorg_GetGUIThreadInfo(GetCurrentThreadId(), &GuiThreadInfo))
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
			else
			{
				bSkipUpdateActiveDoc = TRUE;
			}

			LeaveCriticalSection(&Global.DDEDocFileNameLock);

			if (!DocFullName || bSkipUpdateActiveDoc)
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

		if (!(RightsMask & BUILTIN_RIGHT_SCREENCAP))
		{
			g_fnorg_SetWindowDisplayAffinity(GuiThreadInfo.hwndActive, WDA_MONITOR);
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

		hWnd = GuiThreadInfo.hwndActive;

		Office_Update_WaterMarked_Document_List(hWnd);

	} while (FALSE);

	if (Obligations)
	{
		delete Obligations;
	}

	if (DocFullName && g_fnorg_SysFreeString)
	{
		g_fnorg_SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	if (Doc)
	{
		Doc->Release();
		Doc = NULL;
	}

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowDeactivate(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

