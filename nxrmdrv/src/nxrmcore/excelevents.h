#pragma once

#include "mso.h"
#include "excel.h"
#include "mso2010.h"
#include "excel2010.h"

class ExcelEventListener : public IDispatch
{
public:
	ExcelEventListener();

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

	//STDMETHODIMP NewWorkbook (
	//	/*[in]*/ Excel::_Workbook * Wb);

	//STDMETHODIMP SheetSelectionChange (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::Range * Target );
	//STDMETHODIMP SheetBeforeDoubleClick (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::Range * Target,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP SheetBeforeRightClick (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::Range * Target,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );
	//
	//STDMETHODIMP SheetActivate (
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP SheetDeactivate (
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP SheetCalculate (
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP SheetChange (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::Range * Target );

	//STDMETHODIMP WorkbookOpen (
	//	/*[in]*/ Excel::_Workbook * Wb );

	//STDMETHODIMP WorkbookActivate (
	//	/*[in]*/ Excel::_Workbook * Wb );

	//STDMETHODIMP WorkbookDeactivate (
	//	/*[in]*/ Excel::_Workbook * Wb );

	STDMETHODIMP WorkbookBeforeClose (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in,out]*/ VARIANT_BOOL * Cancel );

	STDMETHODIMP WorkbookBeforeSave (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in]*/ VARIANT_BOOL SaveAsUI,
		/*[in,out]*/ VARIANT_BOOL * Cancel );

	STDMETHODIMP WorkbookBeforePrint (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP WorkbookNewSheet (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP WorkbookAddinInstall (
	//	/*[in]*/ Excel::_Workbook * Wb );

	//STDMETHODIMP WorkbookAddinUninstall (
	//	/*[in]*/ Excel::_Workbook * Wb );

	//STDMETHODIMP WindowResize (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::Window * Wn );

	STDMETHODIMP WindowActivate (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in]*/ Excel::Window * Wn );

	STDMETHODIMP WindowDeactivate (
		/*[in]*/ Excel::_Workbook * Wb,
		/*[in]*/ Excel::Window * Wn );

	//STDMETHODIMP SheetFollowHyperlink (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::Hyperlink * Target );

	//STDMETHODIMP SheetPivotTableUpdate (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::PivotTable * Target );

	//STDMETHODIMP WorkbookPivotTableCloseConnection (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::PivotTable * Target );

	//STDMETHODIMP WorkbookPivotTableOpenConnection (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::PivotTable * Target );

	//STDMETHODIMP WorkbookSync (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ enum Office::MsoSyncEventType SyncEventType );

	//STDMETHODIMP WorkbookBeforeXmlImport (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::XmlMap * Map,
	//	/*[in]*/ BSTR Url,
	//	/*[in]*/ VARIANT_BOOL IsRefresh,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP WorkbookAfterXmlImport (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::XmlMap * Map,
	//	/*[in]*/ VARIANT_BOOL IsRefresh,
	//	/*[in]*/ enum Excel::XlXmlImportResult Result );

	//STDMETHODIMP WorkbookBeforeXmlExport (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::XmlMap * Map,
	//	/*[in]*/ BSTR Url,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );
	//STDMETHODIMP WorkbookAfterXmlExport (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::XmlMap * Map,
	//	/*[in]*/ BSTR Url,
	//	/*[in]*/ enum Excel::XlXmlExportResult Result );

	//STDMETHODIMP WorkbookRowsetComplete (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ BSTR Description,
	//	/*[in]*/ BSTR Sheet,
	//	/*[in]*/ VARIANT_BOOL Success );

	//STDMETHODIMP AfterCalculate ( );

	//STDMETHODIMP SheetPivotTableAfterValueChange (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::PivotTable * TargetPivotTable,
	//	/*[in]*/ Excel::Range * TargetRange );

	//STDMETHODIMP SheetPivotTableBeforeAllocateChanges (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::PivotTable * TargetPivotTable,
	//	/*[in]*/ long ValueChangeStart,
	//	/*[in]*/ long ValueChangeEnd,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP SheetPivotTableBeforeCommitChanges (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::PivotTable * TargetPivotTable,
	//	/*[in]*/ long ValueChangeStart,
	//	/*[in]*/ long ValueChangeEnd,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP SheetPivotTableBeforeDiscardChanges (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::PivotTable * TargetPivotTable,
	//	/*[in]*/ long ValueChangeStart,
	//	/*[in]*/ long ValueChangeEnd );

	//STDMETHODIMP ProtectedViewWindowOpen (
	//	/*[in]*/ Excel::ProtectedViewWindow * Pvw );

	//STDMETHODIMP ProtectedViewWindowBeforeEdit (
	//	/*[in]*/ Excel::ProtectedViewWindow * Pvw,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP ProtectedViewWindowBeforeClose (
	//	/*[in]*/ Excel::ProtectedViewWindow * Pvw,
	//	/*[in]*/ enum Excel::XlProtectedViewCloseReason Reason,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP ProtectedViewWindowResize (
	//	/*[in]*/ Excel::ProtectedViewWindow * Pvw );

	STDMETHODIMP ProtectedViewWindowActivate (
		/*[in]*/ Excel::ProtectedViewWindow * Pvw );

	STDMETHODIMP ProtectedViewWindowDeactivate (
		/*[in]*/ Excel::ProtectedViewWindow * Pvw );

	//STDMETHODIMP WorkbookAfterSave (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ VARIANT_BOOL Success);

	//STDMETHODIMP WorkbookNewChart (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::_Chart * Ch);

	//STDMETHODIMP SheetLensGalleryRenderComplete (
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP SheetTableUpdate (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel::TableObject * Target );

	//STDMETHODIMP WorkbookModelChange (
	//	/*[in]*/ Excel::_Workbook * Wb,
	//	/*[in]*/ Excel::ModelChanges * Changes );

	//STDMETHODIMP SheetBeforeDelete (
	//	/*[in]*/ IDispatch * Sh );

	//////////////////////////////////////////////////////////////////////////
	//
	// Office 2010 version
	//
	//////////////////////////////////////////////////////////////////////////


	//STDMETHODIMP NewWorkbook (
	//	/*[in]*/ Excel2010::_Workbook * Wb );

	//STDMETHODIMP SheetSelectionChange (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::Range * Target );

	//STDMETHODIMP SheetBeforeDoubleClick (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::Range * Target,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP SheetBeforeRightClick (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::Range * Target,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP SheetActivate (
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP SheetDeactivate (
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP SheetCalculate (
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP SheetChange (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::Range * Target );

	//STDMETHODIMP WorkbookOpen (
	//	/*[in]*/ Excel2010::_Workbook * Wb );

	//STDMETHODIMP WorkbookActivate (
	//	/*[in]*/ Excel2010::_Workbook * Wb );

	//STDMETHODIMP WorkbookDeactivate (
	//	/*[in]*/ Excel2010::_Workbook * Wb );

	STDMETHODIMP WorkbookBeforeClose (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in,out]*/ VARIANT_BOOL * Cancel );

	STDMETHODIMP WorkbookBeforeSave (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in]*/ VARIANT_BOOL SaveAsUI,
		/*[in,out]*/ VARIANT_BOOL * Cancel );

	STDMETHODIMP WorkbookBeforePrint (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP WorkbookNewSheet (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ IDispatch * Sh );

	//STDMETHODIMP WorkbookAddinInstall (
	//	/*[in]*/ Excel2010::_Workbook * Wb );

	//STDMETHODIMP WorkbookAddinUninstall (
	//	/*[in]*/ Excel2010::_Workbook * Wb );

	//STDMETHODIMP WindowResize (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel2010::Window * Wn );

	STDMETHODIMP WindowActivate (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in]*/ Excel2010::Window * Wn );

	STDMETHODIMP WindowDeactivate (
		/*[in]*/ Excel2010::_Workbook * Wb,
		/*[in]*/ Excel2010::Window * Wn );

	//STDMETHODIMP SheetFollowHyperlink (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::Hyperlink * Target );

	//STDMETHODIMP SheetPivotTableUpdate (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::PivotTable * Target );

	//STDMETHODIMP WorkbookPivotTableCloseConnection (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel2010::PivotTable * Target );

	//STDMETHODIMP WorkbookPivotTableOpenConnection (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel2010::PivotTable * Target );

	//STDMETHODIMP WorkbookSync (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ enum Office2010::MsoSyncEventType SyncEventType );

	//STDMETHODIMP WorkbookBeforeXmlImport (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel::XmlMap * Map,
	//	/*[in]*/ BSTR Url,
	//	/*[in]*/ VARIANT_BOOL IsRefresh,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP WorkbookAfterXmlImport (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel2010::XmlMap * Map,
	//	/*[in]*/ VARIANT_BOOL IsRefresh,
	//	/*[in]*/ enum XlXmlImportResult Result );

	//STDMETHODIMP WorkbookBeforeXmlExport (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel2010::XmlMap * Map,
	//	/*[in]*/ BSTR Url,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP WorkbookAfterXmlExport (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel2010::XmlMap * Map,
	//	/*[in]*/ BSTR Url,
	//	/*[in]*/ enum Excel2010::XlXmlExportResult Result );

	//STDMETHODIMP WorkbookRowsetComplete (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ BSTR Description,
	//	/*[in]*/ BSTR Sheet,
	//	/*[in]*/ VARIANT_BOOL Success );

	//STDMETHODIMP AfterCalculate ( );

	//STDMETHODIMP SheetPivotTableAfterValueChange (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::PivotTable * TargetPivotTable,
	//	/*[in]*/ Excel2010::Range * TargetRange );

	//STDMETHODIMP SheetPivotTableBeforeAllocateChanges (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::PivotTable * TargetPivotTable,
	//	/*[in]*/ long ValueChangeStart,
	//	/*[in]*/ long ValueChangeEnd,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP SheetPivotTableBeforeCommitChanges (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::PivotTable * TargetPivotTable,
	//	/*[in]*/ long ValueChangeStart,
	//	/*[in]*/ long ValueChangeEnd,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP SheetPivotTableBeforeDiscardChanges (
	//	/*[in]*/ IDispatch * Sh,
	//	/*[in]*/ Excel2010::PivotTable * TargetPivotTable,
	//	/*[in]*/ long ValueChangeStart,
	//	/*[in]*/ long ValueChangeEnd );

	//STDMETHODIMP ProtectedViewWindowOpen (
	//	/*[in]*/ Excel2010::ProtectedViewWindow * Pvw );

	//STDMETHODIMP ProtectedViewWindowBeforeEdit (
	//	/*[in]*/ Excel2010::ProtectedViewWindow * Pvw,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP ProtectedViewWindowBeforeClose (
	//	/*[in]*/ Excel2010::ProtectedViewWindow * Pvw,
	//	/*[in]*/ enum Excel2010::XlProtectedViewCloseReason Reason,
	//	/*[in,out]*/ VARIANT_BOOL * Cancel );

	//STDMETHODIMP ProtectedViewWindowResize (
	//	/*[in]*/ Excel2010::ProtectedViewWindow * Pvw );

	STDMETHODIMP ProtectedViewWindowActivate (
		/*[in]*/ Excel2010::ProtectedViewWindow * Pvw );

	STDMETHODIMP ProtectedViewWindowDeactivate (
		/*[in]*/ Excel2010::ProtectedViewWindow * Pvw );

	//STDMETHODIMP WorkbookAfterSave (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ VARIANT_BOOL Success );

	//STDMETHODIMP WorkbookNewChart (
	//	/*[in]*/ Excel2010::_Workbook * Wb,
	//	/*[in]*/ Excel2010::_Chart * Ch );

private:
	ULONG				m_uRefCount;

	typedef enum _ExcelAppEventId {

		WorkbookBeforeClose_Id = 0x00000622,

		WorkbookBeforeSave_Id = 0x00000623,

		WorkbookBeforePrint_Id = 0x00000624,
		
		WindowActivate_Id = 0x00000614,
		
		WindowDeactivate_Id = 0x00000615,
		
		ProtectedViewWindowActivate_Id = 0x00000b5d,
		
		ProtectedViewWindowDeactivate_Id = 0x00000b5e

	}ExcelAppEventId;
};