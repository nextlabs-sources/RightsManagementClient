#include "stdafx.h"
#include "nxrmcoreext2.h"
#include "powerpoint.h"
#include "powerpoint2010.h"
#include "excel.h"
#include "excel2010.h"
#include "outlook.h"
#include "outlook2010.h"
#include "word.h"
#include "word2010.h"
#include "nxrmdrv.h"
#include "nxrmdrvman.h"
#include "nxrmcorehlp.h"
#include "resource.h"
#include "assert.h"
#include "excelevents.h"
#include "wordevents.h"
#include "powerpointevents.h"

extern CACHE_ALIGN	LONG	g_unxrmcoreext2InstanceCount;
extern CACHE_ALIGN	HMODULE g_hModule;
extern CACHE_ALIGN	BOOL	g_bIsOffice2010;
// {0CCA3189-F325-4D58-AB6D-212CD76C3311}
extern "C" const GUID CLSID_nxrmCoreExt2 = { 0xcca3189, 0xf325, 0x4d58, { 0xab, 0x6d, 0x21, 0x2c, 0xd7, 0x6c, 0x33, 0x11 } };

extern "C" const std::vector<RIBBON_ID_INFO> g_excel_15_ribbon_info = { \
	{L"TabInfo", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabOfficeStart", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabRecent", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileClose", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSave", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FilePrintQuick", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabSave", BUILTIN_RIGHT_SAVEAS | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPrint", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabShare", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabPublish", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ApplicationOptionsDialog", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"AdvancedFileProperties", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"UpgradeDocument", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSendAsAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsPdfEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsXpsEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileInternetFax", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabHome", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInsert", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabFormulas", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabReview", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabData", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"SheetMoveOrCopy", BUILTIN_RIGHT_EDIT |BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenshotInsertGallery", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenClipping", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectctInsert", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Paste", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Cut", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Copy", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"CopyAsPicture", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL} \
};

extern "C" const std::vector<RIBBON_ID_INFO> g_excel_14_ribbon_info = { \
	{L"TabInfo", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileOpen", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabRecent", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileClose", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSave", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FilePrintQuick", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSaveAs", BUILTIN_RIGHT_SAVEAS | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPrint", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabNew", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabShare", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"ApplicationOptionsDialog", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"AdvancedFileProperties", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"UpgradeDocument", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSendAsAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsPdfEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsXpsEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileInternetFax", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabHome", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInsert", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabFormulas", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabReview", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabTeam", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"SheetMoveOrCopy", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenshotInsertGallery", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenClipping", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectctInsert", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Paste", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Cut", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Copy", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL} \
};

extern "C" const std::vector<RIBBON_ID_INFO> g_word_15_ribbon_info = { \
	{L"TabInfo", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabOfficeStart", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabRecent", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileClose", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSave", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FilePrintQuick", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabSave", BUILTIN_RIGHT_SAVEAS | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPrint", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabShare", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabPublish", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ApplicationOptionsDialog", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"UpgradeDocument", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSendAsAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsPdfEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsXpsEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileInternetFax", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabHome", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInsert", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabWordDesign", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPageLayoutWord", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabReferences", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabMailings", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabReviewWord", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenshotInsertGallery", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenClipping", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectInsertMenu", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectctInsert", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Paste", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Cut", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Copy", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL} \
};

extern "C" const std::vector<RIBBON_ID_INFO> g_word_14_ribbon_info = { \
	{L"FileSave", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSaveAs", BUILTIN_RIGHT_SAVEAS | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileOpen", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInfo", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabRecent", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileClose", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FilePrintQuick", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabNew", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPrint", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabShare", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"ApplicationOptionsDialog", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"UpgradeDocument", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSendAsAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsPdfEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsXpsEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileInternetFax", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabHome", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInsert", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPageLayoutWord", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabReferences", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabMailings", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabReviewWord", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabView", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenshotInsertGallery", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenClipping", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectInsertMenu", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectctInsert", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Paste", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Cut", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Copy", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL} \
};

extern "C" const std::vector<RIBBON_ID_INFO> g_powerpoint_15_ribbon_info = { \
	{L"TabInfo", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabOfficeStart", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabRecent", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileClose", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSave", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FilePrintQuick", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabSave", BUILTIN_RIGHT_SAVEAS | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPrint", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabShare", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabPublish", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ApplicationOptionsDialog", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"AdvancedFileProperties", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"UpgradeDocument", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSendAsAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsPdfEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsXpsEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileInternetFax", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabHome", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInsert", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabDesign", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabTransitions", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabAnimations", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabSlideShow", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabReview", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabDeveloper", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabView", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenshotInsertGallery", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenClipping", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectctInsert", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectInsertMenu", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Paste", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Cut", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Copy", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL} \
};

extern "C" const std::vector<RIBBON_ID_INFO> g_powerpoint_14_ribbon_info = { \
	{L"FileSave", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSaveAs", BUILTIN_RIGHT_SAVEAS | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileOpen", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FilePrintQuick", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInfo", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabRecent", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileClose", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabNew", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabPrint", BUILTIN_RIGHT_PRINT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabShare", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"ApplicationOptionsDialog", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"AdvancedFileProperties", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"UpgradeDocument", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"FileSendAsAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsPdfEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileEmailAsXpsEmailAttachment", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"FileInternetFax", BUILTIN_RIGHT_SEND, 0ULL}, \
	{L"TabHome", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabInsert", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabDesign", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabTransitions", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabAnimations", BUILTIN_RIGHT_EDIT | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabSlideShow", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabReview", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabDeveloper", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"TabView", BUILTIN_RIGHT_VIEW | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenshotInsertGallery", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"ScreenClipping", BUILTIN_RIGHT_SCREENCAP | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectctInsert", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"OleObjectInsertMenu", BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Paste", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Cut", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL}, \
	{L"Copy", BUILTIN_RIGHT_CLIPBOARD | BUILTIN_RIGHT_DECRYPT, 0ULL} \
};

static void print_guid(const GUID &id)
{
	char buf[128];

	ULONG p1;
	ULONG p2;
	ULONG p3;

	ULONG p4[8];

	memset(buf,0,sizeof(buf));

	p1 = id.Data1;
	p2 = id.Data2;
	p3 = id.Data3;

	for(int i = 0; i < 8; i++)
	{
		p4[i] = id.Data4[i];
	}


	sprintf_s(buf,
		sizeof(buf),
		"%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x\n",
		p1,
		p2,
		p3,
		p4[0],
		p4[1],
		p4[2],
		p4[3],
		p4[4],
		p4[5],
		p4[6],
		p4[7]);

	OutputDebugStringA(buf);
}

static BSTR LoadCustomUIFromFile(WCHAR *FileName);
static BOOL get_classify_ui(WCHAR *FileName, WCHAR *GroupName);
static BOOL check_enable_protect_menu(void);
static void PairToBuffer(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& pairs, _Out_ std::vector<WCHAR>& buf);
static void BufferToPair(_In_ LPCWSTR buf, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& pairs);

static BOOL excel2013_get_fullname_in_protected_mode(Excel::_Application *pApp, BSTR *DocFullName);
static BOOL excel2010_get_fullname_in_protected_mode(Excel2010::_Application *pApp, BSTR *DocFullName);
static BOOL powerpoint2013_get_fullname_in_protected_mode(PowerPoint::_Application *pApp, BSTR *DocFullName);
static BOOL powerpoint2010_get_fullname_in_protected_mode(PowerPoint2010::_Application *pApp, BSTR *DocFullName);

nxrmCoreExt2::nxrmCoreExt2()
{
	m_uRefCount = 1;
	m_pAppObj = NULL;
	m_OfficeAppType = OfficeAppInvalid;
	m_pRmUIObj = NULL;

	m_pOfficeEventSink = NULL;
}

nxrmCoreExt2::~nxrmCoreExt2()
{
	if (m_pRmUIObj)
	{
		m_pRmUIObj->RmuDeinitialize();
		m_pRmUIObj->Clear();

		delete m_pRmUIObj;
		m_pRmUIObj = NULL;
	}

	if (m_pOfficeEventSink)
	{
		delete m_pOfficeEventSink;
		m_pOfficeEventSink = NULL;
	}

	nudf::util::CRwExclusiveLocker lock(&m_RibbonRightsMapLock);

	m_RibbonRightsMap.clear();
	
	return;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::QueryInterface( 
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
		else if (__uuidof(AddInDesignerObjects::_IDTExtensibility2) == riid)
		{
			punk = (AddInDesignerObjects::_IDTExtensibility2*)this;
		}
		else if (__uuidof(Office::IRibbonExtensibility) == riid)
		{
			punk = (Office::IRibbonExtensibility*)this;
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

ULONG STDMETHODCALLTYPE nxrmCoreExt2::AddRef(void)
{
	m_uRefCount++;

	return m_uRefCount;
}	

ULONG STDMETHODCALLTYPE nxrmCoreExt2::Release(void)
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount)
	{
		delete this;
		InterlockedDecrement(&g_unxrmcoreext2InstanceCount);
	}

	return uCount;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetTypeInfoCount( 
	/* [out] */ __RPC__out UINT *pctinfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetTypeInfo( 
	/* [in] */ UINT iTInfo,
	/* [in] */ LCID lcid,
	/* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetIDsOfNames( 
	/* [in] */ __RPC__in REFIID riid,
	/* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
	/* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
	/* [in] */ LCID lcid,
	/* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId)
{
	HRESULT hr = DISP_E_UNKNOWNNAME;

	UINT i = 0;

	for (i = 0; i < cNames; i++)
	{
		if (wcscmp(rgszNames[i], NXRMCOREUI_ONLOAD_PROC_NAME) == 0)
		{
			rgDispId[i] = NXRMCOREUI_ONLOAD_ID;
			hr = S_OK;
		}
		else if (wcscmp(rgszNames[i], NXRMCOREUI_LOADIMAGE_PROC_NAME) == 0)
		{
			rgDispId[i] = NXRMCOREUI_LOADIMAGE_ID;
			hr = S_OK;
		}
		else if (wcscmp(rgszNames[i], NXRMCOREUI_CHECKPERMISSIONUI_PROC_NAME) == 0)
		{
			rgDispId[i] = NXRMCOREUI_CHECKPERMISSIONUI_ID;
			hr = S_OK;
		}
		else if (wcscmp(rgszNames[i], NXRMCOREUI_PROTECTUI_PROC_NAME) == 0)
		{
			rgDispId[i] = NXRMCOREUI_PROTECTUI_ID;
			hr = S_OK;
		}
		else if (wcscmp(rgszNames[i], NXRMCOREUI_CHECKBUTTONSTATUS_PROC_NAME) == 0)
		{
			rgDispId[i] = NXRMCOREUI_CHECKBUTTONSTATUS_ID;
			hr = S_OK;
		}
		else if (wcscmp(rgszNames[i], NXRMCOREUI_GETPROTECTBUTTONLABLE_PROC_NAME) == 0)
		{
			rgDispId[i] = NXRMCOREUI_GETPROTECTBUTTONLABLE_ID;
			hr = S_OK;
		}
		else if (wcscmp(rgszNames[i], NXRMCOREUI_CHECKMSOBUTTONSTATUS_PROC_NAME) == 0)
		{
			rgDispId[i] = NXRMCOREUI_CHECKMSOBUTTONSTATUS_ID;
			hr = S_OK;
		}
		else
		{
			rgDispId[i] = DISPID_UNKNOWN;
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::Invoke( 
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

	IDispatch *pPicDisp = NULL;
	VARIANT_BOOL bEnable = VARIANT_FALSE;
	BSTR Lable = NULL;

	do 
	{
		switch (dispIdMember)
		{
		case NXRMCOREUI_ONLOAD_ID:
			// (parameters are on stack, thus in reverse order)
			hr = OnLoad((Office::IRibbonUI *)pDispParams->rgvarg[0].pdispVal);

			break;
			
		case NXRMCOREUI_LOADIMAGE_ID:
			// (parameters are on stack, thus in reverse order)
			hr = LoadImage(pDispParams->rgvarg[0].bstrVal, (IPictureDisp **)(&pPicDisp));
			
			if (SUCCEEDED(hr))
			{
				pVarResult->vt			= VT_DISPATCH;
				pVarResult->pdispVal	= pPicDisp;
			}

			break;

		case NXRMCOREUI_CHECKPERMISSIONUI_ID:

			if (g_bIsOffice2010)
			{
				hr = OnCheckPermissonUI2010((Office2010::IRibbonControl*)(pDispParams->rgvarg[1].pdispVal), pDispParams->rgvarg[0].pboolVal);
			}
			else
			{
				hr = OnCheckPermissonUI2013((Office::IRibbonControl*)(pDispParams->rgvarg[1].pdispVal), pDispParams->rgvarg[0].pboolVal);
			}

			break;

		case NXRMCOREUI_PROTECTUI_ID:

			if (g_bIsOffice2010)
			{
				hr = OnProtectUI2010((Office2010::IRibbonControl*)(pDispParams->rgvarg[1].pdispVal), pDispParams->rgvarg[0].pboolVal);
			}
			else
			{
				hr = OnProtectUI2013((Office::IRibbonControl*)(pDispParams->rgvarg[1].pdispVal), pDispParams->rgvarg[0].pboolVal);
			}

			break;

		case NXRMCOREUI_CHECKBUTTONSTATUS_ID:

			if (g_bIsOffice2010)
			{
				hr = OnCheckButtonStatus2010((Office2010::IRibbonControl*)(pDispParams->rgvarg[0].pdispVal), &bEnable);

				if (SUCCEEDED(hr))
				{
					pVarResult->vt			= VT_BOOL;
					pVarResult->boolVal		= bEnable;
				}
			}
			else
			{
				hr = OnCheckButtonStatus2013((Office::IRibbonControl*)(pDispParams->rgvarg[0].pdispVal), &bEnable);

				if (SUCCEEDED(hr))
				{
					pVarResult->vt			= VT_BOOL;
					pVarResult->boolVal		= bEnable;
				}
			}

			break;

		case NXRMCOREUI_CHECKMSOBUTTONSTATUS_ID:

			if (g_bIsOffice2010)
			{
				hr = OnCheckMsoButtonStatus2010((Office2010::IRibbonControl*)(pDispParams->rgvarg[0].pdispVal), &bEnable);

				if (SUCCEEDED(hr))
				{
					pVarResult->vt = VT_BOOL;
					pVarResult->boolVal = bEnable;
				}
			}
			else
			{
				hr = OnCheckMsoButtonStatus2013((Office::IRibbonControl*)(pDispParams->rgvarg[0].pdispVal), &bEnable);

				if (SUCCEEDED(hr))
				{
					pVarResult->vt = VT_BOOL;
					pVarResult->boolVal = bEnable;
				}
			}

			break;

		case NXRMCOREUI_GETPROTECTBUTTONLABLE_ID:

			if (g_bIsOffice2010)
			{
				hr = GetProtectButtonLable2010((Office2010::IRibbonControl*)(pDispParams->rgvarg[0].pdispVal), &Lable);

				if (SUCCEEDED(hr))
				{
					pVarResult->vt			= VT_BSTR;
					pVarResult->bstrVal		= Lable;
				}
			}
			else
			{
				hr = GetProtectButtonLable2013((Office::IRibbonControl*)(pDispParams->rgvarg[0].pdispVal), &Lable);

				if (SUCCEEDED(hr))
				{
					pVarResult->vt			= VT_BSTR;
					pVarResult->bstrVal		= Lable;
				}
			}

			break;

		default:
			break;

		}

	} while (FALSE);

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnConnection (
	/*[in]*/ IDispatch * Application,
	/*[in]*/ enum AddInDesignerObjects::ext_ConnectMode ConnectMode,
	/*[in]*/ IDispatch * AddInInst,
	/*[in]*/ SAFEARRAY * * custom )
{
	HRESULT hr = S_OK;

	PVOID pAppObj = NULL;

	nxrmOfficeAppTyep OfficeAppType = OfficeAppInvalid;

	WCHAR module_path[MAX_PATH] = {0};

	ULONG module_path_length = 0;

	WCHAR *p = NULL;

	if (Application && (m_pAppObj == NULL))
	{
		do 
		{
			hr = Application->QueryInterface(__uuidof(Word::_Application), &pAppObj);

			if (S_OK == hr)
			{
				OfficeAppType = OfficeAppWinword;

				if (g_bIsOffice2010)
				{
					for (const auto &ite : g_word_14_ribbon_info)
					{
						m_RibbonRightsMap.insert(std::pair<std::wstring, RIBBON_ID_INFO>(ite.RibbonId, ite));
					}
				}
				else
				{
					for (const auto &ite : g_word_15_ribbon_info)
					{
						m_RibbonRightsMap.insert(std::pair<std::wstring, RIBBON_ID_INFO>(ite.RibbonId, ite));
					}
				}

				break;
			}

			hr = Application->QueryInterface(__uuidof(PowerPoint::_Application), &pAppObj);

			if (S_OK == hr)
			{
				OfficeAppType = OfficeAppPowerpoint;

				if (g_bIsOffice2010)
				{
					for (const auto &ite : g_powerpoint_14_ribbon_info)
					{
						m_RibbonRightsMap.insert(std::pair<std::wstring, RIBBON_ID_INFO>(ite.RibbonId, ite));
					}
				}
				else
				{
					for (const auto &ite : g_powerpoint_15_ribbon_info)
					{
						m_RibbonRightsMap.insert(std::pair<std::wstring, RIBBON_ID_INFO>(ite.RibbonId, ite));
					}
				}

				break;
			}

			hr = Application->QueryInterface(__uuidof(Excel::_Application), &pAppObj);

			if (S_OK == hr)
			{
				OfficeAppType = OfficeAppExcel;

				if (g_bIsOffice2010)
				{
					for (const auto &ite : g_excel_14_ribbon_info)
					{
						m_RibbonRightsMap.insert(std::pair<std::wstring, RIBBON_ID_INFO>(ite.RibbonId, ite));
					}

				}
				else
				{
					for (const auto &ite : g_excel_15_ribbon_info)
					{
						m_RibbonRightsMap.insert(std::pair<std::wstring, RIBBON_ID_INFO>(ite.RibbonId, ite));
					}
				}

				break;
			}

			hr = Application->QueryInterface(__uuidof(Outlook::_Application), &pAppObj);

			if (S_OK == hr)
			{
				OfficeAppType = OfficeAppOutlook;
				break;
			}

		} while (FALSE);

		if (pAppObj)
		{
			if (NULL == InterlockedCompareExchangePointer((PVOID*)&m_pAppObj, pAppObj, NULL))
			{
				m_OfficeAppType = OfficeAppType;

				if (OfficeAppType != OfficeAppOutlook)
				{
					module_path_length = GetModuleFileNameW(g_hModule, module_path, (sizeof(module_path)/sizeof(WCHAR) - 1));

					if (module_path_length)
					{
						p = wcsstr(module_path, NXRMCOREADDIN_DLL_NAME);

						if (p && (sizeof(NXRMCOREADDIN_DLL_NAME) >= sizeof(NXRMCOREUI_CM_UI_DLL_NAME)))
						{
							memcpy(p, NXRMCOREUI_CM_UI_DLL_NAME, sizeof(NXRMCOREUI_CM_UI_DLL_NAME));

							assert(m_pRmUIObj == NULL);

							m_pRmUIObj = new nudf::util::CRmuObject;

							if (m_pRmUIObj)
							{
								m_pRmUIObj->Initialize(module_path);
								m_pRmUIObj->RmuInitialize();
							}
						}
					}
				}
			}
		}
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnDisconnection (
	/*[in]*/ enum AddInDesignerObjects::ext_DisconnectMode RemoveMode,
	/*[in]*/ SAFEARRAY * * custom )
{
	HRESULT hr = S_OK;

	if (m_pAppObj)
	{
		//
		// we have to release because we got the pointer by using QueryInterface()
		//
		m_pAppObj->Release();
		m_pAppObj = NULL;
		m_OfficeAppType = OfficeAppInvalid;
		
		if (m_pRmUIObj)
		{
			m_pRmUIObj->RmuDeinitialize();
			m_pRmUIObj->Clear();

			delete m_pRmUIObj;
			m_pRmUIObj = NULL;
		}
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnAddInsUpdate (
	/*[in]*/ SAFEARRAY * * custom )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnStartupComplete (
	/*[in]*/ SAFEARRAY * * custom )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnBeginShutdown (
	/*[in]*/ SAFEARRAY * * custom )
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetCustomUI(BSTR RibbonID, BSTR * RibbonXml)
{
	if (!g_bIsOffice2010)
	{
		return GetCustomUI2013(RibbonID, RibbonXml);
	}
	else
	{
		return GetCustomUI2010(RibbonID, RibbonXml);
	}
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetCustomUI2013(BSTR RibbonID, BSTR * RibbonXml)
{
	HRESULT hr = S_OK;

	OFFICE_GET_CUSTOMUI_REQUEST		Request = {0};
	OFFICE_GET_CUSTOMUI_RESPONSE	Resp = {0};

	Office::LanguageSettings *pLangObj = NULL;

	BSTR VerString = NULL;

	PVOID Section = NULL;

	int LangId = 0;

	NXCONTEXT Ctx = NULL;

	BSTR CustomUIXML = NULL;

	USHORT OfficeProduct = 0;

	ULONG RespSize = sizeof(Resp);

	do 
	{
		if (!m_pAppObj)
		{
			break;
		}

		if (m_OfficeAppType == OfficeAppOutlook)
		{
			break;
		}

		Section = init_transporter_client();

		if (!Section)
		{
			break;
		}

		if (!is_transporter_enabled(Section))
		{
			break;
		}

		memset(&Request, 0 , sizeof(Request));

		switch (m_OfficeAppType)
		{
		case OfficeAppPowerpoint:

			OfficeProduct = NXRMDRV_OFFICE_POWERPOINT;

			do 
			{
				PowerPoint::_Application *pPowerPointAppObj = NULL;

				pPowerPointAppObj = (PowerPoint::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_Version(&VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pPowerPointAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		case OfficeAppWinword:

			OfficeProduct = NXRMDRV_OFFICE_WINWORD;

			do 
			{
				Word::_Application *pWinwordAppObj = NULL;

				pWinwordAppObj = (Word::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_Version(&VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pWinwordAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		case OfficeAppExcel:

			OfficeProduct = NXRMDRV_OFFICE_EXCEL;

			do 
			{
				Excel::_Application *pExcelAppObj = NULL;

				pExcelAppObj = (Excel::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_Version(0x0409/*US English*/, &VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pExcelAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		case OfficeAppOutlook:

			OfficeProduct = NXRMDRV_OFFICE_OUTLOOK;

			do 
			{
				Outlook::_Application *pOutlookAppObj = NULL;

				pOutlookAppObj = (Outlook::_Application *)m_pAppObj;

				hr = pOutlookAppObj->get_Version(&VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pOutlookAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		default:

			break;
		}
	
		//
		// use pLangObj as a flag to indicate that we successfully get all information we need
		//
		if (!pLangObj)
		{
			break;
		}

		Request.ProcessId			= GetCurrentProcessId();
		Request.ThreadId			= GetCurrentThreadId();
		Request.OfficeProduct		= OfficeProduct;
		Request.OfficeLanguageId	= (USHORT)LangId;

		memcpy(Request.OfficeVersion, 
			   VerString, 
			   min(wcslen(VerString)*sizeof(WCHAR), sizeof(Request.OfficeVersion) - sizeof(WCHAR)));

		if (!GetTempPathW(sizeof(Request.TempPath)/sizeof(WCHAR), Request.TempPath))
		{
			break;
		}

		if (!GetLongPathNameW(Request.TempPath, Request.TempPath, sizeof(Request.TempPath)/sizeof(WCHAR)))
		{
			break;
		}

		Ctx = submit_request(Section, NXRMDRV_MSG_TYPE_GET_CUSTOMUI, &Request, sizeof(Request));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Section, &Resp, sizeof(Resp), &RespSize))
		{
			Ctx = NULL;
			break;
		}

		CustomUIXML = LoadCustomUIFromFile(Resp.CustomUIFileName);

		if (!CustomUIXML)
		{
			break;
		}

	} while (FALSE);

	if (pLangObj)
	{
		pLangObj->Release();
	}

	if (Section)
	{
		close_transporter_client(Section);
		Section = NULL;
	}

	*RibbonXml = CustomUIXML;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetCustomUI2010(BSTR RibbonID, BSTR * RibbonXml)
{
	HRESULT hr = S_OK;

	OFFICE_GET_CUSTOMUI_REQUEST		Request = {0};
	OFFICE_GET_CUSTOMUI_RESPONSE	Resp = {0};

	Office2010::LanguageSettings *pLangObj = NULL;

	BSTR VerString = NULL;

	PVOID Section = NULL;

	int LangId = 0;

	NXCONTEXT Ctx = NULL;

	BSTR CustomUIXML = NULL;

	USHORT OfficeProduct = 0;

	ULONG RespSize = sizeof(Resp);

	do 
	{
		if (!m_pAppObj)
		{
			break;
		}

		if (m_OfficeAppType == OfficeAppOutlook)
		{
			break;
		}

		Section = init_transporter_client();

		if (!Section)
		{
			break;
		}

		if (!is_transporter_enabled(Section))
		{
			break;
		}

		memset(&Request, 0 , sizeof(Request));

		switch (m_OfficeAppType)
		{
		case OfficeAppPowerpoint:

			OfficeProduct = NXRMDRV_OFFICE_POWERPOINT;

			do 
			{
				PowerPoint2010::_Application *pPowerPointAppObj = NULL;

				pPowerPointAppObj = (PowerPoint2010::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_Version(&VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pPowerPointAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office2010::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		case OfficeAppWinword:

			OfficeProduct = NXRMDRV_OFFICE_WINWORD;

			do 
			{
				Word2010::_Application *pWinwordAppObj = NULL;

				pWinwordAppObj = (Word2010::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_Version(&VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pWinwordAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office2010::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		case OfficeAppExcel:

			OfficeProduct = NXRMDRV_OFFICE_EXCEL;

			do 
			{
				Excel2010::_Application *pExcelAppObj = NULL;

				pExcelAppObj = (Excel2010::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_Version(0x0409/*US English*/, &VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pExcelAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office2010::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		case OfficeAppOutlook:

			OfficeProduct = NXRMDRV_OFFICE_OUTLOOK;

			do 
			{
				Outlook2010::_Application *pOutlookAppObj = NULL;

				pOutlookAppObj = (Outlook2010::_Application *)m_pAppObj;

				hr = pOutlookAppObj->get_Version(&VerString);

				if (FAILED(hr))
				{
					break;
				}

				hr = pOutlookAppObj->get_LanguageSettings(&pLangObj);

				if (FAILED(hr))
				{
					break;
				}

				pLangObj->get_LanguageID(Office2010::msoLanguageIDUI, &LangId);

			} while (FALSE);

			break;

		default:

			break;
		}

		//
		// use pLangObj as a flag to indicate that we successfully get all information we need
		//
		if (!pLangObj)
		{
			break;
		}

		Request.ProcessId			= GetCurrentProcessId();
		Request.ThreadId			= GetCurrentThreadId();
		Request.OfficeProduct		= OfficeProduct;
		Request.OfficeLanguageId	= (USHORT)LangId;

		memcpy(Request.OfficeVersion, 
			   VerString, 
			   min(wcslen(VerString)*sizeof(WCHAR), sizeof(Request.OfficeVersion) - sizeof(WCHAR)));

		if (!GetTempPathW(sizeof(Request.TempPath)/sizeof(WCHAR), Request.TempPath))
		{
			break;
		}

		if (!GetLongPathNameW(Request.TempPath, Request.TempPath, sizeof(Request.TempPath)/sizeof(WCHAR)))
		{
			break;
		}

		Ctx = submit_request(Section, NXRMDRV_MSG_TYPE_GET_CUSTOMUI, &Request, sizeof(Request));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Section, &Resp, sizeof(Resp), &RespSize))
		{
			Ctx = NULL;
			break;
		}

		CustomUIXML = LoadCustomUIFromFile(Resp.CustomUIFileName);

		if (!CustomUIXML)
		{
			break;
		}

	} while (FALSE);

	if (pLangObj)
	{
		pLangObj->Release();
	}

	if (Section)
	{
		close_transporter_client(Section);
		Section = NULL;
	}

	*RibbonXml = CustomUIXML;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnLoad(IDispatch *RibbonUI)
{
	HRESULT hr = S_OK;

	m_pRibbonUI = RibbonUI;

	if (m_OfficeAppType == OfficeAppExcel)
	{
		Attache2Excel();
	}
	else if (m_OfficeAppType == OfficeAppPowerpoint)
	{
		Attache2Powerpoint();
	}
	else if (m_OfficeAppType == OfficeAppWinword)
	{
		Attache2Word();
	}
	else
	{

	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::LoadImage(BSTR pbstrImageId, IPictureDisp **ImageDisp)
{
	HRESULT hr = S_FALSE;

	PICTDESC ImageDesc = {0};

	if (_wcsicmp(pbstrImageId, L"CheckPermission") == 0)
	{
		ImageDesc.cbSizeofstruct	= sizeof(ImageDesc);
		ImageDesc.picType			= PICTYPE_ICON;
		ImageDesc.icon.hicon		= ::LoadIconW(g_hModule, MAKEINTRESOURCE(IDI_ICON_CHECKPERMISSION));

		hr = OleCreatePictureIndirect(&ImageDesc, IID_IPictureDisp, TRUE, (void**)ImageDisp);
	}
	else if (_wcsicmp(pbstrImageId, L"Protect") == 0)
	{
		ImageDesc.cbSizeofstruct	= sizeof(ImageDesc);
		ImageDesc.picType			= PICTYPE_ICON;
		ImageDesc.icon.hicon		= ::LoadIconW(g_hModule, MAKEINTRESOURCE(IDI_ICON_PROTECT));

		hr = OleCreatePictureIndirect(&ImageDesc, IID_IPictureDisp, TRUE, (void**)ImageDisp);
	}
	else
	{

	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnCheckPermissonUI2013(
	/*[in]*/ Office::IRibbonControl *pControl, 
	/*[in]*/ VARIANT_BOOL *pvarfPressed)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	HWND hWnd = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	hWnd = GetActiveWindow();

	switch (m_OfficeAppType)
	{
	case OfficeAppWinword:
		{
			Word::_Application *pWinwordAppObj = NULL;
			Word::_Document *pDoc = NULL;

			do 
			{
				pWinwordAppObj = (Word::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				//
				// take care of rights
				//
				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				m_pRmUIObj->RmuShowPermissionPropPage(hWnd, DocFullName, RightsMask, NULL);

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}
		
		break;

	case OfficeAppPowerpoint:

		{
			PowerPoint::_Application *pPowerPointAppObj = NULL;
			PowerPoint::_Presentation *pPres = NULL;

			do 
			{
				pPowerPointAppObj = (PowerPoint::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pPres->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				//
				// take care of rights
				//
				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				m_pRmUIObj->RmuShowPermissionPropPage(hWnd, DocFullName, RightsMask, NULL);

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}
		}

		break;

	case OfficeAppExcel:

		{
			Excel::_Application *pExcelAppObj = NULL;
			Excel::_Workbook	*pWb = NULL;

			do 
			{
				pExcelAppObj = (Excel::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (!SUCCEEDED(hr) || pWb == NULL)
				{
					break;
				}

				hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				//
				// take care of rights
				//
				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				m_pRmUIObj->RmuShowPermissionPropPage(hWnd, DocFullName, RightsMask, NULL);

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}
		}

		break;

	default:
		break;
	}

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr; 
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnCheckPermissonUI2010(
	/*[in]*/ Office2010::IRibbonControl *pControl, 
	/*[in]*/ VARIANT_BOOL *pvarfPressed)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	HWND hWnd = NULL;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	hWnd = GetActiveWindow();

	switch (m_OfficeAppType)
	{
	case OfficeAppWinword:
		{
			Word2010::_Application *pWinwordAppObj = NULL;
			Word2010::_Document *pDoc = NULL;

			do 
			{
				pWinwordAppObj = (Word2010::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				//
				// take care of rights
				//
				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				m_pRmUIObj->RmuShowPermissionPropPage(hWnd, DocFullName, RightsMask, NULL);

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}

		break;

	case OfficeAppPowerpoint:

		{
			PowerPoint2010::_Application *pPowerPointAppObj = NULL;
			PowerPoint2010::_Presentation *pPres = NULL;

			do 
			{
				pPowerPointAppObj = (PowerPoint2010::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pPres->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				//
				// take care of rights
				//
				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				m_pRmUIObj->RmuShowPermissionPropPage(hWnd, DocFullName, RightsMask, NULL);

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}
		}

		break;

	case OfficeAppExcel:

		{
			Excel2010::_Application *pExcelAppObj = NULL;
			Excel2010::_Workbook	*pWb = NULL;

			do 
			{
				pExcelAppObj = (Excel2010::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (!SUCCEEDED(hr) || pWb == NULL)
				{
					break;
				}

				hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				//
				// take care of rights
				//
				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				m_pRmUIObj->RmuShowPermissionPropPage(hWnd, DocFullName, RightsMask, NULL);

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}
		}

		break;

	default:
		break;
	}

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr; 
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnProtectUI2013(
	/*[in]*/ Office::IRibbonControl *pControl, 
	/*[in]*/ VARIANT_BOOL *pvarfPressed)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	WCHAR ClassifyUIXMLFileName[MAX_PATH] = {0};
	WCHAR GroupName[MAX_PATH] = {0};

	std::vector<std::pair<std::wstring,std::wstring>> tag;

	std::vector<WCHAR> wzTag;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	UCHAR *pTagData = NULL;
	
	USHORT TagDataLength = 0;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	BOOL IsEncryptedFile = TRUE;

	BOOL SkipProcessing = TRUE;

	HWND hWnd = GetActiveWindow();

	do 
	{

		switch (m_OfficeAppType)
		{
		case OfficeAppWinword:
			{
				Word::_Application *pWinwordAppObj = NULL;
				Word::_Document *pDoc = NULL;

				do 
				{
					pWinwordAppObj = (Word::_Application *)m_pAppObj;

					hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					hr = pDoc->get_FullName(&DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					if (!DocFullName)
					{
						break;
					}

					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
					{
						hr = pDoc->Save();

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pDoc->get_FullName(&DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
						{
							break;
						}
					}

					SkipProcessing = FALSE;

				} while (FALSE);

				if (pDoc)
				{
					pDoc->Release();
					pDoc = NULL;
				}
			}

			break;

		case OfficeAppPowerpoint:

			{
				PowerPoint::_Application *pPowerPointAppObj = NULL;
				PowerPoint::_Presentation *pPres = NULL;
				Office::FileDialog	*pSaveDialog = NULL;
				Office::FileDialogSelectedItems	*pSelectedFiles = NULL;

				int SaveRet = 0;

				do 
				{
					pPowerPointAppObj = (PowerPoint::_Application *)m_pAppObj;

					hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					hr = pPres->get_FullName(&DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					if (!DocFullName)
					{
						break;
					}

					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
					{
						hr = pPowerPointAppObj->get_FileDialog(Office::msoFileDialogSaveAs, &pSaveDialog);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						pSaveDialog->put_AllowMultiSelect(VARIANT_FALSE);
						pSaveDialog->put_InitialFileName(DocFullName);

						hr = pSaveDialog->Show(&SaveRet);

						if (!SaveRet)
						{
							break;
						}

						hr = pSaveDialog->get_SelectedItems(&pSelectedFiles);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pSelectedFiles->Item(1, &DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pPres->SaveAs(DocFullName, PowerPoint::ppSaveAsDefault, Office::msoTriStateMixed);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pPres->get_FullName(&DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
						{
							break;
						}
					}

					SkipProcessing = FALSE;

				} while (FALSE);

				if (pSelectedFiles)
				{
					pSelectedFiles->Release();
					pSelectedFiles = NULL;
				}

				if (pSaveDialog)
				{
					pSaveDialog->Release();
					pSaveDialog = NULL;
				}

				if (pPres)
				{
					pPres->Release();
					pPres = NULL;
				}
			}

			break;

		case OfficeAppExcel:

			{
				Excel::_Application *pExcelAppObj = NULL;
				Excel::_Workbook	*pWb = NULL;

				Office::FileDialog	*pSaveDialog = NULL;
				Office::FileDialogSelectedItems	*pSelectedFiles = NULL;

				int SaveRet = 0;

				VARIANT SaveAsFileName;

				do 
				{
					pExcelAppObj = (Excel::_Application *)m_pAppObj;

					hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

					if (!SUCCEEDED(hr) || pWb == NULL)
					{
						break;
					}

					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					if (!DocFullName)
					{
						break;
					}

					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
					{
						hr = pExcelAppObj->get_FileDialog(Office::msoFileDialogSaveAs, &pSaveDialog);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						pSaveDialog->put_AllowMultiSelect(VARIANT_FALSE);
						pSaveDialog->put_InitialFileName(DocFullName);

						hr = pSaveDialog->Show(&SaveRet);

						if (!SaveRet)
						{
							break;
						}

						hr = pSaveDialog->get_SelectedItems(&pSelectedFiles);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pSelectedFiles->Item(1, &DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						SaveAsFileName.vt		= VT_BSTR;
						SaveAsFileName.bstrVal	= DocFullName;

						hr = pWb->SaveAs(SaveAsFileName, vtMissing, vtMissing, vtMissing, vtMissing, vtMissing, Excel::xlNoChange, vtMissing, vtMissing, vtMissing, vtMissing, vtMissing, 0x0409/*US English*/);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
						{
							break;
						}
					}

					SkipProcessing = FALSE;

				} while (FALSE);

				if (pSelectedFiles)
				{
					pSelectedFiles->Release();
					pSelectedFiles = NULL;
				}

				if (pSaveDialog)
				{
					pSaveDialog->Release();
					pSaveDialog = NULL;
				}

				if (pWb)
				{
					pWb->Release();
					pWb = NULL;
				}
			}

			break;

		default:
			break;
		}

		if (SkipProcessing)
		{
			break;
		}

		if (!get_classify_ui(ClassifyUIXMLFileName, GroupName))
		{
			break;
		}

		hFile = CreateFileW(DocFullName,
							GENERIC_READ|FILE_WRITE_EA,
							FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			break;
		}

		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (SUCCEEDED(hr))
		{
			pTagData = (UCHAR*)malloc(64*1024);

			if (!pTagData)
			{
				break;
			}

			memset(pTagData, 0, 64*1024);

			TagDataLength = (USHORT)(64*1024 - 1);

			hr = nudf::util::nxl::NxrmReadTagsEx(hFile, pTagData, &TagDataLength);

			if (hr != S_OK)
			{
				break;
			}

			BufferToPair((LPCWSTR)pTagData, tag);
		}
		else
		{
			IsEncryptedFile = FALSE;
		}

		hr = m_pRmUIObj->RmuShowClassifyDialogEx(hWnd, 
												 DocFullName, 
												 ClassifyUIXMLFileName,
												 GroupName,
												 GetUserDefaultLangID(), 
												 (RightsMask & BUILTIN_RIGHT_CLASSIFY ? FALSE: IsEncryptedFile ? TRUE : FALSE), 
												 tag);

		if (hr != S_OK)
		{
			break;
		}

		PairToBuffer(tag, wzTag);

		if (IsEncryptedFile)
		{
			hr = nudf::util::nxl::NxrmSyncNXLHeader(hFile, NXL_SECTION_TAGS, (UCHAR*)(&wzTag[0]), (USHORT)(wzTag.size() * sizeof(WCHAR)));
		}
		else
		{
			hr = nudf::util::nxl::NxrmEncryptFileEx2(hFile, NXL_SECTION_TAGS, (UCHAR*)(&wzTag[0]), (USHORT)(wzTag.size() * sizeof(WCHAR)));

			if (SUCCEEDED(hr))
			{
				Office::IRibbonUI *pRibbonUI = NULL;

				do 
				{
					hr = m_pRibbonUI->QueryInterface(__uuidof(Office::IRibbonUI), (void**)&pRibbonUI);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					pRibbonUI->InvalidateControl(NXRMCOREUI_CHECKRIGHTS_BUTTON_ID);
					pRibbonUI->InvalidateControl(NXRMCOREUI_PROTECT_BUTTON_ID);

					RefreshMsoControls();

				} while (FALSE);

				if (pRibbonUI)
				{
					pRibbonUI->Release();
					pRibbonUI = NULL;
				}
			}
		}

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (pTagData)
	{
		free(pTagData);
	}

	if (wcslen(ClassifyUIXMLFileName))
	{
		DeleteFileW(ClassifyUIXMLFileName);
	}

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnProtectUI2010(
	/*[in]*/ Office2010::IRibbonControl *pControl, 
	/*[in]*/ VARIANT_BOOL *pvarfPressed)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	WCHAR ClassifyUIXMLFileName[MAX_PATH] = {0};
	WCHAR GroupName[MAX_PATH] = {0};

	std::vector<std::pair<std::wstring,std::wstring>> tag;

	std::vector<WCHAR> wzTag;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	UCHAR *pTagData = NULL;

	USHORT TagDataLength = 0;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRightsMask = 0;
	ULONGLONG EvaluationId = 0;

	BOOL IsEncryptedFile = TRUE;
	
	BOOL SkipProcessing = TRUE;

	HWND hWnd = GetActiveWindow();
	
	do 
	{

		switch (m_OfficeAppType)
		{
		case OfficeAppWinword:
			{
				Word2010::_Application *pWinwordAppObj = NULL;
				Word2010::_Document *pDoc = NULL;

				do 
				{
					pWinwordAppObj = (Word2010::_Application *)m_pAppObj;

					hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					hr = pDoc->get_FullName(&DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					if (!DocFullName)
					{
						break;
					}

					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
					{
						hr = pDoc->Save();

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pDoc->get_FullName(&DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
						{
							break;
						}
					}

					SkipProcessing = FALSE;

				} while (FALSE);

				if (pDoc)
				{
					pDoc->Release();
					pDoc = NULL;
				}
			}

			break;

		case OfficeAppPowerpoint:

			{
				PowerPoint2010::_Application *pPowerPointAppObj = NULL;
				PowerPoint2010::_Presentation *pPres = NULL;
				Office2010::MsoTriState	PreSaved = Office2010::msoFalse;
				Office2010::FileDialog	*pSaveDialog = NULL;
				Office2010::FileDialogSelectedItems	*pSelectedFiles = NULL;

				int SaveRet = 0;

				do 
				{
					pPowerPointAppObj = (PowerPoint2010::_Application *)m_pAppObj;

					hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					hr = pPres->get_FullName(&DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					if (!DocFullName)
					{
						break;
					}

					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
					{
						hr = pPowerPointAppObj->get_FileDialog(Office2010::msoFileDialogSaveAs, &pSaveDialog);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						pSaveDialog->put_AllowMultiSelect(VARIANT_FALSE);
						pSaveDialog->put_InitialFileName(DocFullName);
						
						hr = pSaveDialog->Show(&SaveRet);

						if (!SaveRet)
						{
							break;
						}

						hr = pSaveDialog->get_SelectedItems(&pSelectedFiles);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pSelectedFiles->Item(1, &DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pPres->SaveAs(DocFullName, PowerPoint2010::ppSaveAsDefault, Office2010::msoTriStateMixed);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pPres->get_FullName(&DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
						{
							break;
						}
					}

					SkipProcessing = FALSE;

				} while (FALSE);
				
				if (pSelectedFiles)
				{
					pSelectedFiles->Release();
					pSelectedFiles = NULL;
				}

				if (pSaveDialog)
				{
					pSaveDialog->Release();
					pSaveDialog = NULL;
				}

				if (pPres)
				{
					pPres->Release();
					pPres = NULL;
				}
			}

			break;

		case OfficeAppExcel:

			{
				Excel2010::_Application *pExcelAppObj = NULL;
				Excel2010::_Workbook	*pWb = NULL;
				Office2010::MsoTriState	PreSaved = Office2010::msoFalse;
				Office2010::FileDialog	*pSaveDialog = NULL;
				Office2010::FileDialogSelectedItems	*pSelectedFiles = NULL;

				int SaveRet = 0;

				VARIANT SaveAsFileName;

				do 
				{
					pExcelAppObj = (Excel2010::_Application *)m_pAppObj;

					hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

					if (!SUCCEEDED(hr) || pWb == NULL)
					{
						break;
					}

					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					if (!DocFullName)
					{
						break;
					}

					if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
					{
						hr = pExcelAppObj->get_FileDialog(Office2010::msoFileDialogSaveAs, &pSaveDialog);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						pSaveDialog->put_AllowMultiSelect(VARIANT_FALSE);
						pSaveDialog->put_InitialFileName(DocFullName);

						hr = pSaveDialog->Show(&SaveRet);

						if (!SaveRet)
						{
							break;
						}

						hr = pSaveDialog->get_SelectedItems(&pSelectedFiles);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pSelectedFiles->Item(1, &DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						SaveAsFileName.vt		= VT_BSTR;
						SaveAsFileName.bstrVal	= DocFullName;

						hr = pWb->SaveAs(SaveAsFileName, vtMissing, vtMissing, vtMissing, vtMissing, vtMissing, Excel2010::xlNoChange, vtMissing, vtMissing,vtMissing,vtMissing,vtMissing,0x0409/*US English*/);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

						if (!SUCCEEDED(hr))
						{
							break;
						}

						if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(DocFullName))
						{
							break;
						}
					}

					SkipProcessing = FALSE;

				} while (FALSE);

				if (pSelectedFiles)
				{
					pSelectedFiles->Release();
					pSelectedFiles = NULL;
				}

				if (pSaveDialog)
				{
					pSaveDialog->Release();
					pSaveDialog = NULL;
				}

				if (pWb)
				{
					pWb->Release();
					pWb = NULL;
				}
			}

			break;

		default:
			break;
		}

		if (SkipProcessing)
		{
			break;
		}

		if (!get_classify_ui(ClassifyUIXMLFileName, GroupName))
		{
			break;
		}

		hFile = CreateFileW(DocFullName,
							GENERIC_READ|FILE_WRITE_EA,
							FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			break;
		}

		hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRightsMask, &EvaluationId);

		if (SUCCEEDED(hr))
		{
			pTagData = (UCHAR*)malloc(64*1024);

			if (!pTagData)
			{
				break;
			}

			memset(pTagData, 0, 64*1024);

			TagDataLength = (USHORT)(64*1024 - 1);

			hr = nudf::util::nxl::NxrmReadTagsEx(hFile, pTagData, &TagDataLength);

			if (hr != S_OK)
			{
				break;
			}

			BufferToPair((LPCWSTR)pTagData, tag);
		}
		else
		{
			IsEncryptedFile = FALSE;
		}

		hr = m_pRmUIObj->RmuShowClassifyDialogEx(hWnd, 
												 DocFullName, 
												 ClassifyUIXMLFileName,
												 GroupName,
												 GetUserDefaultLangID(), 
												 (RightsMask & BUILTIN_RIGHT_CLASSIFY ? FALSE: IsEncryptedFile ? TRUE : FALSE), 
												 tag);

		if (hr != S_OK)
		{
			break;
		}

		PairToBuffer(tag, wzTag);

		if (IsEncryptedFile)
		{
			hr = nudf::util::nxl::NxrmSyncNXLHeader(hFile, NXL_SECTION_TAGS, (UCHAR*)(&wzTag[0]), (USHORT)(wzTag.size() * sizeof(WCHAR)));
		}
		else
		{
			hr = nudf::util::nxl::NxrmEncryptFileEx2(hFile, NXL_SECTION_TAGS, (UCHAR*)(&wzTag[0]), (USHORT)(wzTag.size() * sizeof(WCHAR)));

			if (SUCCEEDED(hr))
			{
				Office2010::IRibbonUI *pRibbonUI = NULL;

				do 
				{
					hr = m_pRibbonUI->QueryInterface(__uuidof(Office2010::IRibbonUI), (void**)&pRibbonUI);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					pRibbonUI->InvalidateControl(NXRMCOREUI_CHECKRIGHTS_BUTTON_ID);
					pRibbonUI->InvalidateControl(NXRMCOREUI_PROTECT_BUTTON_ID);

					RefreshMsoControls();

				} while (FALSE);

				if (pRibbonUI)
				{
					pRibbonUI->Release();
					pRibbonUI = NULL;
				}
			}
		}

	} while (FALSE);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	if (pTagData)
	{
		free(pTagData);
	}

	if (wcslen(ClassifyUIXMLFileName))
	{
		DeleteFileW(ClassifyUIXMLFileName);
	}

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnCheckButtonStatus2013(
	/*[in]*/ Office::IRibbonControl *pControl, 
	/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;
	BSTR ButtonId = NULL;

	BOOL bIsNXLFile = FALSE;

	BOOL bIsProtectedMode = FALSE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	WCHAR tmpFileName[MAX_PATH] = {0};

	DWORD dwRet = 0;

	WCHAR c = 0;
	WCHAR d = 0;
	WCHAR e = 0;

	BOOL FileExistOnLocalDisk = FALSE;
	BOOL FileDoesNotExist = FALSE;
	BOOL EnableProtectButton = FALSE;

	switch (m_OfficeAppType)
	{
	case OfficeAppWinword:
		{
			Word::_Application *pWinwordAppObj = NULL;
			Word::_Document *pDoc = NULL;

			do 
			{
				pWinwordAppObj = (Word::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}

		break;

	case OfficeAppPowerpoint:

		{
			PowerPoint::_Application *pPowerPointAppObj = NULL;
			PowerPoint::_Presentation *pPres = NULL;
			PowerPoint::ProtectedViewWindow *pProtectedWn = NULL;

			do 
			{
				pPowerPointAppObj = (PowerPoint::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

				if (!SUCCEEDED(hr))
				{
					pPowerPointAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);
					
					if (pProtectedWn)
					{
						bIsProtectedMode = TRUE;
					}

					break;
				}

				hr = pPres->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}

			if (pProtectedWn)
			{
				pProtectedWn->Release();
				pProtectedWn = NULL;
			}
		}

		break;

	case OfficeAppExcel:

		{
			Excel::_Application *pExcelAppObj = NULL;
			Excel::_Workbook	*pWb = NULL;
			Excel::ProtectedViewWindow *pProtectedWn = NULL;

			do 
			{
				pExcelAppObj = (Excel::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (pWb)
				{
					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					hr = pExcelAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					bIsProtectedMode = TRUE;

					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}

			if (pProtectedWn)
			{
				pProtectedWn->Release();
				pProtectedWn = NULL;
			}
		}

		break;

	default:
		break;
	}

	do 
	{
		if (bIsProtectedMode)
		{
			*pvarfEnabled = FALSE;
			break;
		}

		do 
		{
			if (!DocFullName)
			{
				FileDoesNotExist = TRUE;
				break;
			}

			if (wcslen(DocFullName) == 0)
			{
				FileDoesNotExist = TRUE;
				break;
			}

			hFile = CreateFileW(DocFullName,
								GENERIC_READ,
								FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

			if (hFile == INVALID_HANDLE_VALUE)
			{
				_wcslwr_s(DocFullName, wcslen(DocFullName) + 1);

				if (wcsstr(DocFullName, L"http") == DocFullName)	// covers both http and https
				{
					break;
				}
				else if (wcsstr(DocFullName, L"ftp") == DocFullName)
				{
					break;
				}
				else
				{
					FileDoesNotExist = TRUE;
					break;
				}
			}

			dwRet = GetFinalPathNameByHandleW(hFile, tmpFileName, sizeof(tmpFileName)/sizeof(WCHAR) - 1, VOLUME_NAME_DOS);

			if (dwRet >= sizeof(tmpFileName)/sizeof(WCHAR) || dwRet < 7) // L"\\?\c:\"
			{
				break;
			}

			c = tmpFileName[4];
			d = tmpFileName[5];
			e = tmpFileName[6];

			if (d != L':' || e != L'\\')
			{
				break;
			}

			if ((!(c >= L'A' && c <= L'Z')) && 
				(!(c >='a' && c <='z')))
			{
				break;
			}

			tmpFileName[7] = L'\0';

			if (DRIVE_FIXED != GetDriveTypeW(tmpFileName))
			{
				break;
			}

			FileExistOnLocalDisk = TRUE;

		} while (FALSE);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		hr = pControl->get_Id(&ButtonId);

		if (SUCCEEDED(hr))
		{
			if (wcscmp(ButtonId, NXRMCOREUI_CHECKRIGHTS_BUTTON_ID) == 0)
			{
				if (bIsNXLFile && FileExistOnLocalDisk)
				{
					*pvarfEnabled = VARIANT_TRUE;
				}
				else
				{
					*pvarfEnabled = VARIANT_FALSE;
				}
			}

			if (wcscmp(ButtonId, NXRMCOREUI_PROTECT_BUTTON_ID) == 0)
			{
				EnableProtectButton = check_enable_protect_menu();

				if ((FileExistOnLocalDisk || FileDoesNotExist) && EnableProtectButton)
					*pvarfEnabled = VARIANT_TRUE;
				else
					*pvarfEnabled = VARIANT_FALSE;
			}
		}

	} while (FALSE);

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	if (ButtonId)
	{
		SysFreeString(ButtonId);
		ButtonId = NULL;
	}

	return hr; 

}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnCheckButtonStatus2010(
	/*[in]*/ Office2010::IRibbonControl *pControl, 
	/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;
	BSTR ButtonId = NULL;

	BOOL bIsNXLFile = FALSE;

	BOOL bIsProtectedMode = FALSE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	WCHAR tmpFileName[MAX_PATH] = {0};

	DWORD dwRet = 0;

	WCHAR c = 0;
	WCHAR d = 0;
	WCHAR e = 0;

	BOOL FileExistOnLocalDisk = FALSE;
	BOOL FileDoesNotExist = FALSE;
	BOOL EnableProtectButton = FALSE;

	switch (m_OfficeAppType)
	{
	case OfficeAppWinword:
		{
			Word2010::_Application *pWinwordAppObj = NULL;
			Word2010::_Document *pDoc = NULL;

			do 
			{
				pWinwordAppObj = (Word2010::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}

		break;

	case OfficeAppPowerpoint:

		{
			PowerPoint2010::_Application *pPowerPointAppObj = NULL;
			PowerPoint2010::_Presentation *pPres = NULL;
			PowerPoint2010::ProtectedViewWindow *pProtectedWn = NULL;

			do 
			{
				pPowerPointAppObj = (PowerPoint2010::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

				if (!SUCCEEDED(hr))
				{
					pPowerPointAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);

					if (pProtectedWn)
					{
						bIsProtectedMode = TRUE;
					}

					break;
				}

				hr = pPres->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}

			if (pProtectedWn)
			{
				pProtectedWn->Release();
				pProtectedWn = NULL;
			}
		}

		break;

	case OfficeAppExcel:

		{
			Excel2010::_Application *pExcelAppObj = NULL;
			Excel2010::_Workbook	*pWb = NULL;
			Excel2010::ProtectedViewWindow *pProtectedWn = NULL;

			do 
			{
				pExcelAppObj = (Excel2010::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (pWb)
				{
					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					hr = pExcelAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					bIsProtectedMode = TRUE;

					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}
		}

		break;

	default:
		break;
	}

	do 
	{
		if (bIsProtectedMode)
		{
			*pvarfEnabled = FALSE;
			break;
		}

		do 
		{
			if (!DocFullName)
			{
				FileDoesNotExist = TRUE;
				break;
			}

			if (wcslen(DocFullName) == 0)
			{
				FileDoesNotExist = TRUE;
				break;
			}

			hFile = CreateFileW(DocFullName,
								GENERIC_READ,
								FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);

			if (hFile == INVALID_HANDLE_VALUE)
			{
				_wcslwr_s(DocFullName, wcslen(DocFullName) + 1);

				if (wcsstr(DocFullName, L"http") == DocFullName)	// covers both http and https
				{
					break;
				}
				else if (wcsstr(DocFullName, L"ftp") == DocFullName)
				{
					break;
				}
				else
				{
					FileDoesNotExist = TRUE;
					break;
				}
			}

			dwRet = GetFinalPathNameByHandleW(hFile, tmpFileName, sizeof(tmpFileName)/sizeof(WCHAR) - 1, VOLUME_NAME_DOS);

			if (dwRet >= sizeof(tmpFileName)/sizeof(WCHAR) || dwRet < 7) // L"\\?\c:\"
			{
				break;
			}

			c = tmpFileName[4];
			d = tmpFileName[5];
			e = tmpFileName[6];

			if (d != L':' || e != L'\\')
			{
				break;
			}

			if ((!(c >= L'A' && c <= L'Z')) && 
				(!(c >='a' && c <='z')))
			{
				break;
			}

			tmpFileName[7] = L'\0';

			if (DRIVE_FIXED != GetDriveTypeW(tmpFileName))
			{
				break;
			}

			FileExistOnLocalDisk = TRUE;

		} while (FALSE);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
		}

		hr = pControl->get_Id(&ButtonId);

		if (SUCCEEDED(hr))
		{
			if (wcscmp(ButtonId, NXRMCOREUI_CHECKRIGHTS_BUTTON_ID) == 0)
			{
				if (bIsNXLFile && FileExistOnLocalDisk)
				{
					*pvarfEnabled = VARIANT_TRUE;
				}
				else
				{
					*pvarfEnabled = VARIANT_FALSE;
				}
			}

			if (wcscmp(ButtonId, NXRMCOREUI_PROTECT_BUTTON_ID) == 0)
			{
				EnableProtectButton = check_enable_protect_menu();

				if ((FileExistOnLocalDisk || FileDoesNotExist) && EnableProtectButton)
					*pvarfEnabled = VARIANT_TRUE;
				else
					*pvarfEnabled = VARIANT_FALSE;
			}
		}

	} while (FALSE);

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	if (ButtonId)
	{
		SysFreeString(ButtonId);
		ButtonId = NULL;
	}

	return hr; 
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetProtectButtonLable2013(
	/*[in] */ Office::IRibbonControl *pControl,
	/*[out,retval]*/ BSTR * pbstrLabel)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	BOOL bIsNXLFile = FALSE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	switch (m_OfficeAppType)
	{
	case OfficeAppWinword:
		{
			Word::_Application *pWinwordAppObj = NULL;
			Word::_Document *pDoc = NULL;

			do 
			{
				pWinwordAppObj = (Word::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}

		break;

	case OfficeAppPowerpoint:

		{
			PowerPoint::_Application *pPowerPointAppObj = NULL;
			PowerPoint::_Presentation *pPres = NULL;

			do 
			{
				pPowerPointAppObj = (PowerPoint::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);
				
				if (pPres)
				{
					hr = pPres->get_FullName(&DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					powerpoint2013_get_fullname_in_protected_mode(pPowerPointAppObj, &DocFullName);
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}
		}

		break;

	case OfficeAppExcel:

		{
			Excel::_Application *pExcelAppObj = NULL;
			Excel::_Workbook	*pWb = NULL;

			do 
			{
				pExcelAppObj = (Excel::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (pWb)
				{
					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					excel2013_get_fullname_in_protected_mode(pExcelAppObj, &DocFullName);
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}
		}

		break;

	default:
		break;
	}

	if (bIsNXLFile)
	{
		*pbstrLabel = SysAllocString(NXRMCOREUI_CLASSIFY_LABLE);
	}
	else
	{
		*pbstrLabel = SysAllocString(NXRMCOREUI_PROTECT_LABLE);
	}

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return S_OK; 
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::GetProtectButtonLable2010(
	/*[in] */ Office2010::IRibbonControl *pControl,
	/*[out,retval]*/ BSTR * pbstrLabel)
{
	HRESULT hr = S_OK;

	BSTR DocFullName = NULL;

	BOOL bIsNXLFile = FALSE;

	ULONGLONG RightsMask = 0;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	switch (m_OfficeAppType)
	{
	case OfficeAppWinword:
		{
			Word2010::_Application *pWinwordAppObj = NULL;
			Word2010::_Document *pDoc = NULL;

			do 
			{
				pWinwordAppObj = (Word2010::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}

		break;

	case OfficeAppPowerpoint:

		{
			PowerPoint2010::_Application *pPowerPointAppObj = NULL;
			PowerPoint2010::_Presentation *pPres = NULL;

			do 
			{
				pPowerPointAppObj = (PowerPoint2010::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

				if (pPres)
				{
					hr = pPres->get_FullName(&DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					powerpoint2010_get_fullname_in_protected_mode(pPowerPointAppObj, &DocFullName);
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}
		}

		break;

	case OfficeAppExcel:

		{
			Excel2010::_Application *pExcelAppObj = NULL;
			Excel2010::_Workbook	*pWb = NULL;

			do 
			{
				pExcelAppObj = (Excel2010::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (pWb)
				{
					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					excel2010_get_fullname_in_protected_mode(pExcelAppObj, &DocFullName);
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (SUCCEEDED(hr))
				{
					bIsNXLFile = TRUE;
				}
				else
				{
					bIsNXLFile = FALSE;
				}

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}
		}

		break;

	default:
		break;
	}

	if (bIsNXLFile)
	{
		*pbstrLabel = SysAllocString(NXRMCOREUI_CLASSIFY_LABLE);
	}
	else
	{
		*pbstrLabel = SysAllocString(NXRMCOREUI_PROTECT_LABLE);
	}

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	return S_OK; 
}

static BSTR LoadCustomUIFromFile(WCHAR *FileName)
{
	BSTR CustomUIXML = NULL;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	BOOL bRet = TRUE;

	DWORD dwFileSize = 0;

	BYTE *buf = NULL;
	BYTE *p = NULL;
	DWORD bufLen = 0;

	DWORD BytesRead = 0;
	DWORD TotalBytesRead = 0;

	int BSTRLen = 0;

	do 
	{
		hFile = CreateFileW(FileName,
							GENERIC_READ | GENERIC_WRITE | DELETE,
							0,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE | FILE_FLAG_SEQUENTIAL_SCAN,
							NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			break;
		}

		dwFileSize = GetFileSize(hFile, NULL);

		if (!dwFileSize)
		{
			break;
		}

		//
		// CustomUI.xml should not bigger than 1Mb
		//
		if (dwFileSize >= 1024 * 1024)
		{
			break;
		}

		bufLen = (dwFileSize + sizeof('\0') + 4095) & (~4095);

		buf = (BYTE*)malloc(bufLen);

		if (!buf)
		{
			break;
		}

		memset(buf, 0, bufLen);

		p = buf;
		TotalBytesRead = 0;

		do
		{
			bRet = ReadFile(hFile,
							(p - TotalBytesRead),
							(bufLen - TotalBytesRead),
							&BytesRead,
							NULL);

			if (!bRet)
			{
				break;
			}

			if (BytesRead == 0)
			{
				//
				// End of file
				//
				break;
			}

			TotalBytesRead += BytesRead;

		} while (TRUE);
		
		if (TotalBytesRead != dwFileSize)
		{
			break;
		}

		BSTRLen = MultiByteToWideChar(CP_UTF8,
									  0,
									  (LPCSTR)buf,
									  TotalBytesRead,
									  NULL,
									  0);

		if (BSTRLen <= 0)
		{
			break;
		}

		CustomUIXML = SysAllocStringLen(NULL, BSTRLen);

		if (!CustomUIXML)
		{
			break;
		}

		BSTRLen = MultiByteToWideChar(CP_UTF8,
									  0,
									  (LPCSTR)buf,
									  TotalBytesRead,
									  CustomUIXML,
									  BSTRLen);

		if (BSTRLen <= 0)
		{
			SysFreeString(CustomUIXML);
			CustomUIXML = NULL;
		}

	} while (FALSE);

	if (buf)
	{
		free(buf);
		buf = NULL;
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}

	return CustomUIXML;
}

static BOOL get_classify_ui(WCHAR *FileName, WCHAR *GroupName)
{
	BOOL bRet = FALSE;

	GET_CLASSIFY_UI_REQUEST		Request = {0};
	GET_CLASSIFY_UI_RESPONSE	Resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG RespSize = sizeof(Resp);

	LANGID	LangId = {0};

	PVOID Section = NULL;

	do 
	{
		Section = init_transporter_client();

		if (!Section)
		{
			break;
		}

		if (!is_transporter_enabled(Section))
		{
			break;
		}

		LangId = GetUserDefaultLangID();

		Request.LanguageId	= LangId;
		Request.ProcessId	= GetCurrentProcessId();
		Request.ThreadId	= GetCurrentThreadId();

		if(!ProcessIdToSessionId(Request.ProcessId, &Request.SessionId))
		{
			break;
		}

		if (!GetTempPathW(sizeof(Request.TempPath)/sizeof(WCHAR), Request.TempPath))
		{
			break;
		}

		if (!GetLongPathNameW(Request.TempPath, Request.TempPath, sizeof(Request.TempPath)/sizeof(WCHAR)))
		{
			break;
		}

		Ctx = submit_request(Section, NXRMDRV_MSG_TYPE_GET_CLASSIFYUI, &Request, sizeof(Request));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Section, &Resp, sizeof(Resp), &RespSize))
		{
			Ctx = NULL;
			break;
		}

		memcpy(FileName, 
			   Resp.ClassifyUIFileName, 
			   min(wcslen(Resp.ClassifyUIFileName)*sizeof(WCHAR), MAX_PATH - sizeof(WCHAR)));

		memcpy(GroupName,
			   Resp.GroupName,
			   min(wcslen(Resp.GroupName)*sizeof(WCHAR), MAX_PATH - sizeof(WCHAR)));

		bRet = TRUE;

	} while (FALSE);

	if (Section)
	{
		close_transporter_client(Section);
		Section = NULL;
	}

	return bRet;
}

static void PairToBuffer(_In_ const std::vector<std::pair<std::wstring,std::wstring>>& pairs, _Out_ std::vector<WCHAR>& buf)
{
	buf.clear();
	for(int i=0; i<(int)pairs.size(); i++) {
		if(pairs[i].first.empty() || pairs[i].second.empty()) {
			continue;
		}
		std::wstring ws(pairs[i].first);
		ws += L"=";
		ws += pairs[i].second;
		for(int j=0; j<(int)ws.length(); j++) {
			buf.push_back(ws.c_str()[j]);
		}
		buf.push_back(L'\0');
	}
	if(!buf.empty()) {
		buf.push_back(L'\0');
	}
}

static void BufferToPair(_In_ LPCWSTR buf, _Out_ std::vector<std::pair<std::wstring,std::wstring>>& pairs)
{
	pairs.clear();
	while(buf[0] != L'\0') {
		std::wstring wsPair(buf);
		buf += (wsPair.length() + 1);
		std::wstring name;
		std::wstring value;
		std::wstring::size_type pos = wsPair.find_first_of(L'=');
		if(pos == std::wstring::npos) {
			continue;
		}
		name = wsPair.substr(0, pos);
		value = wsPair.substr(pos+1);
		pairs.push_back(std::pair<std::wstring,std::wstring>(name,value));
	}
}

static BOOL check_enable_protect_menu(void)
{
	BOOL bRet = TRUE;

	CHECK_PROTECT_MENU_REQUEST	Request = {0};
	CHECK_PROTECT_MENU_RESPONSE	Resp = {0};

	NXCONTEXT Ctx = NULL;

	ULONG RespSize = sizeof(Resp);

	PVOID Section = NULL;

	do 
	{
		Section = init_transporter_client();

		if (!Section)
		{
			break;
		}

		if (!is_transporter_enabled(Section))
		{
			break;
		}

		Request.ProcessId	= GetCurrentProcessId();
		Request.ThreadId	= GetCurrentThreadId();

		if(!ProcessIdToSessionId(Request.ProcessId, &Request.SessionId))
		{
			break;
		}

		Ctx = submit_request(Section, NXRMDRV_MSG_TYPE_CHECK_PROTECT_MENU, &Request, sizeof(Request));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Section, &Resp, sizeof(Resp), &RespSize))
		{
			Ctx = NULL;
			break;
		}

		if (Resp.EnableProtectMenu)
		{
			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}

	} while (FALSE);

	if (Section)
	{
		close_transporter_client(Section);
		Section = NULL;
	}

	return bRet;
}

static BOOL excel2013_get_fullname_in_protected_mode(Excel::_Application *pApp, BSTR *DocFullName)
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;

	Excel::ProtectedViewWindow *pProtectedWn = NULL;
	Excel::IProtectedViewWindow *pIWn = NULL;

	BSTR filepath = NULL;
	BSTR filename = NULL;

	ULONG filepath_length = 0;
	ULONG filename_length = 0;
	ULONG totoal_length = 0;

	WCHAR *fullname = NULL;

	do 
	{
		hr = pApp->get_ActiveProtectedViewWindow(&pProtectedWn);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pProtectedWn)
		{
			break;
		}

		pIWn = (Excel::IProtectedViewWindow*)pProtectedWn;

		hr = pIWn->get_SourcePath(&filepath);

		if (!SUCCEEDED(hr) || filepath == NULL)
		{
			break;
		}

		hr = pIWn->get_SourceName(&filename);

		if (!SUCCEEDED(hr) || filename == NULL)
		{
			break;
		}

		filepath_length = (ULONG)(wcslen(filepath) * sizeof(WCHAR));

		filename_length = (ULONG)(wcslen(filename) * sizeof(WCHAR));

		totoal_length = filepath_length + filename_length + sizeof(WCHAR) + sizeof(WCHAR);	// one for L'\\' and one for L'\0'

		fullname = (WCHAR*)malloc(totoal_length);

		if (!fullname)
		{
			break;
		}

		swprintf_s(fullname,
				   totoal_length/sizeof(WCHAR),
				   L"%s\\%s",
				   filepath,
				   filename);

		*DocFullName = SysAllocString(fullname);

		if (*DocFullName == NULL)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	if (fullname)
	{
		free(fullname);
		fullname = NULL;
	}

	if (filepath)
	{
		SysFreeString(filepath);
		filepath = NULL;
	}

	if (filename)
	{
		SysFreeString(filename);
		filename = NULL;
	}

	if (pProtectedWn)
	{
		pProtectedWn->Release();
		pProtectedWn = NULL;
	}

	return bRet;
}

static BOOL excel2010_get_fullname_in_protected_mode(Excel2010::_Application *pApp, BSTR *DocFullName)
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;

	Excel2010::ProtectedViewWindow *pProtectedWn = NULL;
	Excel2010::IProtectedViewWindow *pIWn = NULL;

	BSTR filepath = NULL;
	BSTR filename = NULL;

	ULONG filepath_length = 0;
	ULONG filename_length = 0;
	ULONG totoal_length = 0;

	WCHAR *fullname = NULL;

	do 
	{
		hr = pApp->get_ActiveProtectedViewWindow(&pProtectedWn);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pProtectedWn)
		{
			break;
		}

		pIWn = (Excel2010::IProtectedViewWindow*)pProtectedWn;

		hr = pIWn->get_SourcePath(&filepath);

		if (!SUCCEEDED(hr) || filepath == NULL)
		{
			break;
		}

		hr = pIWn->get_SourceName(&filename);

		if (!SUCCEEDED(hr) || filename == NULL)
		{
			break;
		}

		filepath_length = (ULONG)(wcslen(filepath) * sizeof(WCHAR));

		filename_length = (ULONG)(wcslen(filename) * sizeof(WCHAR));

		totoal_length = filepath_length + filename_length + sizeof(WCHAR) + sizeof(WCHAR);	// one for L'\\' and one for L'\0'

		fullname = (WCHAR*)malloc(totoal_length);

		if (!fullname)
		{
			break;
		}

		swprintf_s(fullname,
				   totoal_length/sizeof(WCHAR),
				   L"%s\\%s",
				   filepath,
				   filename);

		*DocFullName = SysAllocString(fullname);

		if (*DocFullName == NULL)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	if (fullname)
	{
		free(fullname);
		fullname = NULL;
	}

	if (filepath)
	{
		SysFreeString(filepath);
		filepath = NULL;
	}

	if (filename)
	{
		SysFreeString(filename);
		filename = NULL;
	}

	if (pProtectedWn)
	{
		pProtectedWn->Release();
		pProtectedWn = NULL;
	}

	return bRet;
}

static BOOL powerpoint2013_get_fullname_in_protected_mode(PowerPoint::_Application *pApp, BSTR *DocFullName)
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;

	PowerPoint::ProtectedViewWindow *pProtectedWn = NULL;

	BSTR filepath = NULL;
	BSTR filename = NULL;

	ULONG filepath_length = 0;
	ULONG filename_length = 0;
	ULONG totoal_length = 0;

	WCHAR *fullname = NULL;

	do 
	{
		hr = pApp->get_ActiveProtectedViewWindow(&pProtectedWn);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pProtectedWn)
		{
			break;
		}

		hr = pProtectedWn->get_SourcePath(&filepath);

		if (!SUCCEEDED(hr) || filepath == NULL)
		{
			break;
		}

		hr = pProtectedWn->get_SourceName(&filename);

		if (!SUCCEEDED(hr) || filename == NULL)
		{
			break;
		}

		filepath_length = (ULONG)(wcslen(filepath) * sizeof(WCHAR));

		filename_length = (ULONG)(wcslen(filename) * sizeof(WCHAR));

		totoal_length = filepath_length + filename_length + sizeof(WCHAR) + sizeof(WCHAR);	// one for L'\\' and one for L'\0'

		fullname = (WCHAR*)malloc(totoal_length);

		if (!fullname)
		{
			break;
		}

		swprintf_s(fullname,
				   totoal_length/sizeof(WCHAR),
				   L"%s\\%s",
				   filepath,
				   filename);

		*DocFullName = SysAllocString(fullname);

		if (*DocFullName == NULL)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	if (fullname)
	{
		free(fullname);
		fullname = NULL;
	}

	if (filepath)
	{
		SysFreeString(filepath);
		filepath = NULL;
	}

	if (filename)
	{
		SysFreeString(filename);
		filename = NULL;
	}

	if (pProtectedWn)
	{
		pProtectedWn->Release();
		pProtectedWn = NULL;
	}

	return bRet;
}

static BOOL powerpoint2010_get_fullname_in_protected_mode(PowerPoint2010::_Application *pApp, BSTR *DocFullName)
{
	BOOL bRet = FALSE;

	HRESULT hr = S_OK;

	PowerPoint2010::ProtectedViewWindow *pProtectedWn = NULL;

	BSTR filepath = NULL;
	BSTR filename = NULL;

	ULONG filepath_length = 0;
	ULONG filename_length = 0;
	ULONG totoal_length = 0;

	WCHAR *fullname = NULL;

	do 
	{
		hr = pApp->get_ActiveProtectedViewWindow(&pProtectedWn);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (!pProtectedWn)
		{
			break;
		}

		hr = pProtectedWn->get_SourcePath(&filepath);

		if (!SUCCEEDED(hr) || filepath == NULL)
		{
			break;
		}

		hr = pProtectedWn->get_SourceName(&filename);

		if (!SUCCEEDED(hr) || filename == NULL)
		{
			break;
		}

		filepath_length = (ULONG)(wcslen(filepath) * sizeof(WCHAR));

		filename_length = (ULONG)(wcslen(filename) * sizeof(WCHAR));

		totoal_length = filepath_length + filename_length + sizeof(WCHAR) + sizeof(WCHAR);	// one for L'\\' and one for L'\0'

		fullname = (WCHAR*)malloc(totoal_length);

		if (!fullname)
		{
			break;
		}

		swprintf_s(fullname,
				   totoal_length/sizeof(WCHAR),
				   L"%s\\%s",
				   filepath,
				   filename);

		*DocFullName = SysAllocString(fullname);

		if (*DocFullName == NULL)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	if (fullname)
	{
		free(fullname);
		fullname = NULL;
	}

	if (filepath)
	{
		SysFreeString(filepath);
		filepath = NULL;
	}

	if (filename)
	{
		SysFreeString(filename);
		filename = NULL;
	}

	if (pProtectedWn)
	{
		pProtectedWn->Release();
		pProtectedWn = NULL;
	}

	return bRet;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnCheckMsoButtonStatus2013(
	/*[in]*/ Office::IRibbonControl *pControl,
	/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled)
{
	HRESULT hr = S_OK;

	VARIANT_BOOL bEnable = VARIANT_TRUE;

	ULONGLONG	ActiveRights = BUILTIN_RIGHT_ALL;

	BSTR	ribbon_id = NULL;
		
	do 
	{
		if (m_OfficeAppType == OfficeAppExcel && m_pOfficeEventSink)
		{
			ExcelEventListener *pExcelEventSink = (ExcelEventListener *)m_pOfficeEventSink;

			pExcelEventSink->GetActiveRights(ActiveRights);
		}
		else if (m_OfficeAppType == OfficeAppWinword && m_pOfficeEventSink)
		{
			WordEventListener *pWordEventSink = (WordEventListener*)m_pOfficeEventSink;

			pWordEventSink->GetActiveRights(ActiveRights);
		}
		else if (m_OfficeAppType == OfficeAppPowerpoint && m_pOfficeEventSink)
		{
			PowerPointEventListener *pPowerPointEventSink = (PowerPointEventListener*)m_pOfficeEventSink;

			pPowerPointEventSink->GetActiveRights(ActiveRights);
		}
		else
		{
			break;
		}

		pControl->get_Id(&ribbon_id);

		const auto &ite = m_RibbonRightsMap.find(ribbon_id);

		if (ite != m_RibbonRightsMap.end())
		{
			bEnable = ((*ite).second.RightsMask & ActiveRights) ? VARIANT_TRUE : VARIANT_FALSE;
		}

	} while (FALSE);

	*pvarfEnabled = bEnable;

	if (ribbon_id)
	{
		SysFreeString(ribbon_id);
		ribbon_id = NULL;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE nxrmCoreExt2::OnCheckMsoButtonStatus2010(
	/*[in]*/ Office2010::IRibbonControl *pControl,
	/*[out, retval]*/ VARIANT_BOOL *pvarfEnabled)
{
	HRESULT hr = S_OK;

	VARIANT_BOOL bEnable = VARIANT_TRUE;

	ULONGLONG	ActiveRights = BUILTIN_RIGHT_ALL;

	BSTR	ribbon_id = NULL;

	do
	{
		if (m_OfficeAppType == OfficeAppExcel && m_pOfficeEventSink)
		{
			ExcelEventListener *pExcelEventSink = (ExcelEventListener *)m_pOfficeEventSink;

			pExcelEventSink->GetActiveRights(ActiveRights);
		}
		else if (m_OfficeAppType == OfficeAppWinword && m_pOfficeEventSink)
		{
			WordEventListener *pWordEventSink = (WordEventListener*)m_pOfficeEventSink;

			pWordEventSink->GetActiveRights(ActiveRights);
		}
		else if (m_OfficeAppType == OfficeAppPowerpoint && m_pOfficeEventSink)
		{
			PowerPointEventListener *pPowerPointEventSink = (PowerPointEventListener*)m_pOfficeEventSink;

			pPowerPointEventSink->GetActiveRights(ActiveRights);
		}
		else
		{
			break;
		}

		pControl->get_Id(&ribbon_id);

		const auto &ite = m_RibbonRightsMap.find(ribbon_id);

		if (ite != m_RibbonRightsMap.end())
		{
			bEnable = ((*ite).second.RightsMask & ActiveRights) ? VARIANT_TRUE : VARIANT_FALSE;
		}

	} while (FALSE);

	*pvarfEnabled = bEnable;

	if (ribbon_id)
	{
		SysFreeString(ribbon_id);
		ribbon_id = NULL;
	}

	return hr;
}

HRESULT nxrmCoreExt2::Attache2Excel(void)
{
	HRESULT hr = S_OK;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;

	ExcelEventListener *pExcelEventSink = NULL;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	do 
	{
		if (m_OfficeAppType != OfficeAppExcel || m_pRibbonUI == NULL)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (g_bIsOffice2010)
		{
			Excel2010::_Application *pExcelAppObj = NULL;
			Excel2010::_Workbook	*pWb = NULL;
			Excel2010::ProtectedViewWindow *pProtectedWn = NULL;

			do
			{
				pExcelAppObj = (Excel2010::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (pWb)
				{
					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					hr = pExcelAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);

					if (!SUCCEEDED(hr))
					{
						break;
					}

					//
					// TO DO: Adding code to deal with protected view
					//
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}

			if (pProtectedWn)
			{
				pProtectedWn->Release();
				pProtectedWn = NULL;
			}
		}
		else
		{
			Excel::_Application *pExcelAppObj = NULL;
			Excel::_Workbook	*pWb = NULL;
			Excel::ProtectedViewWindow *pProtectedWn = NULL;

			do
			{
				pExcelAppObj = (Excel::_Application *)m_pAppObj;

				hr = pExcelAppObj->get_ActiveWorkbook(&pWb);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (pWb)
				{
					hr = pWb->get_FullName(0x0409/*US English*/, &DocFullName);

					if (!SUCCEEDED(hr))
					{
						break;
					}
				}
				else
				{
					hr = pExcelAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);

					if (!SUCCEEDED(hr))
					{
						break;
					}
					
					//
					// TO DO: Adding code to deal with protected view
					//
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

			} while (FALSE);

			if (pWb)
			{
				pWb->Release();
				pWb = NULL;
			}

			if (pProtectedWn)
			{
				pProtectedWn->Release();
				pProtectedWn = NULL;
			}
		}

		try
		{
			pExcelEventSink = new ExcelEventListener(m_pRibbonUI, DocFullName, RightsMask);
		}
		catch (std::bad_alloc exec)
		{
			pExcelEventSink = NULL;
		}

		if (!pExcelEventSink)
		{
			break;
		}

		hr = m_pAppObj->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (g_bIsOffice2010)
			hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Excel2010::AppEvents), &pConnectionPoint);
		else
			hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Excel::AppEvents), &pConnectionPoint);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise(pExcelEventSink, &dwAdviseCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		m_pOfficeEventSink = pExcelEventSink;

		pExcelEventSink = NULL;

	} while (FALSE);

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	if (pExcelEventSink)
	{
		delete pExcelEventSink;
		pExcelEventSink = NULL;
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

HRESULT nxrmCoreExt2::Attache2Word(void)
{
	HRESULT hr = S_OK;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;

	WordEventListener *pWordEventSink = NULL;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	do
	{
		if (m_OfficeAppType != OfficeAppWinword || m_pRibbonUI == NULL)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (g_bIsOffice2010)
		{
			Word2010::_Application *pWinwordAppObj = NULL;
			Word2010::_Document *pDoc = NULL;

			do
			{
				pWinwordAppObj = (Word2010::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}
		else
		{
			Word::_Application *pWinwordAppObj = NULL;
			Word::_Document *pDoc = NULL;

			do
			{
				pWinwordAppObj = (Word::_Application *)m_pAppObj;

				hr = pWinwordAppObj->get_ActiveDocument(&pDoc);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				hr = pDoc->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

			} while (FALSE);

			if (pDoc)
			{
				pDoc->Release();
				pDoc = NULL;
			}
		}

		try
		{
			pWordEventSink = new WordEventListener(m_pRibbonUI, DocFullName, RightsMask);
		}
		catch (std::bad_alloc exec)
		{
			pWordEventSink = NULL;
		}

		if (!pWordEventSink)
		{
			break;
		}

		hr = m_pAppObj->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(Word::ApplicationEvents4), &pConnectionPoint);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise(pWordEventSink, &dwAdviseCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		m_pOfficeEventSink = pWordEventSink;

		pWordEventSink = NULL;

	} while (FALSE);

	if (DocFullName)
	{
		SysFreeString(DocFullName);
		DocFullName = NULL;
	}

	if (pWordEventSink)
	{
		delete pWordEventSink;
		pWordEventSink = NULL;
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

HRESULT nxrmCoreExt2::Attache2Powerpoint(void)
{
	HRESULT hr = S_OK;

	IConnectionPointContainer	*pConnectionPointContainer = NULL;
	IConnectionPoint			*pConnectionPoint = NULL;
	DWORD                       dwAdviseCookie = 0;

	PowerPointEventListener *pPowerPointEventSink = NULL;

	BSTR DocFullName = NULL;

	ULONGLONG RightsMask = BUILTIN_RIGHT_ALL;
	ULONGLONG CustomRights = 0;
	ULONGLONG EvaluationId = 0;

	do
	{
		if (m_OfficeAppType != OfficeAppPowerpoint || m_pRibbonUI == NULL)
		{
			hr = E_UNEXPECTED;
			break;
		}

		if (g_bIsOffice2010)
		{
			PowerPoint2010::_Application *pPowerPointAppObj = NULL;
			PowerPoint2010::_Presentation *pPres = NULL;
			PowerPoint2010::ProtectedViewWindow *pProtectedWn = NULL;

			do
			{
				pPowerPointAppObj = (PowerPoint2010::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

				if (!SUCCEEDED(hr))
				{
					pPowerPointAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);

					if (pProtectedWn)
					{

					}

					break;
				}

				hr = pPres->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}

			if (pProtectedWn)
			{
				pProtectedWn->Release();
				pProtectedWn = NULL;
			}
		}
		else
		{
			PowerPoint::_Application *pPowerPointAppObj = NULL;
			PowerPoint::_Presentation *pPres = NULL;
			PowerPoint::ProtectedViewWindow *pProtectedWn = NULL;

			do
			{
				pPowerPointAppObj = (PowerPoint::_Application *)m_pAppObj;

				hr = pPowerPointAppObj->get_ActivePresentation(&pPres);

				if (!SUCCEEDED(hr))
				{
					pPowerPointAppObj->get_ActiveProtectedViewWindow(&pProtectedWn);

					if (pProtectedWn)
					{

					}

					break;
				}

				hr = pPres->get_FullName(&DocFullName);

				if (!SUCCEEDED(hr))
				{
					break;
				}

				if (!DocFullName)
				{
					break;
				}

				hr = nudf::util::nxl::NxrmCheckRights(DocFullName, &RightsMask, &CustomRights, &EvaluationId);

				if (!SUCCEEDED(hr))
				{
					break;
				}

			} while (FALSE);

			if (pPres)
			{
				pPres->Release();
				pPres = NULL;
			}

			if (pProtectedWn)
			{
				pProtectedWn->Release();
				pProtectedWn = NULL;
			}
		}

		try
		{
			pPowerPointEventSink = new PowerPointEventListener(m_pRibbonUI, DocFullName, RightsMask);
		}
		catch (std::bad_alloc exec)
		{
			pPowerPointEventSink = NULL;
		}

		if (!pPowerPointEventSink)
		{
			break;
		}

		hr = m_pAppObj->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPointContainer->FindConnectionPoint(__uuidof(PowerPoint::EApplication), &pConnectionPoint);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pConnectionPoint->Advise(pPowerPointEventSink, &dwAdviseCookie);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		m_pOfficeEventSink = pPowerPointEventSink;

		pPowerPointEventSink = NULL;

	} while (FALSE);

	if (pPowerPointEventSink)
	{
		delete pPowerPointEventSink;
		pPowerPointEventSink = NULL;
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

HRESULT nxrmCoreExt2::RefreshMsoControls(void)
{
	HRESULT hr = S_OK;

	do 
	{
		if (m_OfficeAppType == OfficeAppExcel && m_pOfficeEventSink)
		{
			ExcelEventListener *pExcelEventSink = (ExcelEventListener *)m_pOfficeEventSink;

			pExcelEventSink->RefreshActiveRights();
		}
		else if (m_OfficeAppType == OfficeAppWinword && m_pOfficeEventSink)
		{
			WordEventListener *pWordEventSink = (WordEventListener*)m_pOfficeEventSink;

			pWordEventSink->RefreshActiveRights();
		}
		else if (m_OfficeAppType == OfficeAppPowerpoint && m_pOfficeEventSink)
		{
			PowerPointEventListener *pPowerPointEventSink = (PowerPointEventListener*)m_pOfficeEventSink;

			pPowerPointEventSink->RefreshActiveRights();
		}
		else
		{
			break;
		}

	} while (FALSE);

	return hr;
}
