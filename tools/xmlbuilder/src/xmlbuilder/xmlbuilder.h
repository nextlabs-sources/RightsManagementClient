
// xmlbuilder.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include <string>
#include <vector>
#include <map>

// CXmlConfigName

extern const CString CXmlConfigName;

// CxmlbuilderApp:
// See xmlbuilder.cpp for the implementation of this class
//

class CxmlbuilderApp : public CWinApp
{
public:
	CxmlbuilderApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
private:
	std::wstring GetResourceString(_In_ const unsigned int& unResourceID);
	bool CheckConfiguration(const CString& configName);
	bool CheckDefaultRegisterFile(LPCWSTR configName);

private:
	std::vector<CString>	m_vecplatform;
	std::map<CString,std::vector<CString>> m_mapdependfiles;
	std::wstring			m_cert;
	std::wstring			m_defaultUrl;
	bool					m_defaultUseRmsLogin;
	std::wstring			m_defaultDefaultDomain;
};

extern CxmlbuilderApp theApp;