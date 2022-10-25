#pragma once

#ifndef CACHE_LINE
#define CACHE_LINE	64
#endif

#ifndef CACHE_ALIGN
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))
#endif

class IEncryptionOverlay : public IShellIconOverlayIdentifier
{
public:
	IEncryptionOverlay();
	~IEncryptionOverlay();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppobj);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib);

	STDMETHODIMP GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int * pIndex, DWORD * pdwFlags);

	STDMETHODIMP GetPriority(int * pIPriority);

private:
	ULONG				m_uRefCount;
};