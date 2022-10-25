
// imgtest.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CImgTestApp:
// See imgtest.cpp for the implementation of this class
//

class CImgTestApp : public CWinApp
{
public:
	CImgTestApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CImgTestApp theApp;