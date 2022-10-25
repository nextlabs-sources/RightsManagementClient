#include "stdafx.h"
#include "mso.h"
#include "powerpoint.h"
#include "powerpointevents.h"
#include "nxrmcoreext2.h"

extern CACHE_ALIGN	BOOL	g_bIsOffice2010;

extern BOOL init_rm_section_safe(void);

extern "C" const std::vector<RIBBON_ID_INFO> g_powerpoint_14_ribbon_info;
extern "C" const std::vector<RIBBON_ID_INFO> g_powerpoint_15_ribbon_info;

PowerPointEventListener::PowerPointEventListener()
{
	m_uRefCount = 0;
	m_ActiveDocRights = BUILTIN_RIGHT_ALL;
	m_pRibbonUI = NULL;
	m_InvalidCount = 0;
}

PowerPointEventListener::PowerPointEventListener(IDispatch *pRibbonUI, BSTR ActiveDoc, ULONGLONG &ActiveRights)
{
	m_uRefCount = 0;
	m_pRibbonUI = pRibbonUI;
	m_ActiveDocRights = ActiveRights;
	m_ActiveDoc = ActiveDoc ? ActiveDoc : L"";
	m_InvalidCount = 0;
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

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
		{
			hr = WindowDeactivate((PowerPoint2010::_Presentation*)Pres, (PowerPoint2010::DocumentWindow *)Wn);
		}
		else
		{
			hr = WindowDeactivate((PowerPoint::_Presentation*)Pres, (PowerPoint::DocumentWindow *)Wn);
		}

		break;

	case ProtectedViewWindowActivate_Id:

		PvWindow = (void *)pDispParams->rgvarg[0].pdispVal;

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
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

		if (g_bIsOffice2010)
		{
			hr = PresentationClose((PowerPoint2010::_Presentation *)Pres);
		}
		else
		{
			hr = PresentationClose((PowerPoint::_Presentation *)Pres);
		}

		break;

	default:
		break;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowSelectionChange(
/*[in]*/ struct PowerPoint::Selection * Sel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowBeforeRightClick(
/*[in]*/ struct PowerPoint::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowBeforeDoubleClick(
/*[in]*/ struct PowerPoint::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationClose(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationSave(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationOpen(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::NewPresentation(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationNewSlide(
/*[in]*/ struct PowerPoint::_Slide * Sld)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowActivate (
/*[in]*/ struct PowerPoint::_Presentation * Pres,
/*[in]*/ struct PowerPoint::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	BSTR PptFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do
	{
		hr = Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new workbook
		//
		if (!PptFullName)
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
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvlauationId);

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

		if (PptFullName)
		{
			m_ActiveDoc = PptFullName;

			SysFreeString(PptFullName);
			PptFullName = NULL;
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

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowDeactivate (
/*[in]*/ struct PowerPoint::_Presentation * Pres,
/*[in]*/ struct PowerPoint::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowBegin(
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextBuild(
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextSlide(
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}
HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowEnd(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}
HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationPrint(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideSelectionChanged(
/*[in]*/ struct PowerPoint::SlideRange * SldRange)
{
	HRESULT hr = S_OK;

	return hr;
}
HRESULT STDMETHODCALLTYPE PowerPointEventListener::ColorSchemeChanged(
/*[in]*/ struct PowerPoint::SlideRange * SldRange)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationBeforeSave(
/*[in]*/ struct PowerPoint::_Presentation * Pres,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowNextClick(
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn,
/*[in]*/ struct PowerPoint::Effect * nEffect)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterNewPresentation(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterPresentationOpen(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationSync(
/*[in]*/ struct PowerPoint::_Presentation * Pres,
/*[in]*/ enum Office::MsoSyncEventType SyncEventType)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowOnNext(
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::SlideShowOnPrevious(
/*[in]*/ struct PowerPoint::SlideShowWindow * Wn)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationBeforeClose(
/*[in]*/ struct PowerPoint::_Presentation * Pres,
	/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowOpen(
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowBeforeEdit(
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowBeforeClose(
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow,
/*[in]*/ enum PowerPoint::PpProtectedViewCloseReason ProtectedViewCloseReason,
/*[in,out]*/ VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowActivate (
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	BSTR PptFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	PowerPoint::_Presentation *Pres = NULL;

	BOOL UpdateRibbonUI = FALSE;

	do 
	{
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

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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

		if (PptFullName)
		{
			m_ActiveDoc = PptFullName;

			SysFreeString(PptFullName);
			PptFullName = NULL;
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

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowDeactivate (
/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationCloseFinal(
/*[in]*/ struct PowerPoint::_Presentation * Pres)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterDragDropOnSlide(
/*[in]*/ struct PowerPoint::_Slide * Sld,
	/*[in]*/ float X,
	/*[in]*/ float Y)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::AfterShapeSizeChange(
/*[in]*/ struct PowerPoint::Shape * shp)
{
	HRESULT hr = S_OK;

	return hr;
}

//////////////////////////////////////////////////////////////////////////
//
// Office 2010
//
//////////////////////////////////////////////////////////////////////////


HRESULT STDMETHODCALLTYPE PowerPointEventListener::PresentationClose (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowActivate (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
/*[in]*/ struct PowerPoint2010::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	BSTR PptFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvlauationId = 0;

	BOOL UpdateRibbonUI = FALSE;

	do
	{
		hr = Pres->get_FullName(&PptFullName);

		if (FAILED(hr))
		{
			break;
		}

		//
		// skip new workbook
		//
		if (!PptFullName)
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
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvlauationId);

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

		if (PptFullName)
		{
			m_ActiveDoc = PptFullName;

			SysFreeString(PptFullName);
			PptFullName = NULL;
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

HRESULT STDMETHODCALLTYPE PowerPointEventListener::WindowDeactivate (
/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
/*[in]*/ struct PowerPoint2010::DocumentWindow * Wn )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowActivate (
/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	BSTR PptFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	PowerPoint2010::_Presentation *Pres = NULL;

	BOOL UpdateRibbonUI = FALSE;

	do
	{
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

		if (!init_rm_section_safe())
		{
			break;
		}

		//
		// take care of rights
		//
		hr = nudf::util::nxl::NxrmCheckRights(PptFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

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

		if (PptFullName)
		{
			m_ActiveDoc = PptFullName;

			SysFreeString(PptFullName);
			PptFullName = NULL;
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

HRESULT STDMETHODCALLTYPE PowerPointEventListener::ProtectedViewWindowDeactivate (
/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow )
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP PowerPointEventListener::GetActiveDoc(
	/*[in,out]*/ std::wstring &ActiveDoc)
{
	nudf::util::CRwSharedLocker lock(&m_ActiveDocLock);

	ActiveDoc = m_ActiveDoc;

	return S_OK;
}

STDMETHODIMP PowerPointEventListener::GetActiveRights(
	/*[in,out]*/ ULONGLONG &ActiveRights)
{
	nudf::util::CRwSharedLocker lock(&m_ActiveDocLock);

	ActiveRights = m_ActiveDocRights;

	return S_OK;
}

void PowerPointEventListener::InvalidMsoControls(void)
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

			for (ULONG i = 0; i < (ULONG)(g_powerpoint_14_ribbon_info.size()); i++)
			{
				pRibbonUI->InvalidateControlMso((BSTR)g_powerpoint_14_ribbon_info[i].RibbonId);
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

			for (ULONG i = 0; i < (ULONG)(g_powerpoint_15_ribbon_info.size()); i++)
			{
				pRibbonUI->InvalidateControlMso((BSTR)g_powerpoint_15_ribbon_info[i].RibbonId);
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

HRESULT PowerPointEventListener::RefreshActiveRights(void)
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
