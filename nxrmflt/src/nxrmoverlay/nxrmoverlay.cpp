#include "stdafx.h"
#include "nxrmoverlay.h"
#include "EncryptionOverlay.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

CACHE_ALIGN HMODULE g_hModule = NULL;
CACHE_ALIGN	LONG	g_unxrmOverlayInstanceCount = 0;
CACHE_ALIGN	LONG	g_uEncryptionOverlayInstanceCount = 0;

extern "C" const GUID CLSID_IEncryptionOverlay;


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

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hModule = hModule;

			DisableThreadLibraryCalls(hModule);
			break;
		}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

InxrmOverlay::InxrmOverlay()
{
	m_uRefCount		= 1;
	m_uLockCount	= 0;
}

InxrmOverlay::~InxrmOverlay()
{

}

STDMETHODIMP InxrmOverlay::QueryInterface(REFIID riid, void **ppobj)
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

STDMETHODIMP InxrmOverlay::CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject)
{
	HRESULT hr = S_OK;

	IEncryptionOverlay *p = NULL;

	do 
	{
		if(pUnkOuter)
		{
			*ppvObject = NULL;
			hr = CLASS_E_NOAGGREGATION;
			break;
		}

		p = new IEncryptionOverlay;

		if(!p)
		{
			*ppvObject = NULL;
			hr = E_OUTOFMEMORY;
			break;
		}

		InterlockedIncrement(&g_uEncryptionOverlayInstanceCount);

		hr = p->QueryInterface(riid,ppvObject);

		p->Release();

	} while (FALSE);

	return hr;
}

STDMETHODIMP InxrmOverlay::LockServer(BOOL fLock)
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

STDMETHODIMP_(ULONG) InxrmOverlay::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) InxrmOverlay::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount && (m_uLockCount == 0))
	{
		delete this;
		InterlockedDecrement(&g_unxrmOverlayInstanceCount);
	}

	return uCount;
}
_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv)
{
	HRESULT  hr = E_OUTOFMEMORY;

	InxrmOverlay *InxrmOverlayInstance = NULL;

	if(IsEqualCLSID(rclsid, CLSID_IEncryptionOverlay))
	{
		InxrmOverlayInstance = new InxrmOverlay;

		if(InxrmOverlayInstance)
		{
			InterlockedIncrement(&g_unxrmOverlayInstanceCount);

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
	if(g_unxrmOverlayInstanceCount == 0 && g_uEncryptionOverlayInstanceCount == 0)
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

	h1 = delete_key(HKEY_CLASSES_ROOT, NXRMOVERLAY_INSTALL_CLSID_KEY, L"InprocServer32");
	h2 = delete_key(HKEY_CLASSES_ROOT, L"CLSID", NXRMOVERLAY_INSTALL_GUID_KEY);
	h3 = delete_key(HKEY_LOCAL_MACHINE, NXRMOVERLAY_INSTALL_SHELLOVERLAY_KEY, NXRMOVERLAY_INSTALL_NAME);

	if(h1 != S_OK || h2 != S_OK || h3 != S_OK)
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
		if(!GetModuleFileNameW(g_hModule,
			module_path,
			sizeof(module_path)/sizeof(WCHAR)))
		{
			nRet = E_UNEXPECTED;
			break;
		}


		nRet = create_key_with_default_value(HKEY_CLASSES_ROOT,
											 L"CLSID",
											 NXRMOVERLAY_INSTALL_GUID_KEY,
											 NXRMOVERLAY_NAME);

		if(S_OK != nRet)
		{
			break;
		}

		nRet = create_key_with_default_value(HKEY_CLASSES_ROOT,
											 NXRMOVERLAY_INSTALL_CLSID_KEY,
											 L"InprocServer32",
											 module_path);

		if(S_OK != nRet)
		{
			break;
		}

		nRet = set_value_content(NXRMOVERLAY_INSTALL_INPROCSERVER32_KEY,
								 L"ThreadingModel",
								 L"Apartment");

		if(S_OK != nRet)
		{
			break;
		}

		nRet = create_key_with_default_value(HKEY_LOCAL_MACHINE,
											 NXRMOVERLAY_INSTALL_SHELLOVERLAY_KEY,
											 NXRMOVERLAY_INSTALL_NAME,
											 NXRMOVERLAY_INSTALL_GUID_KEY);

		if(S_OK != nRet)
		{
			break;
		}

		//
		// notify Shell to load us
		//
		SHLoadNonloadedIconOverlayIdentifiers();

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

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
