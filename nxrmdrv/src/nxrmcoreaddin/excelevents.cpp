#include "stdafx.h"
#include "excelevents.h"
#include "nxrmcoreext2.h"

extern CACHE_ALIGN	BOOL	g_bIsOffice2010;

extern BOOL init_rm_section_safe(void);

extern "C" const std::vector<RIBBON_ID_INFO> g_excel_14_ribbon_info;
extern "C" const std::vector<RIBBON_ID_INFO> g_excel_15_ribbon_info;

ExcelEventListener::ExcelEventListener()
{
	m_uRefCount = 0;
	m_ActiveDocRights = BUILTIN_RIGHT_ALL;
	m_pRibbonUI = NULL;
	m_InvalidCount = 0;
};

ExcelEventListener::ExcelEventListener(IDispatch *pRibbonUI, BSTR ActiveDoc, ULONGLONG &ActiveRights)
{
	m_uRefCount = 0;
	m_pRibbonUI = pRibbonUI;
	m_ActiveDocRights = ActiveRights;
	m_ActiveDoc = ActiveDoc ? ActiveDoc : L"";
	m_InvalidCount = 0;
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

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
		{
			hr = WindowDeactivate((Excel2010::_Workbook*)Wb, (Excel2010::Window*)Wn);
		}
		else
		{
			hr = WindowDeactivate((Excel::_Workbook*)Wb, (Excel::Window*)Wn);
		}

		break;

	case ProtectedViewWindowActivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
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

STDMETHODIMP ExcelEventListener::WindowActivate (
	/*[in]*/ Excel::_Workbook * Wb,
	/*[in]*/ Excel::Window * Wn )
{
	HRESULT hr = S_OK;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do 
	{
		hr = Wb->get_FullName(0x0409/*US English*/, &WbFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new workbook
		//
		if (!WbFullName)
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
		hr = nudf::util::nxl::NxrmCheckRights(WbFullName, &RightsMask, &CustomRightsMask, &EvlauationId);

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

		if (WbFullName)
		{
			m_ActiveDoc = WbFullName;

			SysFreeString(WbFullName);
			WbFullName = NULL;
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

	Excel::_Workbook *Wb = NULL;
	Excel::IProtectedViewWindow *pProtectVw = NULL;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do 
	{
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
			break;
		}

	} while (FALSE);

	if (Wb)
	{
		Wb->Release();
		Wb = NULL;
	}

	//
	// lock the m_ActiveDocLock while updating m_ActiveDoc and m_ActiveDocRights
	//
	{
		nudf::util::CRwExclusiveLocker lock(&m_ActiveDocLock);

		if (WbFullName)
		{
			m_ActiveDoc = WbFullName;

			SysFreeString(WbFullName);
			WbFullName = NULL;
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

	return hr;
}

STDMETHODIMP ExcelEventListener::WindowActivate (
	/*[in]*/ Excel2010::_Workbook * Wb,
	/*[in]*/ Excel2010::Window * Wn )
{
	HRESULT hr = S_OK;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do 
	{
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

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(WbFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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

		if (WbFullName)
		{
			m_ActiveDoc = WbFullName;

			SysFreeString(WbFullName);
			WbFullName = NULL;
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

STDMETHODIMP ExcelEventListener::WindowDeactivate (
	/*[in]*/ Excel2010::_Workbook * Wb,
	/*[in]*/ Excel2010::Window * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP ExcelEventListener::ProtectedViewWindowActivate (
	/*[in]*/ Excel2010::ProtectedViewWindow * Pvw )
{
	HRESULT hr = S_OK;

	Excel2010::IProtectedViewWindow *pProtectVw = NULL;
	Excel2010::_Workbook *Wb = NULL;

	BSTR WbFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do 
	{
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

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(WbFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (FAILED(hr))
		{
			break;
		}

	} while (FALSE);

	if (Wb)
	{
		Wb->Release();
	}

	//
	// lock the m_ActiveDocLock while updating m_ActiveDoc and m_ActiveDocRights
	//
	{
		nudf::util::CRwExclusiveLocker lock(&m_ActiveDocLock);

		if (WbFullName)
		{
			m_ActiveDoc = WbFullName;

			SysFreeString(WbFullName);
			WbFullName = NULL;
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

	return hr;
}

STDMETHODIMP ExcelEventListener::GetActiveDoc(
	/*[in,out]*/ std::wstring &ActiveDoc)
{
	nudf::util::CRwSharedLocker lock(&m_ActiveDocLock);

	ActiveDoc = m_ActiveDoc;

	return S_OK;
}

STDMETHODIMP ExcelEventListener::GetActiveRights(
	/*[in,out]*/ ULONGLONG &ActiveRights)
{
	nudf::util::CRwSharedLocker lock(&m_ActiveDocLock);

	ActiveRights = m_ActiveDocRights;

	return S_OK;
}

void ExcelEventListener::InvalidMsoControls(void)
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
			
			for (ULONG i = 0; i < (ULONG)(g_excel_14_ribbon_info.size()); i++)
			{
				pRibbonUI->InvalidateControlMso((BSTR)g_excel_14_ribbon_info[i].RibbonId);
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

			for (ULONG i = 0; i < (ULONG)(g_excel_15_ribbon_info.size()); i++)
			{
				pRibbonUI->InvalidateControlMso((BSTR)g_excel_15_ribbon_info[i].RibbonId);
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

HRESULT ExcelEventListener::RefreshActiveRights(void)
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
