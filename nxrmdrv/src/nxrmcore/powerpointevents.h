#pragma once

#include "mso.h"
#include "powerpoint.h"
#include "mso2010.h"
#include "powerpoint2010.h"

class PowerPointEventListener : public PowerPoint::EApplication
{
public:
	PowerPointEventListener();
	
	~PowerPointEventListener();

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

	HRESULT STDMETHODCALLTYPE WindowSelectionChange (
	/*[in]*/ struct PowerPoint::Selection * Sel );
	HRESULT STDMETHODCALLTYPE WindowBeforeRightClick (
	/*[in]*/ struct PowerPoint::Selection * Sel,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE WindowBeforeDoubleClick (
	/*[in]*/ struct PowerPoint::Selection * Sel,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE PresentationClose (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationSave (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationOpen (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE NewPresentation (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationNewSlide (
	/*[in]*/ struct PowerPoint::_Slide * Sld );
	HRESULT STDMETHODCALLTYPE WindowActivate (
	/*[in]*/ struct PowerPoint::_Presentation * Pres,
	/*[in]*/ struct PowerPoint::DocumentWindow * Wn );
	HRESULT STDMETHODCALLTYPE WindowDeactivate (
	/*[in]*/ struct PowerPoint::_Presentation * Pres,
	/*[in]*/ struct PowerPoint::DocumentWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowBegin (
	/*[in]*/ struct PowerPoint::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowNextBuild (
	/*[in]*/ struct PowerPoint::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowNextSlide (
	/*[in]*/ struct PowerPoint::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowEnd (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationPrint (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE SlideSelectionChanged (
	/*[in]*/ struct PowerPoint::SlideRange * SldRange );
	HRESULT STDMETHODCALLTYPE ColorSchemeChanged (
	/*[in]*/ struct PowerPoint::SlideRange * SldRange );
	HRESULT STDMETHODCALLTYPE PresentationBeforeSave (
	/*[in]*/ struct PowerPoint::_Presentation * Pres,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE SlideShowNextClick (
	/*[in]*/ struct PowerPoint::SlideShowWindow * Wn,
	/*[in]*/ struct PowerPoint::Effect * nEffect );
	HRESULT STDMETHODCALLTYPE AfterNewPresentation (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE AfterPresentationOpen (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationSync (
	/*[in]*/ struct PowerPoint::_Presentation * Pres,
		/*[in]*/ enum Office::MsoSyncEventType SyncEventType );
	HRESULT STDMETHODCALLTYPE SlideShowOnNext (
	/*[in]*/ struct PowerPoint::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowOnPrevious (
	/*[in]*/ struct PowerPoint::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE PresentationBeforeClose (
	/*[in]*/ struct PowerPoint::_Presentation * Pres,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowOpen (
	/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowBeforeEdit (
	/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowBeforeClose (
	/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow,
		/*[in]*/ enum PowerPoint::PpProtectedViewCloseReason ProtectedViewCloseReason,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowActivate (
	/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowDeactivate (
	/*[in]*/ struct PowerPoint::ProtectedViewWindow * ProtViewWindow );
	HRESULT STDMETHODCALLTYPE PresentationCloseFinal (
	/*[in]*/ struct PowerPoint::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE AfterDragDropOnSlide (
	/*[in]*/ struct PowerPoint::_Slide * Sld,
		/*[in]*/ float X,
		/*[in]*/ float Y );
	HRESULT STDMETHODCALLTYPE AfterShapeSizeChange (
	/*[in]*/ struct PowerPoint::Shape * shp );

	//////////////////////////////////////////////////////////////////////////
	//
	// Office 2010
	//
	//////////////////////////////////////////////////////////////////////////

	HRESULT STDMETHODCALLTYPE WindowSelectionChange (
	/*[in]*/ struct PowerPoint2010::Selection * Sel );
	HRESULT STDMETHODCALLTYPE WindowBeforeRightClick (
	/*[in]*/ struct PowerPoint2010::Selection * Sel,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE WindowBeforeDoubleClick (
	/*[in]*/ struct PowerPoint2010::Selection * Sel,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE PresentationClose (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationSave (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationOpen (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE NewPresentation (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationNewSlide (
	/*[in]*/ struct PowerPoint2010::_Slide * Sld );
	HRESULT STDMETHODCALLTYPE WindowActivate (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
	/*[in]*/ struct PowerPoint2010::DocumentWindow * Wn );
	HRESULT STDMETHODCALLTYPE WindowDeactivate (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
	/*[in]*/ struct PowerPoint2010::DocumentWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowBegin (
	/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowNextBuild (
	/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowNextSlide (
	/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowEnd (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationPrint (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE SlideSelectionChanged (
	/*[in]*/ struct PowerPoint2010::SlideRange * SldRange );
	HRESULT STDMETHODCALLTYPE ColorSchemeChanged (
	/*[in]*/ struct PowerPoint2010::SlideRange * SldRange );
	HRESULT STDMETHODCALLTYPE PresentationBeforeSave (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE SlideShowNextClick (
	/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn,
	/*[in]*/ struct PowerPoint2010::Effect * nEffect );
	HRESULT STDMETHODCALLTYPE AfterNewPresentation (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE AfterPresentationOpen (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE PresentationSync (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
		/*[in]*/ enum Office::MsoSyncEventType SyncEventType );
	HRESULT STDMETHODCALLTYPE SlideShowOnNext (
	/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE SlideShowOnPrevious (
	/*[in]*/ struct PowerPoint2010::SlideShowWindow * Wn );
	HRESULT STDMETHODCALLTYPE PresentationBeforeClose (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowOpen (
	/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowBeforeEdit (
	/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowBeforeClose (
	/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow,
		/*[in]*/ enum PowerPoint2010::PpProtectedViewCloseReason ProtectedViewCloseReason,
		/*[in,out]*/ VARIANT_BOOL * Cancel );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowActivate (
	/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow );
	HRESULT STDMETHODCALLTYPE ProtectedViewWindowDeactivate (
	/*[in]*/ struct PowerPoint2010::ProtectedViewWindow * ProtViewWindow );
	HRESULT STDMETHODCALLTYPE PresentationCloseFinal (
	/*[in]*/ struct PowerPoint2010::_Presentation * Pres );
	HRESULT STDMETHODCALLTYPE AfterDragDropOnSlide (
	/*[in]*/ struct PowerPoint2010::_Slide * Sld,
		/*[in]*/ float X,
		/*[in]*/ float Y );
	HRESULT STDMETHODCALLTYPE AfterShapeSizeChange (
	/*[in]*/ struct PowerPoint2010::Shape * shp );

private:
	ULONG				m_uRefCount;


	typedef enum _PowerPointAppEventId {

		WindowSelectionChange_Id = 2001,
		WindowBeforeRightClick_Id,
		WindowBeforeDoubleClick_Id,
		PresentationClose_Id,
		PresentationSave_Id,
		PresentationOpen_Id,
		NewPresentation_Id,
		PresentationNewSlide_Id,
		WindowActivate_Id,
		WindowDeactivate_Id,
		SlideShowBegin_Id,
		SlideShowNextBuild_Id,
		SlideShowNextSlide_Id,
		SlideShowEnd_Id,
		PresentationPrint_Id,
		SlideSelectionChanged_Id,
		ColorSchemeChanged_Id,
		PresentationBeforeSave_Id,
		SlideShowNextClick_Id,
		AfterNewPresentation_Id,
		AfterPresentationOpen_Id,
		PresentationSync_Id,
		SlideShowOnNext_Id,
		SlideShowOnPrevious_Id,
		PresentationBeforeClose_Id,
		ProtectedViewWindowOpen_Id,
		ProtectedViewWindowBeforeEdit_Id,
		ProtectedViewWindowBeforeClose_Id,
		ProtectedViewWindowActivate_Id,
		ProtectedViewWindowDeactivate_Id,
		PresentationCloseFinal_Id,
		AfterDragDropOnSlide_Id,
		AfterShapeSizeChange_Id

	}PowerPointAppEventId;
};
