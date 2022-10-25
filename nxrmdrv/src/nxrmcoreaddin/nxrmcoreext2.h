#pragma once

#include "ext2.h"
#include "mso.h"
#include "mso2010.h"
#include "ribbonrights.h"
#include <map>
#include <nudf\rwlock.hpp>

#ifndef CACHE_LINE
#define CACHE_LINE	64
#endif

#ifndef CACHE_ALIGN
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))
#endif

#define NXRMCOREUI_ONLOAD_PROC_NAME					L"OnLoad"
#define NXRMCOREUI_LOADIMAGE_PROC_NAME				L"LoadImage"
#define NXRMCOREUI_CHECKPERMISSIONUI_PROC_NAME		L"CheckPermissionUI"
#define NXRMCOREUI_PROTECTUI_PROC_NAME				L"ProtectUI"
#define NXRMCOREUI_CHECKBUTTONSTATUS_PROC_NAME		L"CheckButtonStatus"
#define NXRMCOREUI_GETPROTECTBUTTONLABLE_PROC_NAME	L"GetProtectButtonLable"
#define NXRMCOREUI_CHECKMSOBUTTONSTATUS_PROC_NAME	L"CheckMsoButtonStatus"

#define NXRMCOREUI_PROTECT_BUTTON_ID				L"Button_Protect_Id"
#define NXRMCOREUI_CHECKRIGHTS_BUTTON_ID			L"Button_CheckRights_Id"

#define NXRMCOREUI_CM_UI_DLL_NAME				L"nxrmcmui.dll"
#define NXRMCOREADDIN_DLL_NAME					L"nxrmcoreaddin.dll"

#define NXRMCOREUI_PROTECT_LABLE	L"Protect"
#define NXRMCOREUI_CLASSIFY_LABLE	L"Classify"

#define NXRMCOREUI_ONLOAD_ID				(0x8001)
#define NXRMCOREUI_LOADIMAGE_ID				(0x8002)
#define NXRMCOREUI_CHECKPERMISSIONUI_ID		(0x9001)
#define NXRMCOREUI_PROTECTUI_ID				(0x9002)
#define NXRMCOREUI_CHECKBUTTONSTATUS_ID		(0x7001)
#define NXRMCOREUI_GETPROTECTBUTTONLABLE_ID (0x6001)
#define NXRMCOREUI_CHECKMSOBUTTONSTATUS_ID	(0x5001)

typedef enum _nxrmOfficeAppType
{
	OfficeAppInvalid = 0,
	OfficeAppPowerpoint = 0x7001,
	OfficeAppWinword,
	OfficeAppExcel,
	OfficeAppOutlook

}nxrmOfficeAppTyep;

class nxrmCoreExt2 : public AddInDesignerObjects::_IDTExtensibility2, public Office::IRibbonExtensibility
{
public:
	
	nxrmCoreExt2();

	~nxrmCoreExt2();

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

	HRESULT STDMETHODCALLTYPE OnConnection (
		/*[in]*/ IDispatch * Application,
		/*[in]*/ enum AddInDesignerObjects::ext_ConnectMode ConnectMode,
		/*[in]*/ IDispatch * AddInInst,
		/*[in]*/ SAFEARRAY * * custom );

	HRESULT STDMETHODCALLTYPE OnDisconnection (
		/*[in]*/ enum AddInDesignerObjects::ext_DisconnectMode RemoveMode,
		/*[in]*/ SAFEARRAY * * custom );

	HRESULT STDMETHODCALLTYPE OnAddInsUpdate (
		/*[in]*/ SAFEARRAY * * custom );

	HRESULT STDMETHODCALLTYPE OnStartupComplete (
		/*[in]*/ SAFEARRAY * * custom );

	HRESULT STDMETHODCALLTYPE OnBeginShutdown (
		/*[in]*/ SAFEARRAY * * custom );

	HRESULT STDMETHODCALLTYPE GetCustomUI (
		/*[in]*/ BSTR RibbonID,
		/*[out,retval]*/ BSTR * RibbonXml );

	HRESULT STDMETHODCALLTYPE OnLoad (
		/*[in]*/ IDispatch	*RibbonUI );

	HRESULT STDMETHODCALLTYPE LoadImage(
		/*[in]*/ BSTR pbstrImageId,
		/*[out, retval]*/ IPictureDisp **ImageDisp);

	HRESULT STDMETHODCALLTYPE GetCustomUI2010(
		/*[in]*/ BSTR RibbonID,
		/*[out,retval]*/ BSTR * RibbonXml );

	HRESULT STDMETHODCALLTYPE GetCustomUI2013(
		/*[in]*/ BSTR RibbonID,
		/*[out,retval]*/ BSTR * RibbonXml );

	HRESULT STDMETHODCALLTYPE OnCheckPermissonUI2013(
		/*[in]*/ Office::IRibbonControl *pControl, 
		/*[in]*/ VARIANT_BOOL *pvarfPressed);

	HRESULT STDMETHODCALLTYPE OnCheckPermissonUI2010(
		/*[in]*/ Office2010::IRibbonControl *pControl, 
		/*[in]*/ VARIANT_BOOL *pvarfPressed);

	HRESULT STDMETHODCALLTYPE OnProtectUI2013(
		/*[in]*/ Office::IRibbonControl *pControl, 
		/*[in]*/ VARIANT_BOOL *pvarfPressed);

	HRESULT STDMETHODCALLTYPE OnProtectUI2010(
		/*[in]*/ Office2010::IRibbonControl *pControl, 
		/*[in]*/ VARIANT_BOOL *pvarfPressed);

	HRESULT STDMETHODCALLTYPE OnCheckButtonStatus2013(
		/*[in]*/ Office::IRibbonControl *pControl, 
		/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled);

	HRESULT STDMETHODCALLTYPE OnCheckButtonStatus2010(
		/*[in]*/ Office2010::IRibbonControl *pControl, 
		/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled);

	HRESULT STDMETHODCALLTYPE GetProtectButtonLable2013(
		/*[in] */ Office::IRibbonControl *pControl,
		/*[out,retval]*/ BSTR * pbstrLabel);

	HRESULT STDMETHODCALLTYPE GetProtectButtonLable2010(
		/*[in] */ Office2010::IRibbonControl *pControl,
		/*[out,retval]*/ BSTR * pbstrLabel);

	HRESULT STDMETHODCALLTYPE OnCheckMsoButtonStatus2013(
		/*[in]*/ Office::IRibbonControl *pControl,
		/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled);

	HRESULT STDMETHODCALLTYPE OnCheckMsoButtonStatus2010(
		/*[in]*/ Office2010::IRibbonControl *pControl,
		/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled);

private:

	HRESULT Attache2Excel(void);

	HRESULT Attache2Word(void);

	HRESULT Attache2Powerpoint(void);

	HRESULT RefreshMsoControls(void);

private:
	ULONG				m_uRefCount;
	IDispatch			*m_pAppObj;
	IDispatch			*m_pRibbonUI;
	nxrmOfficeAppTyep	m_OfficeAppType;
	nudf::util::CRmuObject	*m_pRmUIObj;

	struct _case_insensitive_cmp
	{
		bool operator()(const std::wstring &str1, const std::wstring &str2) const
		{
			return _wcsicmp(str1.c_str(), str2.c_str()) < 0;
		}
	};

	std::map<std::wstring, RIBBON_ID_INFO, _case_insensitive_cmp>	m_RibbonRightsMap;

	nudf::util::CRwLock												m_RibbonRightsMapLock;		// lock that protect ribbon rights map

	IDispatch			*m_pOfficeEventSink;
};

