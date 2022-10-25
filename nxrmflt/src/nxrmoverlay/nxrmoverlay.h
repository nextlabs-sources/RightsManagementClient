#pragma once

#ifndef CACHE_LINE
#define CACHE_LINE	64
#endif

#ifndef CACHE_ALIGN
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))
#endif

#define NXRMOVERLAY_NAME							L"nxrmOverlay"
#define NXRMOVERLAY_INSTALL_NAME					L"        AAAnxrmOverlay"
#define NXRMOVERLAY_INSTALL_GUID_KEY				L"{49732CC5-3666-4F37-886D-6E4BC1C6B262}"
#define NXRMOVERLAY_INSTALL_CLSID_KEY				L"CLSID\\{49732CC5-3666-4F37-886D-6E4BC1C6B262}"
#define NXRMOVERLAY_INSTALL_INPROCSERVER32_KEY		L"CLSID\\{49732CC5-3666-4F37-886D-6E4BC1C6B262}\\InprocServer32"
#define NXRMOVERLAY_INSTALL_SHELLOVERLAY_KEY		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers"

class InxrmOverlay : public IClassFactory
{
public:
	InxrmOverlay();
	~InxrmOverlay();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppobj);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject);

	STDMETHODIMP LockServer(BOOL fLock);

private:
	ULONG				m_uRefCount;
	ULONG				m_uLockCount;
};
