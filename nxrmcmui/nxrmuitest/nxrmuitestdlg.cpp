
// nxrmuitestdlg.cpp : implementation file
//

#include "stdafx.h"
#include "nxrmuitest.h"
#include "nxrmuitestdlg.h"
#include "afxdialogex.h"

#include <string>
#include <vector>

#include <nudf\exception.hpp>
#include <nudf\nxrmuihlp.hpp>
#include <nudf\shared\rightsdef.h>

#include "labelinfo.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



#define CLASSIFY_XML    \
    L"<ClassificationProfile timestamp=\"2015-04-26T16:48:51\">"\
        L"<Classify>"\
            L"<TopLevel>0,1</TopLevel>"\
            L"<LabelList>"\
                L"<Label id=\"0\" name=\"Sensitivity\" display-name=\"Sensitivity\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
                    L"<VALUE priority=\"0\" value=\"Non Business\" />"\
                    L"<VALUE priority=\"1\" value=\"General Business\" />"\
                    L"<VALUE priority=\"2\" value=\"Proprietary\" />"\
                    L"<VALUE priority=\"3\" value=\"Confidential\" />"\
                L"</Label>"\
                L"<Label id=\"1\" name=\"Program\" display-name=\"Program\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
                    L"<VALUE value=\"PR-01\" sub-label=\"2\"/>"\
                    L"<VALUE value=\"PR-02\" sub-label=\"2\"/>"\
                    L"<VALUE value=\"PR-03\" sub-label=\"2\"/>"\
                L"</Label>"\
                L"<Label id=\"2\" name=\"Jurisdiction\" display-name=\"Jurisdiction\" mandatory=\"true\" multi-select=\"false\" default-value=\"0\">"\
                    L"<VALUE value=\"ITAR\"  sub-label=\"3\"/>"\
                    L"<VALUE value=\"EAR\" sub-label=\"4\"/>"\
                    L"<VALUE value=\"BAFA\" sub-label=\"5\"/>"\
                L"</Label>"\
                L"<Label id=\"3\" name=\"ITAR\" display-name=\"ITAR Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
                    L"<VALUE value=\"TAA-01\" />"\
                    L"<VALUE value=\"TAA-02\" />"\
                    L"<VALUE value=\"TAA-03\" />"\
                    L"<VALUE value=\"TAA-04\" />"\
                L"</Label>"\
                L"<Label id=\"4\" name=\"EAR\" display-name=\"EAR Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
                    L"<VALUE value=\"EAR-01\" />"\
                    L"<VALUE value=\"EAR-02\" />"\
                    L"<VALUE value=\"EAR-03\" />"\
                    L"<VALUE value=\"EAR-04\" />"\
                L"</Label>"\
                L"<Label id=\"5\" name=\"BAFA\" display-name=\"BAFA Property\" mandatory=\"true\" multi-select=\"true\" default-value=\"0\">"\
                    L"<VALUE value=\"BAFA-01\" />"\
                    L"<VALUE value=\"BAFA-02\" />"\
                    L"<VALUE value=\"BAFA-03\" />"\
                    L"<VALUE value=\"BAFA-04\" />"\
                L"</Label>"\
            L"</LabelList>"\
        L"</Classify>"\
    L"</ClassificationProfile>"\

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNxUiTestDlg dialog



CNxUiTestDlg::CNxUiTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNxUiTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNxUiTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNxUiTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_SHOWPROP_GENERAL, &CNxUiTestDlg::OnBnClickedBtnShowpropGeneral)
    ON_BN_CLICKED(IDC_BTN_SHOWPROP_PERMISSION, &CNxUiTestDlg::OnBnClickedBtnShowpropPermission)
    ON_BN_CLICKED(IDC_BTN_SHOWPROP_TAGS, &CNxUiTestDlg::OnBnClickedBtnShowpropTags)
    ON_BN_CLICKED(IDC_BTN_SHOWPROP_ALL, &CNxUiTestDlg::OnBnClickedBtnShowpropAll)
    ON_BN_CLICKED(IDC_BTN_SHOWCLASSIFY, &CNxUiTestDlg::OnBnClickedBtnShowclassify)
    ON_BN_CLICKED(IDC_BTN_SHOWPROP_DETAILS, &CNxUiTestDlg::OnBnClickedBtnShowpropDetails)
END_MESSAGE_MAP()


// CNxUiTestDlg message handlers

BOOL CNxUiTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNxUiTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNxUiTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNxUiTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

typedef LONG (WINAPI* SHOWPROPDLG)(_In_ LPCWSTR file, _In_opt_ HWND hParent, _In_ UINT nStartPage);
static const UCHAR KekId[20] = {
    0x2E, 0x48, 0x23, 0x82, 0x97, 0x38, 0xFF, 0x6E, 0x0F, 0xA3,
    0xEC, 0x2B, 0x50, 0x0E, 0xBB, 0x68, 0x19, 0x8A, 0xF8, 0x00
};
static const UCHAR KrkId[20] = {
    0x15, 0x5E, 0x06, 0x02, 0x1E, 0x6C, 0xC7, 0xA5, 0x14, 0xF7,
    0x24, 0xAF, 0xCE, 0xFC, 0x3B, 0xFE, 0x1D, 0x0D, 0x24, 0x0A
};
static const GUID FileUid = {0x3F2504E0, 0x4F89, 0x41D3, 0x9A, 0x0C, 0x03, 0x05, 0xE8, 0x2C, 0x33, 0x01};

void CNxUiTestDlg::OnBnClickedBtnShowpropGeneral()
{
    // TODO: Add your control notification handler code here
    nudf::util::CRmuObject rmu;
    NXL_HEADER             header;
    WCHAR                  wzDll[MAX_PATH] = {0};

    GetCurrentDirectoryW(MAX_PATH, wzDll);
    wcscat_s(wzDll, L"\\nxrmcmui.dll");

    try {
        rmu.Initialize(wzDll);

        memset(&header, 0, sizeof(header));
        header.Signature.Code.HighPart = NXL_SIGNATURE_HIGH;
        header.Signature.Code.LowPart = NXL_SIGNATURE_LOW;
        wcsncpy_s(header.Signature.Message, 67, L"LAM class-0 itar file", _TRUNCATE);
        memcpy(header.Basic.Thumbprint, &FileUid, sizeof(GUID));
        header.Basic.Version = NXL_VERSION_10;
        header.Basic.Flags = 0;
        header.Basic.Alignment = NXL_PAGE_SIZE;
        header.Basic.PointerOfContent = NXL_MIN_SIZE;
        header.Crypto.Algorithm = NXL_ALGORITHM_AES256;
        header.Crypto.CbcSize = NXL_CBC_SIZE;
        header.Crypto.PrimaryKey.KeKeyId.Algorithm = NXL_ALGORITHM_AES256;
        header.Crypto.PrimaryKey.KeKeyId.IdSize = 20;
        memcpy(header.Crypto.PrimaryKey.KeKeyId.Id, KekId, 20);
        header.Crypto.RecoveryKey.KeKeyId.Algorithm = NXL_ALGORITHM_RSA2048;
        header.Crypto.RecoveryKey.KeKeyId.IdSize = 20;
        memcpy(header.Crypto.RecoveryKey.KeKeyId.Id, KrkId, 20);


        rmu.RmuShowGeneralPropPage(GetSafeHwnd(), L"d:\\temp\\temp.docx", &header);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        MessageBoxW(L"Fail to load DLL", L"Error", MB_OK);
    }
}


void CNxUiTestDlg::OnBnClickedBtnShowpropPermission()
{
    // TODO: Add your control notification handler code here
    nudf::util::CRmuObject rmu;
    ULONGLONG newrights = 0;
    ULONGLONG rights = BUILTIN_RIGHT_VIEW
                       | BUILTIN_RIGHT_EDIT
                       | BUILTIN_RIGHT_ANNOTATE
                       | BUILTIN_RIGHT_SEND
                       | BUILTIN_RIGHT_CLASSIFY;
    WCHAR                  wzDll[MAX_PATH] = {0};

    GetCurrentDirectoryW(MAX_PATH, wzDll);
    wcscat_s(wzDll, L"\\nxrmcmui.dll");

    try {
        rmu.Initialize(wzDll);
        rmu.RmuShowPermissionPropPage(NULL, L"d:\\temp\\temp.docx", rights, &newrights);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        MessageBoxW(L"Fail to load DLL", L"Error", MB_OK);
    }
}


void CNxUiTestDlg::OnBnClickedBtnShowpropTags()
{
    // TODO: Add your control notification handler code here
    nudf::util::CRmuObject rmu;
    std::vector<std::pair<std::wstring, std::wstring>> classifydata;
    WCHAR                  wzDll[MAX_PATH] = {0};

    GetCurrentDirectoryW(MAX_PATH, wzDll);
    wcscat_s(wzDll, L"\\nxrmcmui.dll");
    
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Sensitivity", L"Proprietary"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Program", L"PR-02"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Jurisdiction", L"EAR"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-01"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-04"));

    try {
        rmu.Initialize(wzDll);
        rmu.RmuShowClassifyPropPage(NULL, L"d:\\temp\\temp.docx", classifydata);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        MessageBoxW(L"Fail to load DLL", L"Error", MB_OK);
    }
}


void CNxUiTestDlg::OnBnClickedBtnShowpropAll()
{
    // TODO: Add your control notification handler code here
    nudf::util::CRmuObject rmu;
    NXL_HEADER             header;
    std::vector<std::pair<std::wstring, std::wstring>> classifydata;
    ULONGLONG rights = BUILTIN_RIGHT_VIEW
                       | BUILTIN_RIGHT_EDIT
                       | BUILTIN_RIGHT_ANNOTATE
                       | BUILTIN_RIGHT_SEND 
                       | BUILTIN_RIGHT_CLASSIFY;
    WCHAR                  wzDll[MAX_PATH] = {0};

    GetCurrentDirectoryW(MAX_PATH, wzDll);
    wcscat_s(wzDll, L"\\nxrmcmui.dll");
    
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Sensitivity", L"Proprietary"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Program", L"PR-02"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Jurisdiction", L"EAR"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-01"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-04"));

    try {
        rmu.Initialize(wzDll);

        memset(&header, 0, sizeof(header));
        header.Signature.Code.HighPart = NXL_SIGNATURE_HIGH;
        header.Signature.Code.LowPart = NXL_SIGNATURE_LOW;
        wcsncpy_s(header.Signature.Message, 67, L"LAM class-0 itar file", _TRUNCATE);
        memcpy(header.Basic.Thumbprint, &FileUid, sizeof(GUID));
        header.Basic.Version = NXL_VERSION_10;
        header.Basic.Flags = 0;
        header.Basic.Alignment = NXL_PAGE_SIZE;
        header.Basic.PointerOfContent = NXL_MIN_SIZE;
        header.Crypto.Algorithm = NXL_ALGORITHM_AES256;
        header.Crypto.CbcSize = NXL_CBC_SIZE;
        header.Crypto.PrimaryKey.KeKeyId.Algorithm = NXL_ALGORITHM_AES256;
        header.Crypto.PrimaryKey.KeKeyId.IdSize = 20;
        memcpy(header.Crypto.PrimaryKey.KeKeyId.Id, KekId, 20);
        header.Crypto.RecoveryKey.KeKeyId.Algorithm = NXL_ALGORITHM_RSA2048;
        header.Crypto.RecoveryKey.KeKeyId.IdSize = 20;
        memcpy(header.Crypto.RecoveryKey.KeKeyId.Id, KrkId, 20);

        rmu.RmuShowAllPropPages(NULL, L"d:\\temp\\temp.docx", &header, rights, classifydata);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        MessageBoxW(L"Fail to load DLL", L"Error", MB_OK);
    }
}

void CNxUiTestDlg::OnBnClickedBtnShowclassify()
{
    // TODO: Add your control notification handler code here
    nudf::util::CRmuObject rmu;
    std::vector<std::pair<std::wstring,std::wstring>> tags;
    WCHAR wzDll[MAX_PATH] = {0};

    tags.push_back(std::pair<std::wstring,std::wstring>(L"Sensitivity", L"Proprietary"));
    tags.push_back(std::pair<std::wstring,std::wstring>(L"Program", L"PR-02"));
    tags.push_back(std::pair<std::wstring,std::wstring>(L"Jurisdiction", L"EAR"));
    tags.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-01"));
    tags.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-04"));

    GetCurrentDirectoryW(MAX_PATH, wzDll);
    wcscat_s(wzDll, L"\\nxrmcmui.dll");


    try {        
        rmu.Initialize(wzDll);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        MessageBoxW(L"Fail to load DLL", L"Error");
        return;
    }

    std::wstring wsDocFile = L"d:\\classify.docx";
    std::wstring wsXmlFile = L"C:\\Program Files\\NextLabs\\Rights Management\\conf\\classify.xml";
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wsDocFile.c_str())) {
        wsDocFile = L"classify.docx";
    }
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wsXmlFile.c_str())) {
        wsXmlFile = L"d:\\classify.xml";
    }
    if(INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wsXmlFile.c_str())) {
        wsXmlFile = L"classify.xml";
    }
    LONG lRet = rmu.RmuShowClassifyDialogEx(GetSafeHwnd(), wsDocFile.c_str(), wsXmlFile.c_str(), 0x0409, FALSE, tags);
    if(0 == lRet) {
        MessageBoxW(L"OK");
    }
    else {
        MessageBoxW(L"Canceled");
    }
}


void CNxUiTestDlg::OnBnClickedBtnShowpropDetails()
{
    // TODO: Add your control notification handler code here
    nudf::util::CRmuObject rmu;
    std::vector<std::pair<std::wstring, std::wstring>> classifydata;
    WCHAR                  wzDll[MAX_PATH] = {0};
    ULONGLONG rights = BUILTIN_RIGHT_VIEW
                       | BUILTIN_RIGHT_EDIT
                       | BUILTIN_RIGHT_ANNOTATE
                       | BUILTIN_RIGHT_SEND 
                       | BUILTIN_RIGHT_CLASSIFY;

    GetCurrentDirectoryW(MAX_PATH, wzDll);
    wcscat_s(wzDll, L"\\nxrmcmui.dll");
    
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Sensitivity", L"Proprietary"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Program", L"PR-02"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"Jurisdiction", L"EAR"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-01"));
    classifydata.push_back(std::pair<std::wstring,std::wstring>(L"EAR", L"EAR-04"));

    try {
        rmu.Initialize(wzDll);
        rmu.RmuShowDetailsPropPage(NULL, L"d:\\temp\\temp.docx", rights, classifydata);
    }
    catch(const nudf::CException& e) {
        UNREFERENCED_PARAMETER(e);
        MessageBoxW(L"Fail to load DLL", L"Error", MB_OK);
    }
}
