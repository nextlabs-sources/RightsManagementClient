#pragma once

#include "mso.h"
#include "word.h"
#include "mso2010.h"
#include "word2010.h"
#include <nudf\rwlock.hpp>

class WordEventListener : public Word::IApplicationEvents4
{
public:
	WordEventListener();

	WordEventListener(IDispatch *pRibbonUI, BSTR ActiveDoc, ULONGLONG &ActiveRights);

	~WordEventListener();

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

	HRESULT __stdcall Startup();

	HRESULT __stdcall Quit();

	HRESULT __stdcall DocumentChange();

	HRESULT __stdcall DocumentOpen(
	/*[in]*/ struct Word::_Document * Doc);

	HRESULT __stdcall DocumentBeforeClose(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall DocumentBeforePrint(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall DocumentBeforeSave(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ VARIANT_BOOL * SaveAsUI,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall NewDocument(
	/*[in]*/ struct Word::_Document * Doc);

	HRESULT __stdcall WindowActivate(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ struct Word::Window * Wn);

	HRESULT __stdcall WindowDeactivate(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ struct Word::Window * Wn);

	HRESULT __stdcall WindowSelectionChange(
	/*[in]*/ struct Word::Selection * Sel);

	HRESULT __stdcall WindowBeforeRightClick(
	/*[in]*/ struct Word::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall WindowBeforeDoubleClick(
	/*[in]*/ struct Word::Selection * Sel,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall EPostagePropertyDialog(
	/*[in]*/ struct Word::_Document * Doc);

	HRESULT __stdcall EPostageInsert(
	/*[in]*/ struct Word::_Document * Doc);

	HRESULT __stdcall MailMergeAfterMerge(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ struct Word::_Document * DocResult);

	HRESULT __stdcall MailMergeAfterRecordMerge(
	/*[in]*/ struct Word::_Document * Doc);

	HRESULT __stdcall MailMergeBeforeMerge(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ long StartRecord,
	/*[in]*/ long EndRecord,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall MailMergeBeforeRecordMerge(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall MailMergeDataSourceLoad(
	/*[in]*/ struct Word::_Document * Doc);

	HRESULT __stdcall MailMergeDataSourceValidate(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ VARIANT_BOOL * Handled);

	HRESULT __stdcall MailMergeWizardSendToCustom(
	/*[in]*/ struct Word::_Document * Doc);

	HRESULT __stdcall MailMergeWizardStateChange(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ int * FromState,
	/*[in]*/ int * ToState,
	/*[in]*/ VARIANT_BOOL * Handled);

	HRESULT __stdcall WindowSize(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ struct Word::Window * Wn);

	HRESULT __stdcall XMLSelectionChange(
	/*[in]*/ struct Word::Selection * Sel,
	/*[in]*/ struct Word::XMLNode * OldXMLNode,
	/*[in]*/ struct Word::XMLNode * NewXMLNode,
	/*[in]*/ long * Reason);

	HRESULT __stdcall XMLValidationError(
	/*[in]*/ struct Word::XMLNode * XMLNode);

	HRESULT __stdcall DocumentSync(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in]*/ enum Office::MsoSyncEventType SyncEventType);

	HRESULT __stdcall EPostageInsertEx(
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
	/*[in,out]*/ VARIANT_BOOL * fCancel);

	HRESULT __stdcall MailMergeDataSourceValidate2(
	/*[in]*/ struct Word::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Handled);

	HRESULT __stdcall ProtectedViewWindowOpen(
	/*[in]*/ struct Word::ProtectedViewWindow * PvWindow);

	HRESULT __stdcall ProtectedViewWindowBeforeEdit(
	/*[in]*/ struct Word::ProtectedViewWindow * PvWindow,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall ProtectedViewWindowBeforeClose(
	/*[in]*/ struct Word::ProtectedViewWindow * PvWindow,
	/*[in]*/ int CloseReason,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall ProtectedViewWindowSize(
	/*[in]*/ struct Word::ProtectedViewWindow * PvWindow);

	HRESULT __stdcall ProtectedViewWindowActivate(
	/*[in]*/ struct Word::ProtectedViewWindow * PvWindow);

	HRESULT __stdcall ProtectedViewWindowDeactivate(
	/*[in]*/ struct Word::ProtectedViewWindow * PvWindow);

	//////////////////////////////////////////////////////////////////////////
	//
	// Office 2010 version
	//
	//////////////////////////////////////////////////////////////////////////

	HRESULT __stdcall DocumentBeforeClose(
	/*[in]*/ struct Word2010::_Document * Doc,
	/*[in,out]*/ VARIANT_BOOL * Cancel);

	HRESULT __stdcall WindowActivate(
	/*[in]*/ struct Word2010::_Document * Doc,
	/*[in]*/ struct Word2010::Window * Wn);

	HRESULT __stdcall WindowDeactivate(
	/*[in]*/ struct Word2010::_Document * Doc,
	/*[in]*/ struct Word2010::Window * Wn);

	HRESULT __stdcall ProtectedViewWindowActivate(
	/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow);

	HRESULT __stdcall ProtectedViewWindowDeactivate(
	/*[in]*/ struct Word2010::ProtectedViewWindow * PvWindow);

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

	typedef enum _WordAppEventId {

		Startup_Id = 1,
		Quit_Id,
		DocumentChange_Id,
		DocumentOpen_Id,
		DocumentBeforeClose_Id = 6,
		DocumentBeforePrint_Id,
		DocumentBeforeSave_Id,
		NewDocument_Id,
		WindowActivate_Id,
		WindowDeactivate_Id,
		WindowSelectionChange_Id,
		WindowBeforeRightClick_Id,
		WindowBeforeDoubleClick_Id,
		EPostagePropertyDialog_Id,
		EPostageInsert_Id,
		MailMergeAfterMerge_Id,
		MailMergeAfterRecordMerge_Id,
		MailMergeBeforeMerge_Id,
		MailMergeBeforeRecordMerge_Id,
		MailMergeDataSourceLoad_Id,
		MailMergeDataSourceValidate_Id,
		MailMergeWizardSendToCustom_Id,
		MailMergeWizardStateChange_Id,
		WindowSize_Id,
		XMLSelectionChange_Id,
		XMLValidationError_Id,
		DocumentSync_Id,
		EPostageInsertEx_Id,
		MailMergeDataSourceValidate2_Id,
		ProtectedViewWindowOpen_Id,
		ProtectedViewWindowBeforeEdit_Id,
		ProtectedViewWindowBeforeClose_Id,
		ProtectedViewWindowSize_Id,
		ProtectedViewWindowActivate_Id,
		ProtectedViewWindowDeactivate_Id

	}WordAppEventId;

	void InvalidMsoControls(void);
};