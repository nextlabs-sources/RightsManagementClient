#include "stdafx.h"
#include "string"
#include "EncryptionOverlay.h"

extern CACHE_ALIGN HMODULE g_hModule;
extern CACHE_ALIGN LONG	g_uEncryptionOverlayInstanceCount;

// {49732CC5-3666-4F37-886D-6E4BC1C6B262}
extern "C" const GUID CLSID_IEncryptionOverlay		= {0x49732cc5, 0x3666, 0x4f37, {0x88, 0x6d, 0x6e, 0x4b, 0xc1, 0xc6, 0xb2, 0x62}};

IEncryptionOverlay::IEncryptionOverlay()
{
	m_uRefCount = 1;
}

IEncryptionOverlay::~IEncryptionOverlay()
{

}

STDMETHODIMP IEncryptionOverlay::QueryInterface(REFIID riid, void **ppobj)
{
	HRESULT hRet = S_OK;

	IUnknown *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if((IID_IUnknown == riid) || (IID_IShellIconOverlayIdentifier == riid))
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

STDMETHODIMP_(ULONG) IEncryptionOverlay::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) IEncryptionOverlay::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount)
	{
		delete this;
		InterlockedDecrement(&g_uEncryptionOverlayInstanceCount);
	}

	return uCount;
}

STDMETHODIMP IEncryptionOverlay::IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib)
{
	HRESULT nRet = S_FALSE;

	HANDLE hFile = INVALID_HANDLE_VALUE;

	std::wstring file_name = L"";

	//if(dwAttrib & FILE_ATTRIBUTE_ENCRYPTED)
	{
		file_name = (std::wstring)pwszPath + L".nxl";
		hFile = CreateFileW(file_name.c_str(),
			GENERIC_READ,
							FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			nRet = S_OK;
		}
	}

	return nRet;
}

STDMETHODIMP IEncryptionOverlay::GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int * pIndex, DWORD * pdwFlags)
{
	HRESULT nRet = S_OK;

	if(!GetModuleFileNameW(g_hModule,
						   pwszIconFile,
						   cchMax))
	{
		nRet = S_FALSE;
	}
	else
	{
		*pIndex = 0;
		*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
	}

	return nRet;
}

STDMETHODIMP IEncryptionOverlay::GetPriority(int * pIPriority)
{
	*pIPriority = 0;

	return S_OK;
}
