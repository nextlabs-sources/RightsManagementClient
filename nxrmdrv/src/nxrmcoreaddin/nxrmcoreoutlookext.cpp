#include "stdafx.h"
#include "nxrmdrv.h"
#include "nxrmdrvman.h"
#include "nxrmcorehlp.h"
#include "nxrmcoreoutlookext.h"

#include <atlbase.h>
#include <atlcomcli.h>

#include <nudf\exception.hpp>
#include <nudf\regex.hpp>

extern CACHE_ALIGN	HMODULE g_hModule;
extern CACHE_ALIGN	BOOL	g_bIsOffice2010;

nxrmCoreOutlookExt::nxrmCoreOutlookExt()
{
	m_uRefCount	= 1;
}

nxrmCoreOutlookExt::~nxrmCoreOutlookExt()
{
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */ 
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppvObject = NULL;

	do 
	{
		if(IID_IUnknown == riid || IID_IDispatch == riid || riid == __uuidof(Outlook::ApplicationEvents_11))
		{
			punk = this;
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

ULONG STDMETHODCALLTYPE nxrmCoreOutlookExt::AddRef(void)
{
	m_uRefCount++;

	return m_uRefCount;
}	

ULONG STDMETHODCALLTYPE nxrmCoreOutlookExt::Release(void)
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

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::GetTypeInfoCount( 
	/* [out] */ __RPC__out UINT *pctinfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::GetTypeInfo( 
	/* [in] */ UINT iTInfo,
	/* [in] */ LCID lcid,
	/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::GetIDsOfNames( 
	/* [in] */ __RPC__in REFIID riid,
	/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
	/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
	/* [in] */ LCID lcid,
	/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::Invoke( 
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

	IDispatch *pItem = NULL;
	VARIANT_BOOL *Cancel = NULL;

	do 
	{
		switch (dispIdMember)
		{
		case Quit_Id:
			
			if (g_bIsOffice2010)
			{
				hr = Quit2010();
			}
			else
			{
				hr = Quit();
			}

			break;

		case ItemSend_Id:

			if (pDispParams->rgdispidNamedArgs)
			{
				for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
				{
					if (pDispParams->rgdispidNamedArgs[i] == 0)
					{
						pItem = pDispParams->rgvarg[i].pdispVal;
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
				pItem = pDispParams->rgvarg[1].pdispVal;
				Cancel = pDispParams->rgvarg[0].pboolVal;
			}
			
			if (g_bIsOffice2010)
			{
				hr = ItemSend2010(pItem, Cancel);
			}
			else
			{
				hr = ItemSend(pItem, Cancel);
			}

			break;

		case ItemLoad_Id:

			pItem = pDispParams->rgvarg[0].pdispVal;

			if (g_bIsOffice2010)
			{
				hr = ItemLoad2010(pItem);
			}
			else
			{
				hr = ItemLoad(pItem);
			}

			break;

		default:
			break;

		}

	} while (FALSE);

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::Quit()
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::Quit2010()
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::ItemSend(IDispatch *Item, VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	PVOID Section = NULL;

	do 
	{
		//
		// only perform inspection when service is running
		//
		Section = init_transporter_client();

		if (!Section)
		{
			break;
		}

		if (!is_transporter_enabled(Section))
		{
			break;
		}


	} while (FALSE);

	if (Section)
	{
		close_transporter_client(Section);
		Section = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::ItemSend2010(IDispatch *Item, VARIANT_BOOL * Cancel)
{
	HRESULT hr = S_OK;

	PVOID Section = NULL;

	do 
	{
		//
		// only perform inspection when service is running
		//
		Section = init_transporter_client();

		if (!Section)
		{
			break;
		}

		if (!is_transporter_enabled(Section))
		{
			break;
		}


	} while (FALSE);

	if (Section)
	{
		close_transporter_client(Section);
		Section = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::ItemLoad(IDispatch *Item)
{
	HRESULT hr = S_OK;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;

	nxrmCoreItemLoadEventSink *pItemEventSink = NULL;

	do 
	{
		pItemEventSink = new nxrmCoreItemLoadEventSink;

		if (!pItemEventSink)
		{
			break;
		}

		hr = Item->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Outlook::ItemEvents_10), &pConnectionPoint);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise(pItemEventSink, &dwAdviseCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pItemEventSink = NULL;

	} while (FALSE);

	if (pItemEventSink)
	{
		delete pItemEventSink;
		pItemEventSink = NULL;
	}

	if (pConnectionPoint)
	{
		pConnectionPoint->Release();
		pConnectionPoint = NULL;
	}

	if (pConnectionPointContainer)
	{
		pConnectionPointContainer->Release();
		pConnectionPointContainer = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreOutlookExt::ItemLoad2010(IDispatch *Item)
{
	HRESULT hr = S_OK;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;

	nxrmCoreItemLoadEventSink *pItemEventSink = NULL;

	do 
	{
		pItemEventSink = new nxrmCoreItemLoadEventSink;

		if (!pItemEventSink)
		{
			break;
		}

		hr = Item->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Outlook2010::ItemEvents_10), &pConnectionPoint);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise(pItemEventSink, &dwAdviseCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pItemEventSink = NULL;

	} while (FALSE);

	if (pItemEventSink)
	{
		delete pItemEventSink;
		pItemEventSink = NULL;
	}

	if (pConnectionPoint)
	{
		pConnectionPoint->Release();
		pConnectionPoint = NULL;
	}

	if (pConnectionPointContainer)
	{
		pConnectionPointContainer->Release();
		pConnectionPointContainer = NULL;
	}

	return hr;
}


nxrmCoreItemLoadEventSink::nxrmCoreItemLoadEventSink()
{
	m_uRefCount	= 1;
}

nxrmCoreItemLoadEventSink::~nxrmCoreItemLoadEventSink()
{
}

HRESULT STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::QueryInterface( 
	/* [in] */ REFIID riid,
	/* [annotation][iid_is][out] */ 
	_COM_Outptr_  void **ppvObject)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppvObject = NULL;

	do 
	{
		if(IID_IUnknown == riid || IID_IDispatch == riid)
		{
			punk = this;
		}
		else if (g_bIsOffice2010 && riid == __uuidof(Outlook2010::ItemEvents_10))
		{
			punk = this;
		}
		else if (!g_bIsOffice2010 && riid == __uuidof(Outlook::ItemEvents_10))
		{
			punk = this;
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

ULONG STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::AddRef(void)
{
	m_uRefCount++;

	return m_uRefCount;
}	

ULONG STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::Release(void)
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

HRESULT STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::GetTypeInfoCount( 
	/* [out] */ __RPC__out UINT *pctinfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::GetTypeInfo( 
	/* [in] */ UINT iTInfo,
	/* [in] */ LCID lcid,
	/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::GetIDsOfNames( 
	/* [in] */ __RPC__in REFIID riid,
	/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
	/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
	/* [in] */ LCID lcid,
	/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::Invoke( 
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

	void *pAttachment = NULL;
	VARIANT_BOOL *Cancel = NULL;

	do 
	{
		switch (dispIdMember)
		{

		case BeforeAttachmentAdd_Id:

			if (pDispParams->rgdispidNamedArgs)
			{
				for (UINT i = 0 ; i < min(pDispParams->cArgs, 2); i++)
				{
					if (pDispParams->rgdispidNamedArgs[i] == 0)
					{
						pAttachment = (void*)pDispParams->rgvarg[i].pdispVal;
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
				pAttachment = pDispParams->rgvarg[1].pdispVal;
				Cancel = pDispParams->rgvarg[0].pboolVal;
			}

			if (g_bIsOffice2010)
			{
				hr = BeforeAttachmentAdd2010((Outlook2010::Attachment*)pAttachment, Cancel);
			}
			else
			{
				hr = BeforeAttachmentAdd((Outlook::Attachment*)pAttachment, Cancel);
			}

			break;

		default:
			break;

		}

	} while (FALSE);

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::BeforeAttachmentAdd(
	_In_ Outlook::Attachment* Attachment, 
	_Out_opt_	VARIANT_BOOL* Cancel)
{
	HRESULT hr = S_OK;


	//CComPtr<IDispatch> spItemDispatch;
	//CComPtr<Outlook::_MailItem> spItem;
	//CComPtr<Outlook::Attachments> spAttachments;

	//CComBSTR bstrTempFilePath;
	//CComBSTR bstrFileName;

	//Outlook::OlAttachmentType nAttachType;
	//long lPosition = 0;
	//long lSize = 0;


	//hr = Attachment->get_Type(&nAttachType);
	//if (!SUCCEEDED(hr)) {
	//    return S_OK;
	//}

	//if (nAttachType != Outlook::olByValue) {
	//    return S_OK;
	//}
	//
	//hr = Attachment->get_Position(&lPosition);
	//if (!SUCCEEDED(hr)) {
	//    return S_OK;
	//}

	//hr = Attachment->get_Size(&lSize);
	//if (!SUCCEEDED(hr)) {
	//    return S_OK;
	//}

	//if (lSize < 12 * 1024) {
	//    return S_OK;
	//}

	//hr = Attachment->GetTemporaryFilePath(&bstrTempFilePath);
	//if (!SUCCEEDED(hr)) {
	//    return S_OK;
	//}

	//nudf::util::nxl::CFile nf(bstrTempFilePath.m_str);
	//if (nf.Validate() != nudf::util::nxl::NXL_OK) {
	//    return S_OK;
	//}

	//hr = Attachment->get_FileName(&bstrFileName);
	//if (!SUCCEEDED(hr) || NULL==bstrFileName.m_str) {
	//    return S_OK;
	//}
	//
	//std::wstring wsFileName(bstrFileName.m_str);
	//bstrFileName.Empty();
	//if(wsFileName.empty()) {
	//    return S_OK;
	//}

	//if(nudf::util::regex::Match<wchar_t>(wsFileName, L".*\\.nxl$", true)) {
	//    return S_OK;
	//}

	//hr = Attachment->get_Parent(&spItemDispatch);
	//if (!SUCCEEDED(hr)) {
	//    return S_OK;
	//}

	//hr = spItemDispatch->QueryInterface(__uuidof(Outlook::_MailItem), (void**)&spItem);
	//if (!SUCCEEDED(hr)) {
	//    return S_OK;
	//}

	//hr = spItem->get_Attachments(&spAttachments);
	//if (!SUCCEEDED(hr)) {
	//    return S_OK;
	//}
	//
	//WCHAR wzTempFile[MAX_PATH];
	//GetTempPathW(MAX_PATH, wzTempFile);
	//if(wzTempFile[wcslen(wzTempFile) - 1] != L'\\') {
	//    wcsncat_s(wzTempFile, MAX_PATH, L"\\", _TRUNCATE);
	//}
	//wsFileName += L".nxl";
	//wcsncat_s(wzTempFile, MAX_PATH, wsFileName.c_str(), _TRUNCATE);

	//// Temp file with NXL extension
	//hr = Attachment->SaveAsFile(wzTempFile);


	//CComVariant varSource(wzTempFile);
	//CComVariant varType((long)nAttachType);
	//CComVariant varPosition((long)lPosition+1);
	//CComVariant varDisplayName(wsFileName.c_str());    

	//CComPtr<Outlook::Attachment> spNewAttachment;
	//hr = spAttachments->Add(varSource, varType, varPosition, varDisplayName, &spNewAttachment);
	//if(SUCCEEDED(hr)) {
	//    hr = Attachment->Delete();
	//    spAttachments->Remove(lPosition);
	//    // Cancel current attach
	//    if(NULL != Cancel) {
	//        *Cancel = VARIANT_TRUE;
	//    }
	//    spItem->Save();
	//}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE nxrmCoreItemLoadEventSink::BeforeAttachmentAdd2010(
	_In_ Outlook2010::Attachment* Attachment, 
	_Out_opt_	VARIANT_BOOL* Cancel)
{
	HRESULT hr = S_OK;

	//CComPtr<IDispatch> spItemDispatch = NULL;
	//CComPtr<Outlook2010::_MailItem> spItem = NULL;
	//CComPtr<Outlook2010::Attachments> spAttachments = NULL;

	//CComBSTR bstrTempFilePath;
	//CComBSTR bstrFileName;

	//Outlook2010::OlAttachmentType nAttachType;
	//long lPosition = 0;
	//long lSize = 0;

	//std::wstring wsFileName;

	//do 
	//{
	//	hr = Attachment->get_Type(&nAttachType);

	//	if (!SUCCEEDED(hr)) 
	//	{
	//		break;
	//	}

	//	if (nAttachType != Outlook::olByValue) 
	//	{
	//		break;
	//	}
	//
	//	hr = Attachment->get_Position(&lPosition);

	//	if (!SUCCEEDED(hr)) 
	//	{
	//		break;
	//	}

	//	hr = Attachment->get_Size(&lSize);

	//	if (!SUCCEEDED(hr)) 
	//	{
	//		break;
	//	}

	//	if (lSize < 12 * 1024) 
	//	{
	//		break;
	//	}

	//	hr = Attachment->GetTemporaryFilePath(&bstrTempFilePath);

	//	if (!SUCCEEDED(hr)) 
	//	{
	//		break;
	//	}

	//	nudf::util::nxl::CFile nf(bstrTempFilePath.m_str);

	//	if (nf.Validate() != nudf::util::nxl::NXL_OK) 
	//	{
	//		break;
	//	}

	//	hr = Attachment->get_FileName(&bstrFileName);

	//	if (!SUCCEEDED(hr) || NULL==bstrFileName.m_str) 
	//	{
	//		break;
	//	}

	//	wsFileName = bstrFileName.m_str;
	//	
	//	if(wsFileName.empty()) 
	//	{
	//		break;
	//	}

	//	hr = Attachment->get_Parent(&spItemDispatch);
	//	
	//	if (!SUCCEEDED(hr)) 
	//	{
	//		break;
	//	}

	//	hr = spItemDispatch->QueryInterface(__uuidof(Outlook::_MailItem), (void**)&spItem);

	//	if (!SUCCEEDED(hr)) 
	//	{
	//		break;
	//	}

	//	hr = spItem->get_Attachments(&spAttachments);

	//	if (!SUCCEEDED(hr)) 
	//	{
	//		break;
	//	}
	//	
	//	WCHAR wzTempFile[MAX_PATH];

	//	GetTempPathW(MAX_PATH, wzTempFile);

	//	if(wzTempFile[wcslen(wzTempFile) - 1] != L'\\') 
	//	{
	//		wcsncat_s(wzTempFile, MAX_PATH, L"\\", _TRUNCATE);
	//	}

	//	wsFileName += L".aaa.nxl";
	//	wcsncat_s(wzTempFile, MAX_PATH, wsFileName.c_str(), _TRUNCATE);

	//	// Temp file with NXL extension
	//	hr = Attachment->SaveAsFile(wzTempFile);


	//	CComVariant varSource(wzTempFile);
	//	CComVariant varType((long)nAttachType);
	//	CComVariant varPosition((long)lPosition+1);
	//	CComVariant varDisplayName(wsFileName.c_str());    

	//	CComPtr<Outlook2010::Attachment> spNewAttachment;

	//	hr = spAttachments->Add(varSource, varType, varPosition, varDisplayName, &spNewAttachment);

	//	if(SUCCEEDED(hr)) 
	//	{
	//		hr = Attachment->Delete();

	//		spAttachments->Remove(lPosition);
	//		// Cancel current attach
	//		
	//		if(NULL != Cancel) 
	//		{
	//			*Cancel = VARIANT_TRUE;
	//		}

	//		spItem->Save();
	//	}

	//} while (FALSE);

	return hr;
}
