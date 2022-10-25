
// xmlbuilderDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include <map>

// Percentage increase of font height for Welcome title
//
// We choose 22% here because this value makes the Welcome title font
// identical to the one InstallShield uses in RMC installer at 96dpi (100%)
// and 144dpi (150%) display settings.
#define WELCOME_TITLE_FONT_HT_PERCENT_BOOST     22      // 22%

#define MAX_NUM_PLATFORMS       2 // max. # of platforms (Win-32bit, etc.)

// CxmlbuilderDlg dialog
class CxmlbuilderDlg : public CDialogEx
{
// Construction
public:
	CxmlbuilderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_XMLBUILDER_DIALOG};

	void SetPlatform(const std::vector<CString>& m_vecplatform,const std::map<CString,std::vector<CString>>& m_mapdependfiles);
	void SetCert(const std::wstring cert);
	void SetDefaultUrl(const std::wstring url);
	void SetDefaultUseRmsLogin(bool useRmsLogin);
	void SetDefaultDefaultDomain(const std::wstring defaultDomain);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	void InitUIDefaultValue();
	void EnableUI(int which, bool bEnable);
	void GetUserResult();
	bool ConfirmAndAcceptDestPath(const CString path);
	void DisplayLastError(const CString prefix, const CString suffix);
	bool EnsureDirectoryExists(const CString path);
	bool GenerateXMLFile(std::map<std::wstring, std::wstring>& mapXMLInfo, const std::wstring& filepath);
	bool InitLangugeSet();

public:
	// Page 1
	CStatic m_page1WelcomePic;
	CStatic m_page1WelcomeTitle;
	CStatic m_page1WelcomeText;
	CButton m_page1Agree;

	// Pages 2-4
	CStatic m_bannerPic;
	CStatic m_bannerTitle;
	CStatic m_bannerText;

	// Page 2
	CStatic m_page2PlatformPrompt;
	CButton m_page2Platform[MAX_NUM_PLATFORMS];
	CStatic m_page2LanguagePrompt;
	CComboBox m_page2Language;

	// Page 3
	CStatic m_page3UrlPrompt;
	CEdit   m_page3Url;
	CStatic m_page3DestPathPrompt;
	CEdit   m_page3DestPath;
	CButton m_page3Browse;
	CStatic m_page3InvalidSymbols;
	CButton m_page3UseRmsLogin;
	CStatic m_page3DefaultDomainPrompt;
	CEdit   m_page3DefaultDomain;

	// Page 4
	CStatic m_page4Result;

	// All pages
	CButton m_previous;
	CButton m_nextOK;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedPrevious();
	afx_msg void OnBnClickedAgree();
	afx_msg void OnBnClickedCheckPlatform();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnEnChangeEdit();
	afx_msg void OnBnClickedUseRmsLogin();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
	CBrush	m_brush;
	int currentPage;
	std::map<std::wstring,std::wstring> xmlInfo[MAX_NUM_PLATFORMS];
	int m_numXmlInfo;
	std::wstring m_cert;
	std::wstring m_defaultUrl;
	bool m_defaultUseRmsLogin;
	std::wstring m_defaultDefaultDomain;
	bool m_destPathAccepted;
public:
	std::map<CString,CString> m_languge;	//<"0x0409","English (United States)">
	std::vector<CString>	m_vecconfigurationplatform;	//cache platforms
	CString m_SaveToPath;
	bool m_useRmsLogin;
	CString m_defaultDomain;
	std::map<CString,std::vector<CString>> m_mapvecdependfiles;
};
