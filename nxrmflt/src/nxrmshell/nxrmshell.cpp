// nxrmctxmenu.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "nxrmshell.h"
#include "ContextMenu.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "nxrmshellglobal.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	SHELL_GLOBAL_DATA Global;

	BOOL init_rm_section_safe(void);

#ifdef __cplusplus
}
#endif

extern "C" const GUID CLSID_IRmCtxMenu;


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

static HRESULT RegisterFileType(_In_ LPCWSTR modpath);

#define NXL_FILETYPE_KEY        L".nxl"
#define NXL_FILETYPE_APPKEY     L"NextLabs.Handler.1"

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			Global.hModule = hModule;

			InitializeCriticalSection(&Global.SectionLock);

			DisableThreadLibraryCalls(hModule);

			break;
		}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			DeleteCriticalSection(&Global.SectionLock);
		}

		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

Inxrmshell::Inxrmshell()
{
	m_uRefCount		= 1;
	m_uLockCount	= 0;
}

Inxrmshell::~Inxrmshell()
{

}

STDMETHODIMP Inxrmshell::QueryInterface(REFIID riid, void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if (IID_IClassFactory == riid)
		{
			punk = (ICallFactory *)this;
		}
		else if (IID_IUnknown == riid)
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

STDMETHODIMP Inxrmshell::CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject)
{
	HRESULT hr = S_OK;

	IRmCtxMenu *p = NULL;

	do 
	{
		if(pUnkOuter)
		{
			*ppvObject = NULL;
			hr = CLASS_E_NOAGGREGATION;
			break;
		}

		p = new IRmCtxMenu;

		if(!p)
		{
			*ppvObject = NULL;
			hr = E_OUTOFMEMORY;
			break;
		}

		InterlockedIncrement(&Global.ContextMenuInstanceCount);

		hr = p->QueryInterface(riid,ppvObject);

		p->Release();

	} while (FALSE);

	return hr;
}

STDMETHODIMP Inxrmshell::LockServer(BOOL fLock)
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

STDMETHODIMP_(ULONG) Inxrmshell::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) Inxrmshell::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount && (m_uLockCount == 0))
	{
		delete this;
		InterlockedDecrement(&Global.nxrmshellInstanceCount);
	}

	return uCount;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	HRESULT  hr = E_OUTOFMEMORY;

	Inxrmshell *Inxrmshellnstance = NULL;

	if(IsEqualCLSID(rclsid, CLSID_IRmCtxMenu))
	{
		Inxrmshellnstance = new Inxrmshell;

		if(Inxrmshellnstance)
		{
			InterlockedIncrement(&Global.nxrmshellInstanceCount);

			hr = Inxrmshellnstance->QueryInterface(riid,ppv);

			Inxrmshellnstance->Release();
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
	if(Global.nxrmshellInstanceCount == 0 && Global.ContextMenuInstanceCount == 0)
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
	HRESULT h1 = S_OK;
	HRESULT h2 = S_OK;
	HRESULT h3 = S_OK;
	HRESULT h4 = S_OK;

	h1 = delete_key(HKEY_CLASSES_ROOT, NXRMSHELL_INSTALL_CLSID_KEY, L"InprocServer32");
	h2 = delete_key(HKEY_CLASSES_ROOT, L"CLSID", NXRMSHELL_INSTALL_GUID_KEY);
	h3 = delete_key(HKEY_CLASSES_ROOT, NXRMSHELL_INSTALL_CONTEXTMENUHANDLERS_KEY, NXRMSHELL_NAME);
	h4 = delete_key(HKEY_CLASSES_ROOT, NXRMSHELL_INSTALL_DIR_CONTEXTMENUHANDLERS_KEY, NXRMSHELL_NAME);

    ::RegDeleteKeyW(HKEY_CLASSES_ROOT, NXL_FILETYPE_KEY);
    delete_key(HKEY_CLASSES_ROOT, L"NextLabs.Handler.1\\shell\\open", L"command");
    delete_key(HKEY_CLASSES_ROOT, L"NextLabs.Handler.1\\shell", L"open");
    delete_key(HKEY_CLASSES_ROOT, L"NextLabs.Handler.1", L"shell");
    delete_key(HKEY_CLASSES_ROOT, L"NextLabs.Handler.1", L"DefaultIcon");
    ::RegDeleteKeyW(HKEY_CLASSES_ROOT, NXL_FILETYPE_APPKEY);

	if(h1 != S_OK || h2 != S_OK || h3 != S_OK || h4!= S_OK)
	{
		return E_UNEXPECTED;
	}
	else
	{
		return S_OK;
	}
}

STDAPI DllRegisterServer(void)
{
	HRESULT nRet = S_OK;

	WCHAR module_path[MAX_PATH];

	memset(module_path,0,sizeof(module_path));

	do 
	{
		if(!GetModuleFileNameW(Global.hModule,
							   module_path,
							   sizeof(module_path)/sizeof(WCHAR)))
		{
			nRet = E_UNEXPECTED;
			break;
		}


		nRet = create_key_with_default_value(HKEY_CLASSES_ROOT,
											 L"CLSID",
											 NXRMSHELL_INSTALL_GUID_KEY,
											 NXRMSHELL_NAME);

		if(S_OK != nRet)
		{
			break;
		}

		nRet = create_key_with_default_value(HKEY_CLASSES_ROOT,
											 NXRMSHELL_INSTALL_CLSID_KEY,
											 L"InprocServer32",
											 module_path);

		if(S_OK != nRet)
		{
			break;
		}

		nRet = set_value_content(NXRMSHELL_INSTALL_INPROCSERVER32_KEY,
								 L"ThreadingModel",
								 L"Apartment");

		if(S_OK != nRet)
		{
			break;
		}

		nRet = create_key_with_default_value(HKEY_CLASSES_ROOT,
											 NXRMSHELL_INSTALL_CONTEXTMENUHANDLERS_KEY,
											 NXRMSHELL_NAME,
											 NXRMSHELL_INSTALL_GUID_KEY);

		if(S_OK != nRet)
		{
			break;
		}

		nRet = create_key_with_default_value(HKEY_CLASSES_ROOT,
			NXRMSHELL_INSTALL_DIR_CONTEXTMENUHANDLERS_KEY,
			NXRMSHELL_NAME,
			NXRMSHELL_INSTALL_GUID_KEY);

		if (S_OK != nRet)
		{
			break;
		}

        nRet = RegisterFileType(module_path);
		if(S_OK != nRet)
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

static HRESULT RegisterFileType(_In_ LPCWSTR modpath)
{
	HKEY hKey = NULL;
	HKEY hKeyShell = NULL;
	HKEY hKeyOpen = NULL;
	HKEY hKeyCmd = NULL;
	HKEY hKeyIcon = NULL;
    DWORD dwDisposition = 0;

    std::wstring wsCmdKeyName(NXL_FILETYPE_APPKEY);
    std::wstring wsCmd;
    std::wstring wsIcon;

    wsCmd = L"rundll32.exe \"";
    wsCmd += modpath;
    wsCmd += L"\",OpenRemoteFileWithCache %1";

    wsIcon = L"\"";
    wsIcon += modpath;
    wsIcon += L"\",0";

    if(0 != ::RegCreateKeyExW(HKEY_CLASSES_ROOT, NXL_FILETYPE_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if(0 != RegSetValueExW(hKey, NULL, 0, REG_SZ, (const BYTE*)wsCmdKeyName.c_str(), (DWORD)(wsCmdKeyName.length() + 1)*sizeof(WCHAR))) {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    RegCloseKey(hKey);
    hKey = NULL;

    if(0 != ::RegCreateKeyExW(HKEY_CLASSES_ROOT, wsCmdKeyName.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if(0 != ::RegCreateKeyExW(hKey, L"shell", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyShell, &dwDisposition)) {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if(0 != ::RegCreateKeyExW(hKeyShell, L"open", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyOpen, &dwDisposition)) {
        RegCloseKey(hKeyShell);
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if(0 != ::RegCreateKeyExW(hKeyOpen, L"command", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyCmd, &dwDisposition)) {
        RegCloseKey(hKeyOpen);
        RegCloseKey(hKeyShell);
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if(0 != RegSetValueExW(hKeyCmd, NULL, 0, REG_SZ, (const BYTE*)wsCmd.c_str(), (DWORD)(wsCmd.length() + 1)*sizeof(WCHAR))) {
        RegCloseKey(hKeyCmd);
        RegCloseKey(hKeyOpen);
        RegCloseKey(hKeyShell);
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    // Set Icon
    if(0 != ::RegCreateKeyExW(hKey, L"DefaultIcon", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyIcon, &dwDisposition)) {
        RegCloseKey(hKeyCmd);
        RegCloseKey(hKeyOpen);
        RegCloseKey(hKeyShell);
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    if(0 != RegSetValueExW(hKeyIcon, NULL, 0, REG_SZ, (const BYTE*)wsIcon.c_str(), (DWORD)(wsIcon.length() + 1)*sizeof(WCHAR))) {
        RegCloseKey(hKeyIcon);
        RegCloseKey(hKeyCmd);
        RegCloseKey(hKeyOpen);
        RegCloseKey(hKeyShell);
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    RegCloseKey(hKeyIcon);
    RegCloseKey(hKeyCmd);
    RegCloseKey(hKeyOpen);
    RegCloseKey(hKeyShell);
    RegCloseKey(hKey);
    return S_OK;
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

static HRESULT delete_key(const HKEY root, const WCHAR *parent, const WCHAR *key)
{
	HRESULT nRet = S_OK;

	HKEY hKey = NULL;

	do 
	{
		if(ERROR_SUCCESS != RegOpenKeyExW(root,
										  parent,
										  0,
										  DELETE,
										  &hKey))
		{
			nRet = E_UNEXPECTED;
			break;
		}

		if(ERROR_SUCCESS != RegDeleteKeyW(hKey,key))
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

BOOL init_rm_section_safe(void)
{
	BOOL bRet = FALSE;

	do 
	{
		if(!Global.Section)
		{
			EnterCriticalSection(&Global.SectionLock);

			Global.Section = init_transporter_client();

			LeaveCriticalSection(&Global.SectionLock);
		}

		if(!Global.Section)
		{
			break;
		}

		bRet = is_transporter_enabled(Global.Section);

	} while (FALSE);

	return bRet;
}