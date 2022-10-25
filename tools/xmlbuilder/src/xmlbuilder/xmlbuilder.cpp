
// xmlbuilder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "nudf\xmlparser.hpp"
#include "nudf\exception.hpp"
#include "xmlbuilder.h"
#include "xmlbuilderDlg.h"

//#include <afx.h>
#include<shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CXmlConfigName

const CString CXmlConfigName(".\\ClientPackageBuilder.cfg");
const LPCWSTR CXmlDefaultRegisterFileName = L"register.xml";


// CxmlbuilderApp

BEGIN_MESSAGE_MAP(CxmlbuilderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CxmlbuilderApp construction

CxmlbuilderApp::CxmlbuilderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CxmlbuilderApp object

CxmlbuilderApp theApp;


// CxmlbuilderApp initialization
std::wstring CxmlbuilderApp::GetResourceString(_In_ const unsigned int& unResourceID)
{
    wchar_t wszResStr[MAX_PATH] = { 0 };
    LoadStringW(GetModuleHandle(NULL), unResourceID, wszResStr, MAX_PATH);
    return wszResStr;
}

bool CxmlbuilderApp::CheckConfiguration(const CString& configName)
{
	bool bRet = true;
	CStdioFile mFile;
	CFileException mExcept;
	mFile.Open(configName,CFile::modeRead,&mExcept);
	if( mExcept.m_cause != CFileException::none )
	{
		if( mExcept.m_cause == CFileException::fileNotFound )
		{
			::MessageBoxW(NULL,GetResourceString(IDS_STRINGfilenotfound).c_str(),GetResourceString(IDS_STRINGerror).c_str(),MB_OK|MB_ICONERROR);
		}
		else
		{
			//@todo other exception
		}
		bRet = false;
	}
	else
	{
		CString oneLine;
		CString oneLinetemp;
		std::vector<CString> m_vecdependfiles;
		while(mFile.ReadString(oneLine))
		{
			if(oneLine.GetAt(0) == '[')
			{
				if( !oneLinetemp.IsEmpty() )
				{
					if( oneLinetemp.CompareNoCase(L"Languages") )
					{
						m_mapdependfiles[oneLinetemp] = m_vecdependfiles;
					}
					oneLinetemp.Empty();
					m_vecdependfiles.clear();
				}
				oneLinetemp = oneLine.Mid(1,oneLine.GetLength()-2);
				if( oneLinetemp.CompareNoCase(L"Languages") )
				{
					m_vecplatform.push_back(oneLinetemp);
				}
			}
			else
			{
				m_vecdependfiles.push_back(oneLine);
			}
		}
		if( oneLinetemp.CompareNoCase(L"Languages") )
		{
			m_mapdependfiles[oneLinetemp] = m_vecdependfiles;
		}
		mFile.Close();
	}

	return bRet;
}

// The behavior of CheckDefaultRegisterFile() function is as follow:
// 1. If register.xml is absent, return true.
// 2. If register.xml is present but the content cannot be read or is not
//    valid XML, return false.
// 3. If register.xml can be read and the content is valid XML, return true
//    regardless of whether <Cert> and/or <URL> are present.
bool CxmlbuilderApp::CheckDefaultRegisterFile(LPCWSTR regFileName)
{
	nudf::util::CXmlDocument doc;

	try
	{
		doc.LoadFromFile(regFileName);
	}
	catch(const nudf::CException& e)
	{
		long lRet = e.GetCode();

		if (lRet == INET_E_OBJECT_NOT_FOUND)
		{
			// File not found.
			return true;
		}
		else
		{
			// lRet is probably one of the deprecated XML_E_xxx error codes.
			// See https://msdn.microsoft.com/en-us/library/ms762295(v=vs.85).aspx
			return false;
		}
	}

	IXMLDOMElement *root;
	if( !doc.GetDocRoot(&root) )
	{
		// Cannot get root for some reason.  Treat it as invalid XML.
		return false;
	}

	IXMLDOMNode *pElem, *pElem2;

	if( doc.FindChildElement(root, L"Cert", &pElem) )
	{
		m_cert = doc.GetNodeText(pElem);
	}

	if( doc.FindChildElement(root, L"URL", &pElem) )
	{
		m_defaultUrl = doc.GetNodeText(pElem);
	}

	if( doc.FindChildElement(root, L"AuthnType", &pElem) )
	{

		m_defaultUseRmsLogin = doc.GetNodeText(pElem) == L"external";
	}

	if( doc.FindChildElement(root, L"Domains", &pElem) &&
		doc.FindChildElement(pElem, L"Domain", &pElem2) )
	{
		m_defaultDefaultDomain = doc.GetNodeText(pElem2);
	}

	return true;
}

BOOL CxmlbuilderApp::InitInstance()
{
	if(!CheckConfiguration(CXmlConfigName))
	{
		return TRUE;
	}

	if (!CheckDefaultRegisterFile(CXmlDefaultRegisterFileName))
	{
		if (IDNO == ::MessageBoxW(NULL, L"The content of register.xml is invalid.  Continue anyway?",
								  L"Warning", MB_YESNO|MB_ICONWARNING))
		{
			return FALSE;
		}
	}

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CxmlbuilderDlg dlg;//(IDD_DIALOG1);
	dlg.SetPlatform(m_vecplatform,m_mapdependfiles);
	dlg.SetCert(m_cert);
	dlg.SetDefaultUrl(m_defaultUrl);
	dlg.SetDefaultUseRmsLogin(m_defaultUseRmsLogin);
	dlg.SetDefaultDefaultDomain(m_defaultDefaultDomain);
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

