
// xmlbuilderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "xmlbuilder.h"
#include "xmlbuilderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "Header.h"
#include <fstream>


// CxmlbuilderDlg dialog



CxmlbuilderDlg::CxmlbuilderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CxmlbuilderDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CxmlbuilderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	// Page 1
	DDX_Control(pDX, IDC_STATICwelcomepic, m_page1WelcomePic);
	DDX_Control(pDX, IDC_STATICwelcometitle, m_page1WelcomeTitle);
	DDX_Control(pDX, IDC_STATICwelcometext, m_page1WelcomeText);
	DDX_Control(pDX, IDC_CHECKagree, m_page1Agree);

	// Pages 2-4
	DDX_Control(pDX, IDC_STATICbannerpic, m_bannerPic);
	DDX_Control(pDX, IDC_STATICbannertitle, m_bannerTitle);
	DDX_Control(pDX, IDC_STATICbannertext, m_bannerText);

	// Page 2
	DDX_Control(pDX, IDC_STATICplatformprompt, m_page2PlatformPrompt);
	DDX_Control(pDX, IDC_CHECKplatform0, m_page2Platform[0]);
	DDX_Control(pDX, IDC_CHECKplatform1, m_page2Platform[1]);
	DDX_Control(pDX, IDC_STATIClanguageprompt, m_page2LanguagePrompt);
	DDX_Control(pDX, IDC_COMBOlanguage, m_page2Language);

	// Page 3
	DDX_Control(pDX, IDC_STATICurlprompt, m_page3UrlPrompt);
	DDX_Control(pDX, IDC_EDITurl, m_page3Url);
	DDX_Control(pDX, IDC_STATICdestpathprompt, m_page3DestPathPrompt);
	DDX_Control(pDX, IDC_EDITdestpath, m_page3DestPath);
	DDX_Control(pDX, IDBROWSE, m_page3Browse);
	DDX_Control(pDX, IDC_STATICinvalidsymbols, m_page3InvalidSymbols);
	DDX_Control(pDX, IDC_CHECKusermslogin, m_page3UseRmsLogin);
	DDX_Control(pDX, IDC_STATICdefaultdomainprompt, m_page3DefaultDomainPrompt);
	DDX_Control(pDX, IDC_EDITdefaultdomain, m_page3DefaultDomain);

	// Page 4
	DDX_Control(pDX, IDC_STATICresult, m_page4Result);

	// All pages
	DDX_Control(pDX, IDPREVIOUS, m_previous);
	DDX_Control(pDX, IDOK, m_nextOK);
}

BEGIN_MESSAGE_MAP(CxmlbuilderDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CxmlbuilderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDPREVIOUS, &CxmlbuilderDlg::OnBnClickedPrevious)
	ON_BN_CLICKED(IDC_CHECKagree, &CxmlbuilderDlg::OnBnClickedAgree)
	ON_BN_CLICKED(IDC_CHECKplatform0, &CxmlbuilderDlg::OnBnClickedCheckPlatform)
	ON_BN_CLICKED(IDC_CHECKplatform1, &CxmlbuilderDlg::OnBnClickedCheckPlatform)
	ON_BN_CLICKED(IDBROWSE, &CxmlbuilderDlg::OnBnClickedBrowse)
	ON_EN_CHANGE(IDC_EDITurl, &CxmlbuilderDlg::OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDITdestpath, &CxmlbuilderDlg::OnEnChangeEdit)
	ON_BN_CLICKED(IDC_CHECKusermslogin, &CxmlbuilderDlg::OnBnClickedUseRmsLogin)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CxmlbuilderDlg message handlers
void CxmlbuilderDlg::SetPlatform(const std::vector<CString>& m_vecplatform,const std::map<CString,std::vector<CString>>& m_mapdependfiles)
{
	m_vecconfigurationplatform = m_vecplatform;
	m_mapvecdependfiles = m_mapdependfiles;
}

void CxmlbuilderDlg::SetCert(const std::wstring cert)
{
	m_cert = cert;
}

void CxmlbuilderDlg::SetDefaultUrl(const std::wstring url)
{
	m_defaultUrl = url;
}

void CxmlbuilderDlg::SetDefaultUseRmsLogin(bool useRmsLogin)
{
	m_defaultUseRmsLogin = useRmsLogin;
}

void CxmlbuilderDlg::SetDefaultDefaultDomain(const std::wstring defaultDomain)
{
	m_defaultDefaultDomain = defaultDomain;
}

BOOL CxmlbuilderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	const LOGBRUSH logBrush = {BS_HOLLOW};
	m_brush.CreateBrushIndirect(&logBrush);
	InitUIDefaultValue();
	currentPage = 1;
	m_numXmlInfo = 0;
	m_destPathAccepted = false;

	EnableUI(currentPage,true);

	for(int i = currentPage+1;i <= MAXPAGESIZE; ++i)
	{
		EnableUI(i,false);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CxmlbuilderDlg::OnPaint()
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
HCURSOR CxmlbuilderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CxmlbuilderDlg::EnableUI(int which, bool bEnable)
{
	const int nCmdShow = bEnable ? SW_SHOW : SW_HIDE;

	switch(which)
	{
	case 1:
		if(bEnable)
		{
			m_nextOK.EnableWindow(m_page1Agree.GetCheck() == BST_CHECKED);
			m_nextOK.SetWindowTextW(L"&Next >");
		}

		m_page1WelcomePic.ShowWindow(nCmdShow);
		m_page1WelcomeTitle.ShowWindow(nCmdShow);
		m_page1WelcomeText.ShowWindow(nCmdShow);
		m_page1Agree.ShowWindow(nCmdShow);
		break;
	case 2:
		if(bEnable)
		{
			bool bSomeSysChecked = false;
			for( int i = 0; i < MAX_NUM_PLATFORMS; i++ )
			{
				if( m_page2Platform[i].GetCheck() == BST_CHECKED )
				{
					bSomeSysChecked = true;
					break;
				}
			}
			m_nextOK.EnableWindow(bSomeSysChecked);
		}

		m_page2PlatformPrompt.ShowWindow(nCmdShow);
		for( int i = 0; i < MAX_NUM_PLATFORMS; i++ )
		{
			m_page2Platform[i].ShowWindow(nCmdShow);
		}
		m_page2LanguagePrompt.ShowWindow(nCmdShow);
		m_page2Language.ShowWindow(nCmdShow);
		break;
	case 3:
		m_page3UrlPrompt.ShowWindow(nCmdShow);
		m_page3Url.ShowWindow(nCmdShow);
		m_page3DestPathPrompt.ShowWindow(nCmdShow);
		m_page3DestPath.ShowWindow(nCmdShow);
		m_page3Browse.ShowWindow(nCmdShow);
		if(!bEnable)
		{
			m_page3InvalidSymbols.ShowWindow(SW_HIDE);
		}

		m_page3UseRmsLogin.ShowWindow(nCmdShow);
		if(bEnable && m_page3UseRmsLogin.GetCheck() == BST_CHECKED)
		{
			m_page3DefaultDomainPrompt.ShowWindow(SW_SHOW);
			m_page3DefaultDomain.ShowWindow(SW_SHOW);
		}
		else
		{
			m_page3DefaultDomainPrompt.ShowWindow(SW_HIDE);
			m_page3DefaultDomain.ShowWindow(SW_HIDE);
		}
		break;
	case 4:
		if(bEnable)
		{
			std::wstring result;

			result.append(L"Platform(s):\t\t");
			for( int i = 0; i < m_numXmlInfo; i++ )
			{
				if( i > 0 )
				{
					result.append(L", ");
				}
				result.append(xmlInfo[i][L"PLATFORM"]);
			}
			result.append(L"\n\nLanguage:\t\t"+xmlInfo[0][L"LANGUAGE"]);
			result.append(L"\n\nWeb service URL:\t"+xmlInfo[0][L"SERVER"]);
			result.append(L"\n\nDestination location:\t"+m_SaveToPath);
			result.append(std::wstring(L"\n\nUse RMS login:\t\t")+(m_useRmsLogin?L"Yes":L"No"));

			if( m_useRmsLogin )
			{
				result.append(L"\n\nDefault domain:\t\t"+m_defaultDomain);
			}

			m_page4Result.SetWindowText(result.c_str());
		}

		m_page4Result.ShowWindow(nCmdShow);
		break;
	}

	switch(which)
	{
	case 2:
	case 3:
	case 4:
		if(bEnable)
		{
			m_bannerTitle.SetWindowText(bannerTitle[which-1].c_str());
			m_bannerText.SetWindowText(bannerText[which-1].c_str());
		}
		m_bannerPic.ShowWindow(nCmdShow);
		m_bannerTitle.ShowWindow(nCmdShow);
		m_bannerText.ShowWindow(nCmdShow);
		break;
	}
}

void CxmlbuilderDlg::GetUserResult()
{
	int i = 0;

	for( int j = 0; j < MAX_NUM_PLATFORMS; j++ )
	{
		if( m_page2Platform[j].GetCheck() != BST_CHECKED )
		{
			continue;
		}

		CString temp;
		m_page3Url.GetWindowText(temp);
		xmlInfo[i][L"SERVER"] = temp.GetString();
		m_page2Language.GetWindowText(temp);
		xmlInfo[i][L"LANGUAGE"] = temp.GetString();
		m_page2Platform[j].GetWindowText(temp);
		xmlInfo[i][L"PLATFORM"] = temp.GetString();

		//Currently TENANTID is not used.  Add dummy entry.
		xmlInfo[i][L"TENANTID"] = L"N/A";

		i++;
	}

	m_numXmlInfo = i;
}

// Return true if path is accepted.
bool CxmlbuilderDlg::ConfirmAndAcceptDestPath(const CString path)
{
	if( PathIsDirectory(path) == (BOOL)FILE_ATTRIBUTE_DIRECTORY &&
		IDNO == ::MessageBoxW(theApp.m_pMainWnd->m_hWnd,path + L" folder already exists.  All existing files will be overwritten.  Do you want to use it anyway?",L"Warning",MB_YESNO|MB_ICONWARNING|MB_APPLMODAL) )
	{
		return false;
	}

	m_SaveToPath = path;
	m_page3DestPath.SetWindowText(path.GetString());
	return true;
}

void CxmlbuilderDlg::DisplayLastError(const CString prefix, const CString suffix)
{
	DWORD lastErr = GetLastError();
	LPTSTR lpBuffer;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				  NULL, lastErr, 0, (LPTSTR)&lpBuffer, 0, NULL);
	::MessageBoxW(theApp.m_pMainWnd->m_hWnd, prefix + L": " + lpBuffer + suffix,
				  L"Error", MB_OK|MB_ICONERROR|MB_APPLMODAL);
	LocalFree(lpBuffer);
}

// The behavior of EnsureDirectoryExists function is as follow:
// - Return true if the directory already exists, or has been created
//   successfully.
// - Return false if the directory cannot be created for some reason, e.g.
//   - a file with the passed path already exists.
//   - permission problem. 
//   - I/O error.
//   An error dialog will have been displayed in this case.
bool CxmlbuilderDlg::EnsureDirectoryExists(const CString fullPath)
{
	if( PathFileExists(fullPath.GetString()) )
	{
		if( PathIsDirectory(fullPath.GetString()) == (BOOL)FILE_ATTRIBUTE_DIRECTORY )
		{
			return true;
		}
		else
		{
			::MessageBoxW(theApp.m_pMainWnd->m_hWnd,
						  L"Cannot create folder " + fullPath + L".  A file with this path name already exists.  The application will exit.",
						  L"Error",
						  MB_OK|MB_ICONERROR|MB_APPLMODAL);
			return false;
		}
	}

	int index = fullPath.ReverseFind(L'\\');
	if( index == -1 )
	{
		// fullPath is the root directory of a drive that we cannot access.
		DisplayLastError(L"Cannot access volume \"" + fullPath + L"\"", L"The application will exit.");
		return false;
	}

	CString parentPath = fullPath;
	parentPath.Delete(index, INT_MAX - index);

	if( !EnsureDirectoryExists(parentPath) )
	{
		return false;
	}

	if( CreateDirectory(fullPath, NULL) )
	{
		return true;
	}
	else 
	{
		DisplayLastError(L"Cannot create folder " + fullPath, L"The application will exit.");
		return false;
	}
}

void CxmlbuilderDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CString buf;
	m_nextOK.GetWindowTextW(buf);
	if(buf == TEXT("&Finish"))
	{
		for( int i = 0; i < m_numXmlInfo; i++ )
		{
			CString platformDir = m_SaveToPath+L"\\"+xmlInfo[i][L"PLATFORM"].c_str();

			if( !EnsureDirectoryExists(platformDir) )
			{
				CDialogEx::OnOK();
				return;
			}

			GenerateXMLFile(xmlInfo[i],std::wstring(platformDir+L"\\register.xml"));
			for( auto one : m_mapvecdependfiles[xmlInfo[i][L"PLATFORM"].c_str()] )
			{
				if( !CopyFile(one,platformDir + L"\\" + one.Mid(one.Find(L'\\')),FALSE) )
				{
					DisplayLastError(L"Cannot copy file to " + platformDir + L"\\" + one.Mid(one.Find(L'\\')), L"The application will exit.");
					CDialogEx::OnOK();
					return;
				}
			}
		}
		CDialogEx::OnOK();
	}
	else
	{
		if( currentPage == 3 )
		{
			CString temp;
			m_page3DestPath.GetWindowText(temp);

			wchar_t buf[MAX_PATH];
			if( !PathCanonicalize(buf, temp.GetString()) )
			{
				DisplayLastError(L"Error canonicalizing path " + temp, L"The application will exit.");
				CDialogEx::OnOK();
				return;
			}
			temp = buf;

			if( temp.Right(11).CompareNoCase(L"\\RMCpackage") != 0)
			{
				if( temp.Right(1) != L'\\' )
				{
					temp += L'\\';
				}
				temp += L"RMCpackage";
			}

			if( !m_destPathAccepted )
			{
				if( !ConfirmAndAcceptDestPath(temp) )
				{
					return;
				}
				m_destPathAccepted = true;
			}

			m_useRmsLogin = (m_page3UseRmsLogin.GetCheck() == BST_CHECKED);
			if( m_useRmsLogin )
			{
				m_page3DefaultDomain.GetWindowText(m_defaultDomain);
			}
		}

		currentPage++;
		
		//m_toppicture
		if(currentPage == MAXPAGESIZE)
		{
			GetUserResult();
			m_nextOK.SetWindowTextW(L"&Finish");
		}
		if(currentPage > 1)
		{
			m_previous.EnableWindow(TRUE);
		}

		if( currentPage == MAXPAGESIZE-1 )
		{
			CString temppath,tempdns;
			m_page3DestPath.GetWindowText(temppath);
			m_page3Url.GetWindowText(tempdns);
			if( tempdns.IsEmpty() || temppath.IsEmpty() )
			{
				m_nextOK.EnableWindow(FALSE);
			}
			else
			{
				m_nextOK.EnableWindow(TRUE);
			}
		}
		if( currentPage == 3 )
		{
			for( int i = 0; i < MAX_NUM_PLATFORMS; i++ )
			{
				CString temp;

				if( m_page2Platform[i].GetCheck() != BST_CHECKED )
				{
					continue;
				}
				m_page2Platform[i].GetWindowText(temp);

				for( auto two : m_mapvecdependfiles[temp] )
				{
					if( !PathFileExists(two) )
					{
						::MessageBoxW(theApp.m_pMainWnd->m_hWnd,two + L" is not found.  The application will exit.",L"Error",MB_OK|MB_ICONERROR|MB_APPLMODAL);
						CDialogEx::OnCancel();
						break;
					}
				}
			}
		}
		EnableUI(currentPage-1,false);
		EnableUI(currentPage,true);
	}
}


void CxmlbuilderDlg::OnBnClickedPrevious()
{
	// TODO: Add your control notification handler code here
	currentPage--;
	EnableUI(currentPage+1,false);
	EnableUI(currentPage,true);

	m_nextOK.SetWindowTextW(L"&Next >");
	m_nextOK.EnableWindow(TRUE);
	if(currentPage == 1)
	{
		m_previous.EnableWindow(FALSE);
	}
}

bool CxmlbuilderDlg::InitLangugeSet()
{
	bool bRet = false;

	wchar_t buffer[4096*2] = { 0 };
	wchar_t key[MAX_PATH] = { 0 };
	wchar_t name[MAX_PATH] = { 0 };
	DWORD  dw = GetPrivateProfileSection(L"Languages",buffer,4096*2,CXmlConfigName.AllocSysString());
	int pos = 0;
	for( DWORD  i = 1; i < dw; i++ )
	{
		if( buffer[i] == L'\0' )
		{
			bool bflage = true;
			for( DWORD m=pos; m<=i ;m++)
			{
				if(buffer[m] == L'=')
				{
					key[m-pos] = L'\0';
					bflage = false;
					pos = m+1;
				}
				else if(bflage)
				{
					key[m-pos] = buffer[m];
				}
				else
				{
					name[m-pos] = buffer[m];
				}
			}
			pos = i+1;
			m_languge[key] = name;
			bRet = true;
		}
	}
	return bRet;
}

void CxmlbuilderDlg::InitUIDefaultValue()
{
	// This text is longer than 256 characters, which generates RC4206
	// warning and gets truncated if we put it statically in the .rc file.
	// So we have to set it here at run-time instead.
	m_page1WelcomeText.SetWindowText(welcomeText.c_str());

	for( int i = 0; i < MAX_NUM_PLATFORMS; i++ )
	{
		m_page2Platform[i].SetWindowText(m_vecconfigurationplatform[i]);
	}

	m_previous.EnableWindow(FALSE);

	if(!InitLangugeSet())
	{
		::MessageBoxW(NULL,L"The language set was not found in the configuration file.  The application will use English (United States) as the default language.",L"Warning",MB_OK|MB_ICONWARNING);
	}
	for( auto onelanguge : m_languge )
	{
		m_page2Language.AddString(onelanguge.second);
	}

	m_page2Language.SelectString(-1,m_languge[L"0x0409"]);

	m_page3Url.SetWindowText(m_defaultUrl.c_str());
	m_page3UseRmsLogin.SetCheck(m_defaultUseRmsLogin?BST_CHECKED:BST_UNCHECKED);
	m_page3DefaultDomain.SetWindowText(m_defaultDefaultDomain.c_str());
}

void CxmlbuilderDlg::OnBnClickedAgree()
{
	// TODO: Add your control notification handler code here
	if(m_page1Agree.GetCheck() == BST_CHECKED)
	{
		m_nextOK.EnableWindow(TRUE);
	}
	else
	{
		m_nextOK.EnableWindow(FALSE);
	}
}

void CxmlbuilderDlg::OnBnClickedCheckPlatform()
{
	for( int i = 0; i < MAX_NUM_PLATFORMS; i++ )
	{
		if( m_page2Platform[i].GetCheck() == BST_CHECKED )
		{
			m_nextOK.EnableWindow(TRUE);
			return;
		}
	}

	m_nextOK.EnableWindow(FALSE);
}

void CxmlbuilderDlg::OnBnClickedBrowse()
{
	// TODO: Add your control notification handler code here
	CFolderPickerDialog fpdlg;
	if(fpdlg.DoModal() == IDOK )
	{
		CString temp = fpdlg.GetFolderPath();
		if( temp.Right(11).CompareNoCase(L"\\RMCpackage") != 0)
		{
			if( temp.Right(1) != L'\\' )
			{
				temp.Append(L"\\");
			}
			temp.Append(L"RMCpackage");
		}
		if( ConfirmAndAcceptDestPath(temp) )
		{
			m_destPathAccepted = true;
		}
	}
}

bool CxmlbuilderDlg::GenerateXMLFile(std::map<std::wstring, std::wstring>& mapXMLInfo, const std::wstring& filepath)
{
	std::wstring xmlinfo = L"<?xml version=\"1.0\"?>";
	xmlinfo.append(L"\n<REGISTER>");

	for( auto one : m_languge )
	{
		if(one.second.Compare(mapXMLInfo[L"LANGUAGE"].c_str()) == 0)
		{
			mapXMLInfo[L"LANGUAGE"] = one.first;
		}
	}

	if( !m_cert.empty() )
	{
		mapXMLInfo[L"CERTIFICATE"] = m_cert;
	}

	for( std::map<std::wstring,std::wstring>::reverse_iterator rit = mapXMLInfo.rbegin(); rit != mapXMLInfo.rend(); ++rit)
	{
		xmlinfo.append(L"\n    <");
		xmlinfo.append(rit->first);
		xmlinfo.append(L">");
		xmlinfo.append(rit->second);
		xmlinfo.append(L"</");
		xmlinfo.append(rit->first);
		xmlinfo.append(L">");
	}

	xmlinfo.append(L"\n    <AUTHN>");
	xmlinfo.append(L"\n        <TYPE>");
	xmlinfo.append(m_useRmsLogin ? L"external" : L"local");
	xmlinfo.append(L"</TYPE>");
	if( m_useRmsLogin && m_defaultDomain != L"")
	{
		xmlinfo.append(L"\n        <DOMAINS>");
		xmlinfo.append(L"\n            <DOMAIN>");
		xmlinfo.append(m_defaultDomain);
		xmlinfo.append(L"</DOMAIN>");
		xmlinfo.append(L"\n        </DOMAINS>");
	}
	xmlinfo.append(L"\n    </AUTHN>");

	xmlinfo.append(L"\n</REGISTER>");

	std::wofstream ofs(filepath);
	ofs << xmlinfo <<std::endl;
	ofs.close();
	return true;
}




void CxmlbuilderDlg::OnEnChangeEdit()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CString temppath,tempdns;
	m_page3DestPath.GetWindowText(temppath);
	m_page3Url.GetWindowText(tempdns);

	bool bWrongFileNmae = true;
	std::wstring filename = temppath.GetString();
	std::size_t pos = filename.find(L'\\');
	if(pos != std::wstring::npos)
	{
		std::wstring temp = filename.substr(0,pos+1);
		if(PathIsRootW(temp.c_str()) &&
		   (pos == filename.length()-1 ||
			(filename[filename.length()-1] != L'\\' &&
			 filename.find(L"\\\\") == std::wstring::npos)))
		{
			bWrongFileNmae = false;
			wchar_t ivalidsymbol[] = {L'<', L'>', L'|', L'"', L'/', L':', L'*', L'?' };
			temp = filename.substr(pos);
			for( int i = 0; i < sizeof(ivalidsymbol)/sizeof(wchar_t); i++)
			{
				if(temp.find(ivalidsymbol[i]) != std::wstring::npos)
				{
					bWrongFileNmae = true;
					break;
				}
			}
			if(bWrongFileNmae)
			{
				m_page3InvalidSymbols.ShowWindow(SW_SHOW);
			}
			else
			{
				m_page3InvalidSymbols.ShowWindow(SW_HIDE);
			}
		}
	}
	

	if( bWrongFileNmae || tempdns.IsEmpty() || temppath.IsEmpty() )
	{
		m_nextOK.EnableWindow(FALSE);
	}
	else
	{
		m_nextOK.EnableWindow(TRUE);
	}

	m_destPathAccepted = false;
}

void CxmlbuilderDlg::OnBnClickedUseRmsLogin()
{
	const int nCmdShow = (m_page3UseRmsLogin.GetCheck() == BST_CHECKED ?
						  SW_SHOW : SW_HIDE);
	m_page3DefaultDomainPrompt.ShowWindow(nCmdShow);
	m_page3DefaultDomain.ShowWindow(nCmdShow);
}

inline double RoundToNearest(double x)
{
	return _copysign(floor(fabs(x) + 0.5), x);
}

HBRUSH CxmlbuilderDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	if( pWnd->m_hWnd == m_page1WelcomeTitle.m_hWnd ||
	    pWnd->m_hWnd == m_page1WelcomeText.m_hWnd ||
	    pWnd->m_hWnd == m_page1Agree.m_hWnd ||
	    pWnd->m_hWnd == m_bannerTitle.m_hWnd ||
	    pWnd->m_hWnd == m_bannerText.m_hWnd )
	{
		if( pWnd->m_hWnd == m_page1WelcomeTitle.m_hWnd || pWnd->m_hWnd == m_bannerTitle )
		{
			CFont *pFont, newFont;
			LOGFONT logFont;

			// Cannot use pDC->GetCurrentFont() here, because
			// m_hAttribDC is NULL which would trigger a failed
			// assertion in afxwin1.inl line 798.
			pFont = CFont::FromHandle((HFONT)::GetCurrentObject(pDC->m_hDC, OBJ_FONT));

			if( pFont->GetLogFont(&logFont) != 0 )
			{
				logFont.lfWeight = FW_BOLD;
				if( pWnd->m_hWnd == m_page1WelcomeTitle.m_hWnd )
				{
					logFont.lfHeight = (LONG)
					  RoundToNearest(logFont.lfHeight * (1 + ((double) WELCOME_TITLE_FONT_HT_PERCENT_BOOST) / 100));
				}

				if( newFont.CreateFontIndirect(&logFont) )
				{
					pDC->SelectObject(&newFont);
				}
			}
		}

		pDC->SetBkMode(TRANSPARENT);
		return m_brush;
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

