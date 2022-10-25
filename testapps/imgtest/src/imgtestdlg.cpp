
// imgtestdlg.cpp : implementation file
//

#include "stdafx.h"
#include "imgtest.h"
#include "imgtestdlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CImgTestDlg dialog



CImgTestDlg::CImgTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImgTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CImgTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CImgTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_DARK, &CImgTestDlg::OnBnClickedDark)
    ON_BN_CLICKED(IDC_WHITE, &CImgTestDlg::OnBnClickedWhite)
    ON_BN_CLICKED(IDC_WHITE2, &CImgTestDlg::OnBnClickedWhite2)
    ON_BN_CLICKED(IDC_GRAY, &CImgTestDlg::OnBnClickedGray)
    ON_BN_CLICKED(IDC_COLOR, &CImgTestDlg::OnBnClickedColor)
    ON_WM_DESTROY()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CImgTestDlg message handlers

BOOL CImgTestDlg::OnInitDialog()
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
    
    m_hBmpColor = ::LoadBitmapW(NULL, MAKEINTRESOURCEW(IDB_BMP_COLOR));
    m_hBmpGray = ::LoadBitmapW(NULL, MAKEINTRESOURCEW(IDB_BMP_DARK));
    m_hBmpDark = ::LoadBitmapW(NULL, MAKEINTRESOURCEW(IDB_BMP_DARK));
    m_hBmpWhite1 = ::LoadBitmapW(NULL, MAKEINTRESOURCEW(IDB_BMP_WHITE));
    m_hBmpWhite2 = ::LoadBitmapW(NULL, MAKEINTRESOURCEW(IDB_BMP_WHITE2));

    
    m_hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    m_hGrayBrush = CreateSolidBrush(RGB(196, 196, 196));
    
    HFONT hFont = CreateFontW(36, 0, 0, 0, FW_BOLD,
                        0, FALSE, FALSE,
                        ANSI_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        PROOF_QUALITY,
                        DEFAULT_PITCH | FF_DONTCARE,
                        L"Sitka Text");

    if(m_txtBmp.Create(L"Hello, World!", hFont, RGB(255,255,255), RGB(0,0,0))) {

        m_txtBmp.ToFile(L"text.bmp");

        if(m_txtBmp.Rotate(NULL, -45)) {
            m_txtBmp.ToFile(L"text-rotated.bmp");
        }
        else {
            MessageBoxW(L"Fail to rotate text bmp!");
        }
    }
    else {
        MessageBoxW(L"Fail to create text bmp!");
    }
    
    ::DeleteObject(hFont);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CImgTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CImgTestDlg::OnPaint()
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
HCURSOR CImgTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CImgTestDlg::OnBnClickedDark()
{
    // TODO: Add your control notification handler code here
}


void CImgTestDlg::OnBnClickedWhite()
{
    // TODO: Add your control notification handler code here
}


void CImgTestDlg::OnBnClickedWhite2()
{
    // TODO: Add your control notification handler code here
}


void CImgTestDlg::OnBnClickedGray()
{
    // TODO: Add your control notification handler code here
}


void CImgTestDlg::OnBnClickedColor()
{
    // TODO: Add your control notification handler code here
}


void CImgTestDlg::OnDestroy()
{
    CDialogEx::OnDestroy();

    // TODO: Add your message handler code here
}


BOOL CImgTestDlg::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default
    RECT rect = {0, 0, 0, 0};
    HDC hDC = pDC->GetSafeHdc();

    COLORREF bkColor = RGB(255,255,255); // RGB(125,125,125); // RGB(255,255,255);
    COLORREF fontColor = RGB(0,0,0);

    GetClientRect(&rect);
    FloodFill(hDC, 0, 0, fontColor);

    ::SetBkColor(hDC, fontColor);

    return TRUE; //CDialogEx::OnEraseBkgnd(pDC);
}
