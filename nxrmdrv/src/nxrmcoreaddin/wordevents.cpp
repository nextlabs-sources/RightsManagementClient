#include "stdafx.h"
#include "mso.h"
#include "word.h"
#include "wordevents.h"
#include "nxrmcoreext2.h"

extern CACHE_ALIGN	BOOL	g_bIsOffice2010;

extern BOOL init_rm_section_safe(void);

extern "C" const std::vector<RIBBON_ID_INFO> g_word_14_ribbon_info;
extern "C" const std::vector<RIBBON_ID_INFO> g_word_15_ribbon_info;

WordEventListener::WordEventListener()
{
	m_uRefCount = 0;
	m_ActiveDocRights = BUILTIN_RIGHT_ALL;
	m_pRibbonUI = NULL;
	m_InvalidCount = 0;
};

WordEventListener::WordEventListener(IDispatch *pRibbonUI, BSTR ActiveDoc, ULONGLONG &ActiveRights)
{
	m_uRefCount = 0;
	m_pRibbonUI = pRibbonUI;
	m_ActiveDocRights = ActiveRights;
	m_ActiveDoc = ActiveDoc ? ActiveDoc : L"";
	m_InvalidCount = 0;
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

		if (g_bIsOffice2010)
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
		
		if (g_bIsOffice2010)
		{
			hr = WindowDeactivate((Word2010::_Document*)Doc, (Word2010::Window*)Wn);
		}
		else
		{
			hr = WindowDeactivate((Word::_Document*)Doc, (Word::Window*)Wn);
		}

		break;

	case ProtectedViewWindowActivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
		{
			hr = DocumentBeforeClose((Word2010::_Document*)Doc, Cancel);
		}
		else
		{
			hr = DocumentBeforeClose((Word::_Document*)Doc, Cancel);
		}

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

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentBeforePrint(
/*[in]*/ struct Word::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentBeforeSave(
/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ VARIANT_BOOL * SaveAsUI,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

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

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do
	{
		hr = Doc->get_FullName(&DocFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new workbook
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
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvlauationId);

		if (FAILED(hr))
		{
			break;
		}

	} while (FALSE);

	//
	// lock the m_ActiveDocLock while updating m_ActiveDoc and m_ActiveDocRights
	//
	{
		nudf::util::CRwExclusiveLocker lock(&m_ActiveDocLock);

		if (DocFullName)
		{
			m_ActiveDoc = DocFullName;

			SysFreeString(DocFullName);
			DocFullName = NULL;
		}
		else
		{
			m_ActiveDoc.clear();
		}

		if (m_ActiveDocRights != RightsMask)
		{
			UpdateRibbonUI = TRUE;
			m_ActiveDocRights = RightsMask;
		}
	}

	if (UpdateRibbonUI || m_InvalidCount == 0)
	{
		InvalidMsoControls();
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

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	Word::_Document *Doc = NULL;

	BOOL UpdateRibbonUI = FALSE;

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
			break;
		}

	} while (FALSE);

	if (Doc)
	{
		Doc->Release();
		Doc = NULL;
	}

	//
	// lock the m_ActiveDocLock while updating m_ActiveDoc and m_ActiveDocRights
	//
	{
		nudf::util::CRwExclusiveLocker lock(&m_ActiveDocLock);

		if (DocFullName)
		{
			m_ActiveDoc = DocFullName;

			SysFreeString(DocFullName);
			DocFullName = NULL;
		}
		else
		{
			m_ActiveDoc.clear();
		}

		if (m_ActiveDocRights != RightsMask)
		{
			UpdateRibbonUI = TRUE;
			m_ActiveDocRights = RightsMask;
		}
	}

	if (UpdateRibbonUI || m_InvalidCount == 0)
	{
		InvalidMsoControls();
	}

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowDeactivate(
/*[in]*/ struct Word::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::DocumentBeforeClose(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT __stdcall WordEventListener::WindowActivate(
/*[in]*/ struct Word2010::_Document * Doc,
/*[in]*/ struct Word2010::Window * Wn)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	BOOL UpdateRibbonUI = FALSE;
	
	do
	{
		hr = Doc->get_FullName(&DocFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new workbook
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
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvlauationId);

		if (FAILED(hr))
		{
			break;
		}

	} while (FALSE);

	//
	// lock the m_ActiveDocLock while updating m_ActiveDoc and m_ActiveDocRights
	//
	{
		nudf::util::CRwExclusiveLocker lock(&m_ActiveDocLock);

		if (DocFullName)
		{
			m_ActiveDoc = DocFullName;

			SysFreeString(DocFullName);
			DocFullName = NULL;
		}
		else
		{
			m_ActiveDoc.clear();
		}

		if (m_ActiveDocRights != RightsMask)
		{
			UpdateRibbonUI = TRUE;
			m_ActiveDocRights = RightsMask;
		}
	}

	if (UpdateRibbonUI || m_InvalidCount == 0)
	{
		InvalidMsoControls();
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

HRESULT __stdcall WordEventListener::ProtectedViewWindowActivate(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	HWND hWnd = 0;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	Word2010::_Document *Doc = NULL;
	
	BOOL UpdateRibbonUI = FALSE;

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
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

	} while (FALSE);

	if (Doc)
	{
		Doc->Release();
		Doc = NULL;
	}

	//
	// lock the m_ActiveDocLock while updating m_ActiveDoc and m_ActiveDocRights
	//
	{
		nudf::util::CRwExclusiveLocker lock(&m_ActiveDocLock);

		if (DocFullName)
		{
			m_ActiveDoc = DocFullName;

			SysFreeString(DocFullName);
			DocFullName = NULL;
		}
		else
		{
			m_ActiveDoc.clear();
		}

		if (m_ActiveDocRights != RightsMask)
		{
			UpdateRibbonUI = TRUE;
			m_ActiveDocRights = RightsMask;
		}
	}

	if (UpdateRibbonUI || m_InvalidCount == 0)
	{
		InvalidMsoControls();
	}

	return hr;
}

HRESULT __stdcall WordEventListener::ProtectedViewWindowDeactivate(
/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP WordEventListener::GetActiveDoc(
	/*[in,out]*/ std::wstring &ActiveDoc)
{
	nudf::util::CRwSharedLocker lock(&m_ActiveDocLock);

	ActiveDoc = m_ActiveDoc;

	return S_OK;
}

STDMETHODIMP WordEventListener::GetActiveRights(
	/*[in,out]*/ ULONGLONG &ActiveRights)
{
	nudf::util::CRwSharedLocker lock(&m_ActiveDocLock);

	ActiveRights = m_ActiveDocRights;

	return S_OK;
}

void WordEventListener::InvalidMsoControls(void)
{
	HRESULT hr = S_OK;

	do
	{
		if (!m_pRibbonUI)
		{
			break;
		}

		if (g_bIsOffice2010)
		{
			Office2010::IRibbonUI *pRibbonUI = NULL;

			hr = m_pRibbonUI->QueryInterface(__uuidof(Office::IRibbonUI), (void**)&pRibbonUI);

			if (!SUCCEEDED(hr))
			{
				break;
			}

			for (ULONG i = 0; i < (ULONG)(g_word_14_ribbon_info.size()); i++)
			{
				pRibbonUI->InvalidateControlMso((BSTR)g_word_14_ribbon_info[i].RibbonId);
			}

			if (pRibbonUI)
			{
				pRibbonUI->Release();
				pRibbonUI = NULL;
			}
		}
		else
		{
			Office::IRibbonUI *pRibbonUI = NULL;

			hr = m_pRibbonUI->QueryInterface(__uuidof(Office::IRibbonUI), (void**)&pRibbonUI);

			if (!SUCCEEDED(hr))
			{
				break;
			}

			for (ULONG i = 0; i < (ULONG)(g_word_15_ribbon_info.size()); i++)
			{
				pRibbonUI->InvalidateControlMso((BSTR)g_word_15_ribbon_info[i].RibbonId);
			}

			if (pRibbonUI)
			{
				pRibbonUI->Release();
				pRibbonUI = NULL;
			}
		}

		m_InvalidCount++;

	} while (FALSE);

	return;
}

HRESULT WordEventListener::RefreshActiveRights(void)
{
	HRESULT hr = S_OK;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do
	{
		if (m_ActiveDoc.empty())
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
		hr = nudf::util::nxl::NxrmCheckRights(m_ActiveDoc.c_str(), &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

		//
		// lock the m_ActiveDocLock while updating m_ActiveDoc and m_ActiveDocRights
		//
		{
			nudf::util::CRwExclusiveLocker lock(&m_ActiveDocLock);

			if (m_ActiveDocRights != RightsMask)
			{
				UpdateRibbonUI = TRUE;
				m_ActiveDocRights = RightsMask;
			}
		}

		if (UpdateRibbonUI)
		{
			InvalidMsoControls();
		}

	} while (FALSE);

	return hr;
}
