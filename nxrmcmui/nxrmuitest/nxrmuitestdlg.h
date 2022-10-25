
// nxrmuitestdlg.h : header file
//

#pragma once

#include <string>

// CNxUiTestDlg dialog
class CNxUiTestDlg : public CDialogEx
{
// Construction
public:
	CNxUiTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NXRMUITEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnShowpropGeneral();
    afx_msg void OnBnClickedBtnShowpropPermission();
    afx_msg void OnBnClickedBtnShowpropTags();
    afx_msg void OnBnClickedBtnShowpropAll();
    afx_msg void OnBnClickedBtnShowclassify();
    afx_msg void OnBnClickedBtnShowpropDetails();
};
