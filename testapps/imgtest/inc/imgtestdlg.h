
// imgtestdlg.h : header file
//

#pragma once


#include <nudf\image\bitmap.hpp>

// CImgTestDlg dialog
class CImgTestDlg : public CDialogEx
{
// Construction
public:
	CImgTestDlg(CWnd* pParent = NULL);	// standard constructor
    
    nudf::image::CTextBitmap m_txtBmp;
    HBITMAP m_hBmpColor;
    HBITMAP m_hBmpGray;
    HBITMAP m_hBmpDark;
    HBITMAP m_hBmpWhite1;
    HBITMAP m_hBmpWhite2;
    HBRUSH  m_hWhiteBrush;
    HBRUSH  m_hGrayBrush;

// Dialog Data
	enum { IDD = IDD_IMGTEST_DIALOG };

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
    afx_msg void OnBnClickedDark();
    afx_msg void OnBnClickedWhite();
    afx_msg void OnBnClickedWhite2();
    afx_msg void OnBnClickedGray();
    afx_msg void OnBnClickedColor();
    afx_msg void OnDestroy();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
