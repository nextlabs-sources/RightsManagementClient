#pragma once

#include "mso.h"
#include "excel.h"
#include "mso2010.h"
#include "excel2010.h"
#include <nudf\rwlock.hpp>

class ExcelEventListener : public IDispatch
{
public:
	ExcelEventListener();

	ExcelEventListener(IDispatch *pRibbonUI, BSTR ActiveDoc, ULONGLONG &ActiveRights);
	
	~ExcelEventListener();

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

	STDMETHODIMP WorkbookBeforeClose (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in,out]*/ VARIANT_BOOL * Cancel );


	STDMETHODIMP WindowActivate (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in]*/ Excel::Window * Wn );

	STDMETHODIMP WindowDeactivate (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in]*/ Excel::Window * Wn );


	STDMETHODIMP ProtectedViewWindowActivate (
		/*[in]*/ Excel::ProtectedViewWindow * Pvw );

	STDMETHODIMP ProtectedViewWindowDeactivate (
		/*[in]*/ Excel::ProtectedViewWindow * Pvw );


	//////////////////////////////////////////////////////////////////////////
	//
	// Office 2010 version
	//
	//////////////////////////////////////////////////////////////////////////

	STDMETHODIMP WorkbookBeforeClose (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in,out]*/ VARIANT_BOOL * Cancel );

	STDMETHODIMP WindowActivate (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in]*/ Excel2010::Window * Wn );

	STDMETHODIMP WindowDeactivate (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in]*/ Excel2010::Window * Wn );

	STDMETHODIMP ProtectedViewWindowActivate (
		/*[in]*/ Excel2010::ProtectedViewWindow * Pvw );

	STDMETHODIMP ProtectedViewWindowDeactivate (
		/*[in]*/ Excel2010::ProtectedViewWindow * Pvw );

	STDMETHODIMP GetActiveDoc(
		/*[in,out]*/ std::wstring &ActiveDoc);

	STDMETHODIMP GetActiveRights(
		/*[in,out]*/ ULONGLONG &ActiveRights);

	HRESULT RefreshActiveRights(void);

private:
	ULONG				m_uRefCount;

	IDispatch			*m_pRibbonUI;

	nudf::util::CRwLock	m_ActiveDocLock;

	std::wstring		m_ActiveDoc;

	ULONGLONG			m_ActiveDocRights;

	ULONG				m_InvalidCount;

	typedef enum _ExcelAppEventId {

		WorkbookBeforeClose_Id = 0x00000622,

		WorkbookBeforeSave_Id = 0x00000623,

		WorkbookBeforePrint_Id = 0x00000624,
		
		WindowActivate_Id = 0x00000614,
		
		WindowDeactivate_Id = 0x00000615,
		
		ProtectedViewWindowActivate_Id = 0x00000b5d,
		
		ProtectedViewWindowDeactivate_Id = 0x00000b5e

	}ExcelAppEventId;

	void InvalidMsoControls(void);
};