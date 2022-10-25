#pragma once

#include "mso.h"
#include "mso2010.h"
#include "outlook.h"
#include "outlook2010.h"

#ifndef CACHE_LINE
#define CACHE_LINE	64
#endif

#ifndef CACHE_ALIGN
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))
#endif

class nxrmCoreOutlookExt : public Outlook::ApplicationEvents_11
{
public:
	nxrmCoreOutlookExt();
	~nxrmCoreOutlookExt();

	HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [annotation][iid_is][out] */ 
		_COM_Outptr_  void **ppvObject);

	ULONG STDMETHODCALLTYPE AddRef(void);

	ULONG STDMETHODCALLTYPE Release(void);

	HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
		/* [out] */ __RPC__out UINT *pctinfo);

	HRESULT STDMETHODCALLTYPE GetTypeInfo( 
		/* [in] */ UINT iTInfo,
		/* [in] */ LCID lcid,
		/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);

	HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
		/* [in] */ __RPC__in REFIID riid,
		/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
		/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
		/* [in] */ LCID lcid,
		/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);

	HRESULT STDMETHODCALLTYPE Invoke( 
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
		_Out_opt_  UINT *puArgErr);

	HRESULT STDMETHODCALLTYPE Quit();

	HRESULT STDMETHODCALLTYPE Quit2010();

	HRESULT STDMETHODCALLTYPE ItemSend(IDispatch *Item, VARIANT_BOOL * Cancel);

	HRESULT STDMETHODCALLTYPE ItemSend2010(IDispatch *Item, VARIANT_BOOL * Cancel);

	HRESULT STDMETHODCALLTYPE ItemLoad(IDispatch *Item);

	HRESULT STDMETHODCALLTYPE ItemLoad2010(IDispatch *Item);

private:

	ULONG				m_uRefCount;
	
	typedef enum _OutlookAppEventId {

		ItemSend_Id = 0x0000f002,

		Quit_Id = 0x0000f007,

		ItemLoad_Id = 0x0000fba7

	}OutlookAppEventId;

};


class nxrmCoreItemLoadEventSink : public Outlook::ItemEvents_10
{
public:
	nxrmCoreItemLoadEventSink();
	~nxrmCoreItemLoadEventSink();

	HRESULT STDMETHODCALLTYPE QueryInterface( 
		/* [in] */ REFIID riid,
		/* [annotation][iid_is][out] */ 
		_COM_Outptr_  void **ppvObject);

	ULONG STDMETHODCALLTYPE AddRef(void);

	ULONG STDMETHODCALLTYPE Release(void);

	HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
		/* [out] */ __RPC__out UINT *pctinfo);

	HRESULT STDMETHODCALLTYPE GetTypeInfo( 
		/* [in] */ UINT iTInfo,
		/* [in] */ LCID lcid,
		/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);

	HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
		/* [in] */ __RPC__in REFIID riid,
		/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
		/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
		/* [in] */ LCID lcid,
		/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);

	HRESULT STDMETHODCALLTYPE Invoke( 
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
		_Out_opt_  UINT *puArgErr);

	HRESULT STDMETHODCALLTYPE BeforeAttachmentAdd(
		_In_ Outlook::Attachment* Attachment, 
		_Out_opt_	VARIANT_BOOL* Cancel);

	HRESULT STDMETHODCALLTYPE BeforeAttachmentAdd2010(
		_In_ Outlook2010::Attachment* Attachment, 
		_Out_opt_	VARIANT_BOOL* Cancel);

private:
	
	ULONG				m_uRefCount;

	typedef enum _OutlookItemEventId {

		BeforeAttachmentAdd_Id = 0x0000fbb0,

	}OutlookItemEventId;
};