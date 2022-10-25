// nxrmcoreaddin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "nxrmcoreaddin.h"
#include "nxrmcoreext2.h"

extern CACHE_ALIGN HMODULE g_hModule;

CACHE_ALIGN	LONG	g_unxrmcoreaddinInstanceCount = 0;
CACHE_ALIGN	LONG	g_unxrmcoreext2InstanceCount = 0;

extern "C" const GUID CLSID_nxrmCoreExt2;

static HRESULT create_key_with_default_value(
	const HKEY	root,
	const WCHAR *parent, 
	const WCHAR *key, 
	const WCHAR *default_value);

static HRESULT set_value_content(
	const WCHAR *key, 
	const WCHAR *valuename, 
	const WCHAR *content);

static HRESULT delete_key(
	const HKEY	root,
	const WCHAR *parent,
	const WCHAR *key);

static HRESULT install_com_component(WCHAR *addin_path);
static HRESULT install_powerpoint_addin(void);
static HRESULT install_excel_addin(void);
static HRESULT install_word_addin(void);
static HRESULT install_outlook_addin(void);

static HRESULT uninstall_com_component(void);
static HRESULT uninstall_powerpoint_addin(void);
static HRESULT uninstall_excel_addin(void);
static HRESULT uninstall_word_addin(void);
static HRESULT uninstall_outlook_addin(void);

Inxrmcoreaddin::Inxrmcoreaddin()
{
	m_uRefCount		= 1;
	m_uLockCount	= 0;
}

Inxrmcoreaddin::~Inxrmcoreaddin()
{

}

STDMETHODIMP Inxrmcoreaddin::QueryInterface(REFIID riid, void **ppobj)
{
	HRESULT hRet = S_OK;

	IUnknown *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if((IID_IUnknown == riid) || (IID_IClassFactory == riid))
		{
			punk = (IUnknown *)this;
		}
		else
		{
			hRet = E_NOINTERFACE;
			break;
		}

		AddRef();

		*ppobj = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP Inxrmcoreaddin::CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject)
{
	HRESULT hr = S_OK;

	nxrmCoreExt2 *p = NULL;

	do 
	{
		if(pUnkOuter)
		{
			*ppvObject = NULL;
			hr = CLASS_E_NOAGGREGATION;
			break;
		}

		p = new nxrmCoreExt2;

		if(!p)
		{
			*ppvObject = NULL;
			hr = E_OUTOFMEMORY;
			break;
		}

		InterlockedIncrement(&g_unxrmcoreext2InstanceCount);

		hr = p->QueryInterface(riid,ppvObject);

		p->Release();

	} while (FALSE);

	return hr;
}

STDMETHODIMP Inxrmcoreaddin::LockServer(BOOL fLock)
{
	if(fLock)
	{
		m_uLockCount++;
	}
	else
	{
		if(m_uLockCount > 0)
			m_uLockCount--;
	}

	return m_uLockCount;	
}

STDMETHODIMP_(ULONG) Inxrmcoreaddin::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) Inxrmcoreaddin::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount && (m_uLockCount == 0))
	{
		delete this;
		InterlockedDecrement(&g_unxrmcoreaddinInstanceCount);
	}

	return uCount;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	HRESULT  hr = E_OUTOFMEMORY;

	Inxrmcoreaddin *InxrmOverlayInstance = NULL;

	if(IsEqualCLSID(rclsid, CLSID_nxrmCoreExt2))
	{
		InxrmOverlayInstance = new Inxrmcoreaddin;

		if(InxrmOverlayInstance)
		{
			InterlockedIncrement(&g_unxrmcoreaddinInstanceCount);

			hr = InxrmOverlayInstance->QueryInterface(riid,ppv);

			InxrmOverlayInstance->Release();
		}
	}
	else
	{
		hr = CLASS_E_CLASSNOTAVAILABLE;
	}

	return(hr);
}

STDAPI DllCanUnloadNow(void)
{
	if(g_unxrmcoreaddinInstanceCount == 0 && g_unxrmcoreext2InstanceCount == 0)
	{
		return S_OK;
	}
	else
	{
		return S_FALSE;
	}
}

STDAPI DllUnregisterServer(void)
{
	uninstall_powerpoint_addin();

	uninstall_excel_addin();

	uninstall_word_addin();

	uninstall_outlook_addin();

	uninstall_com_component();

	return S_OK;
}

STDAPI DllRegisterServer(void)
{
	HRESULT nRet = S_OK;

	WCHAR module_path[MAX_PATH];

	memset(module_path,0,sizeof(module_path));

	do 
	{
		if(!GetModuleFileNameW(g_hModule,
							   module_path,
							   sizeof(module_path)/sizeof(WCHAR)))
		{
			nRet = E_UNEXPECTED;
			break;
		}

		nRet = install_com_component(module_path);

		if (S_OK != nRet)
		{
			break;
		}

		nRet = install_powerpoint_addin();

		if (S_OK != nRet)
		{
			break;
		}

		nRet = install_excel_addin();
		
		if (S_OK != nRet)
		{
			break;
		}

		nRet = install_word_addin();

		if (S_OK != nRet)
		{
			break;
		}

		nRet= install_outlook_addin();

		if (S_OK != nRet)
		{
			break;
		}

	} while (FALSE);

	if(nRet != S_OK)
	{
		DllUnregisterServer();
	}

	return nRet;
}

static HRESULT create_key_with_default_value(
	const HKEY	root,
	const WCHAR *parent, 
	const WCHAR *key, 
	const WCHAR *default_value)
{
	HRESULT nRet = S_OK;

	HKEY hParent = NULL;
	HKEY hKey = NULL;

	do 
	{
		if(ERROR_SUCCESS != RegOpenKeyExW(root,
										  parent,
										  0,
										  KEY_WRITE,
										  &hParent))
		{
			nRet = E_UNEXPECTED;
			break;
		}

		if(ERROR_SUCCESS != RegCreateKey(hParent,
										 key,
										 &hKey))
		{
			nRet = E_UNEXPECTED;
			break;
		}

		if(!default_value)
		{
			break;
		}

		if(ERROR_SUCCESS != RegSetValueExW(hKey,
										   NULL,
										   0,
										   REG_SZ,
										   (const BYTE*)default_value,
										   (DWORD)(wcslen(default_value) + 1)*sizeof(WCHAR)))
		{
			nRet = E_UNEXPECTED;
			break;
		}

	} while (FALSE);

	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	if(hParent)
	{
		RegCloseKey(hParent);
		hParent = NULL;
	}

	return nRet;
}

static HRESULT set_value_content(
	const WCHAR *key, 
	const WCHAR *valuename, 
	const WCHAR *content)
{
	HRESULT nRet = S_OK;

	HKEY hKey = NULL;

	do 
	{
		if(ERROR_SUCCESS != RegOpenKeyExW(HKEY_CLASSES_ROOT,
										  key,
										  0,
										  KEY_SET_VALUE,
										  &hKey))
		{
			nRet = E_UNEXPECTED;
			break;
		}

		if(ERROR_SUCCESS != RegSetValueExW(hKey,
										   valuename,
										   0,
										   REG_SZ,
										   (const BYTE*)content,
										   (DWORD)(wcslen(content)+1)*sizeof(WCHAR)))
		{
			nRet = E_UNEXPECTED;
			break;
		}

	} while (FALSE);

	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return nRet;
}

static HRESULT create_dword_value(
	const HKEY	root,
	const WCHAR	*key,
	const WCHAR	*value_name,
	const DWORD	value)
{
	HRESULT hr = S_OK;

	HKEY hKey = NULL;

	do 
	{
		//
		// the key must has been created in advance
		//
		if(ERROR_SUCCESS != RegOpenKeyExW(root,
										  key,
										  0,
										  KEY_WRITE,
										  &hKey))
		{
			hr = E_UNEXPECTED;
			break;
		}

		if(ERROR_SUCCESS != RegSetValueExW(hKey,
										   value_name,
										   0,
										   REG_DWORD,
										   (const BYTE*)&value,
										   sizeof(value)))
		{
			hr = E_UNEXPECTED;
			break;
		}

	} while (FALSE);

	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return hr;
}

static HRESULT create_sz_value(
	const HKEY	root,
	const WCHAR	*key,
	const WCHAR	*value_name,
	const WCHAR	*value)
{
	HRESULT hr = S_OK;

	HKEY hKey = NULL;

	do 
	{
		//
		// the key must has been created in advance
		//
		if(ERROR_SUCCESS != RegOpenKeyExW(root,
										  key,
										  0,
										  KEY_WRITE,
										  &hKey))
		{
			hr = E_UNEXPECTED;
			break;
		}

		if(ERROR_SUCCESS != RegSetValueExW(hKey,
										   value_name,
										   0,
										   REG_SZ,
										   (const BYTE*)value,
										   (DWORD)(wcslen(value)+1)*sizeof(WCHAR)))
		{
			hr = E_UNEXPECTED;
			break;
		}

	} while (FALSE);

	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return hr;

}

static HRESULT delete_key(const HKEY root, const WCHAR *parent, const WCHAR *key)
{
	HRESULT nRet = S_OK;

	HKEY hKey = NULL;

	do 
	{
		if(ERROR_SUCCESS != RegOpenKeyExW(root,
										  parent,
										  0,
										  DELETE|KEY_SET_VALUE,
										  &hKey))
		{
			nRet = E_UNEXPECTED;
			break;
		}

		if(ERROR_SUCCESS != RegDeleteTreeW(hKey,key))
		{
			nRet = E_UNEXPECTED;
			break;
		}

	} while (FALSE);

	if(hKey)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return nRet;
}

static HRESULT install_com_component(WCHAR *addin_path)
{
	HRESULT hr = S_OK;

	do 
	{
		//
		// install component under CLSID 
		//
		hr = create_key_with_default_value(HKEY_CLASSES_ROOT,
										   L"CLSID",
										   NXRMCOREADDIN_GUID_KEY,
										   NXRMCOREADDIN_NAME);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_key_with_default_value(HKEY_CLASSES_ROOT,
										   NXRMCOREADDIN_INSTALL_CLSID_KEY,
										   L"InprocServer32",
										   addin_path);

		if (S_OK != hr)
		{
			break;
		}

		hr = set_value_content(NXRMCOREADDIN_INSTALL_INPROCSERVER32_KEY,
							   L"ThreadingModel",
							   L"Apartment");

		if (S_OK != hr)
		{
			break;
		}

		//
		// install component under HKEY_CLASSES_ROOT
		//

		hr = create_key_with_default_value(HKEY_CLASSES_ROOT,
										   NULL,
										   NXRMCOREADDIN_NAME,
										   L"nxrmcoreaddin Class");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_key_with_default_value(HKEY_CLASSES_ROOT,
										   NXRMCOREADDIN_NAME,
										   L"CLSID",
										   NXRMCOREADDIN_GUID_KEY);

		if (S_OK != hr)
		{
			break;
		}

	} while (FALSE);

	return hr;
}

static HRESULT install_powerpoint_addin(void)
{
	HRESULT hr = S_OK;

	WCHAR addin_key[260] = {0};

	do 
	{
		memset(addin_key, 0, sizeof(addin_key));

		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 NXRMCOREADDIN_INSTALL_POWERPOINT_ADDIN_KEY);

		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 L"\\");

		wcscat_s((WCHAR*)addin_key,
				 sizeof(addin_key)/sizeof(WCHAR),
				 NXRMCOREADDIN_NAME);

		//
		// create powerpoint addin first in case office is not installed
		//
		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NULL,
										   NXRMCOREADDIN_INSTALL_POWERPOINT_ADDIN_KEY,
										   NULL);
		
		if (S_OK != hr)
		{
			break;
		}

		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NXRMCOREADDIN_INSTALL_POWERPOINT_ADDIN_KEY,
										   NXRMCOREADDIN_NAME,
										   NULL);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_DESCRIPTION_VALUE,
							 L"Enable NextLabs Rights Management service for PowerPoint");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_FRIENDLYNAME_VALUE,
							 L"NextLabs Rights Management for PowerPoint");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_dword_value(HKEY_LOCAL_MACHINE,
								(const WCHAR*)addin_key,
								NXRMCOREADDIN_INSTALL_LOADBEHAVIOR_VALUE,
								3);

		if (S_OK != hr)
		{
			break;
		}

	} while (FALSE);

	return hr;
}

static HRESULT install_excel_addin(void)
{
	HRESULT hr = S_OK;

	WCHAR addin_key[260] = {0};

	do 
	{
		memset(addin_key, 0, sizeof(addin_key));

		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 NXRMCOREADDIN_INSTALL_EXCEL_ADDIN_KEY);

		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 L"\\");

		wcscat_s((WCHAR*)addin_key,
				 sizeof(addin_key)/sizeof(WCHAR),
				 NXRMCOREADDIN_NAME);

		//
		// create excel addin first in case office is not installed
		//
		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NULL,
										   NXRMCOREADDIN_INSTALL_EXCEL_ADDIN_KEY,
										   NULL);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NXRMCOREADDIN_INSTALL_EXCEL_ADDIN_KEY,
										   NXRMCOREADDIN_NAME,
										   NULL);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_DESCRIPTION_VALUE,
							 L"Enable NextLabs Rights Management service for Excel");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_FRIENDLYNAME_VALUE,
							 L"NextLabs Rights Management for Excel");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_dword_value(HKEY_LOCAL_MACHINE,
								(const WCHAR*)addin_key,
								NXRMCOREADDIN_INSTALL_LOADBEHAVIOR_VALUE,
								3);

		if (S_OK != hr)
		{
			break;
		}

	} while (FALSE);

	return hr;

}
static HRESULT install_word_addin(void)
{
	HRESULT hr = S_OK;

	WCHAR addin_key[260] = {0};

	do 
	{
		memset(addin_key, 0, sizeof(addin_key));

		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 NXRMCOREADDIN_INSTALL_WINWORD_ADDIN_KEY);
		
		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 L"\\");

		wcscat_s((WCHAR*)addin_key,
				 sizeof(addin_key)/sizeof(WCHAR),
				 NXRMCOREADDIN_NAME);

		//
		// create word addin first in case office is not installed
		//
		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NULL,
										   NXRMCOREADDIN_INSTALL_WINWORD_ADDIN_KEY,
										   NULL);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NXRMCOREADDIN_INSTALL_WINWORD_ADDIN_KEY,
										   NXRMCOREADDIN_NAME,
										   NULL);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_DESCRIPTION_VALUE,
							 L"Enable NextLabs Rights Management service for Word");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_FRIENDLYNAME_VALUE,
							 L"NextLabs Rights Management for Word");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_dword_value(HKEY_LOCAL_MACHINE,
							    (const WCHAR*)addin_key,
								NXRMCOREADDIN_INSTALL_LOADBEHAVIOR_VALUE,
								3);

		if (S_OK != hr)
		{
			break;
		}

	} while (FALSE);

	return hr;

}
static HRESULT install_outlook_addin(void)
{
	HRESULT hr = S_OK;

	WCHAR addin_key[260] = {0};

	do 
	{
		memset(addin_key, 0, sizeof(addin_key));

		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 NXRMCOREADDIN_INSTALL_OUTLOOK_ADDIN_KEY);
		
		wcscat_s((WCHAR*)addin_key, 
				 sizeof(addin_key)/sizeof(WCHAR), 
				 L"\\");

		wcscat_s((WCHAR*)addin_key,
				 sizeof(addin_key)/sizeof(WCHAR),
				 NXRMCOREADDIN_NAME);

		//
		// create outlook addin first in case office is not installed
		//
		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NULL,
										   NXRMCOREADDIN_INSTALL_OUTLOOK_ADDIN_KEY,
										   NULL);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_key_with_default_value(HKEY_LOCAL_MACHINE,
										   NXRMCOREADDIN_INSTALL_OUTLOOK_ADDIN_KEY,
										   NXRMCOREADDIN_NAME,
										   NULL);

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_DESCRIPTION_VALUE,
							 L"Enable NextLabs Rights Management service for Outlook");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_sz_value(HKEY_LOCAL_MACHINE,
							 (const WCHAR*)addin_key,
							 NXRMCOREADDIN_INSTALL_FRIENDLYNAME_VALUE,
							 L"NextLabs Rights Management for Outlook");

		if (S_OK != hr)
		{
			break;
		}

		hr = create_dword_value(HKEY_LOCAL_MACHINE,
								(const WCHAR*)addin_key,
								NXRMCOREADDIN_INSTALL_LOADBEHAVIOR_VALUE,
								3);

		if (S_OK != hr)
		{
			break;
		}

	} while (FALSE);

	return hr;
}

static HRESULT uninstall_com_component(void)
{
	HRESULT hr = S_OK;

	do 
	{
		hr = delete_key(HKEY_CLASSES_ROOT,
						L"CLSID",
						NXRMCOREADDIN_GUID_KEY);

		hr = delete_key(HKEY_CLASSES_ROOT,
						NULL,
						NXRMCOREADDIN_NAME);

	} while (FALSE);

	return hr;
}

static HRESULT uninstall_powerpoint_addin(void)
{
	HRESULT hr = S_OK;

	do 
	{
		hr = delete_key(HKEY_LOCAL_MACHINE,
						NXRMCOREADDIN_INSTALL_POWERPOINT_ADDIN_KEY,
						NXRMCOREADDIN_NAME);

	} while (FALSE);

	return hr;
}

static HRESULT uninstall_excel_addin(void)
{
	HRESULT hr = S_OK;

	do 
	{
		hr = delete_key(HKEY_LOCAL_MACHINE,
						NXRMCOREADDIN_INSTALL_EXCEL_ADDIN_KEY,
						NXRMCOREADDIN_NAME);

	} while (FALSE);

	return hr;

}

static HRESULT uninstall_word_addin(void)
{
	HRESULT hr = S_OK;

	do 
	{
		hr = delete_key(HKEY_LOCAL_MACHINE,
						NXRMCOREADDIN_INSTALL_WINWORD_ADDIN_KEY,
						NXRMCOREADDIN_NAME);

	} while (FALSE);

	return hr;

}

static HRESULT uninstall_outlook_addin(void)
{
	HRESULT hr = S_OK;

	do 
	{
		hr = delete_key(HKEY_LOCAL_MACHINE,
						NXRMCOREADDIN_INSTALL_OUTLOOK_ADDIN_KEY,
						NXRMCOREADDIN_NAME);

	} while (FALSE);

	return hr;
}
