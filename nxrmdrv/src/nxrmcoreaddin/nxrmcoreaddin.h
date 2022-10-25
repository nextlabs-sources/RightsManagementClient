// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NXRMCOREADDIN_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NXRMCOREADDIN_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NXRMCOREADDIN_EXPORTS
#define NXRMCOREADDIN_API __declspec(dllexport)
#else
#define NXRMCOREADDIN_API __declspec(dllimport)
#endif

#ifndef CACHE_LINE
#define CACHE_LINE	64
#endif

#ifndef CACHE_ALIGN
#define CACHE_ALIGN	__declspec(align(CACHE_LINE))
#endif

#define NXRMCOREADDIN_NAME							L"NxRMAddin"
#define NXRMCOREADDIN_GUID_KEY						L"{0CCA3189-F325-4D58-AB6D-212CD76C3311}"
#define NXRMCOREADDIN_INSTALL_CLSID_KEY				L"CLSID\\{0CCA3189-F325-4D58-AB6D-212CD76C3311}"
#define NXRMCOREADDIN_INSTALL_INPROCSERVER32_KEY	L"CLSID\\{0CCA3189-F325-4D58-AB6D-212CD76C3311}\\InprocServer32"
#define NXRMCOREADDIN_INSTALL_NXRMCOREADDIN_KEY		L"NxRMAddin\\CLSID"
#define NXRMCOREADDIN_INSTALL_POWERPOINT_ADDIN_KEY	L"Software\\Microsoft\\Office\\PowerPoint\\Addins"
#define NXRMCOREADDIN_INSTALL_WINWORD_ADDIN_KEY		L"Software\\Microsoft\\Office\\Word\\Addins"
#define NXRMCOREADDIN_INSTALL_EXCEL_ADDIN_KEY		L"Software\\Microsoft\\Office\\Excel\\Addins"
#define NXRMCOREADDIN_INSTALL_OUTLOOK_ADDIN_KEY		L"Software\\Microsoft\\Office\\Outlook\\Addins"
#define NXRMCOREADDIN_INSTALL_FRIENDLYNAME_VALUE	L"FriendlyName"
#define NXRMCOREADDIN_INSTALL_LOADBEHAVIOR_VALUE	L"LoadBehavior"
#define NXRMCOREADDIN_INSTALL_DESCRIPTION_VALUE		L"Description"

class Inxrmcoreaddin : public IClassFactory
{
public:
	Inxrmcoreaddin();
	~Inxrmcoreaddin();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppobj);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject);

	STDMETHODIMP LockServer(BOOL fLock);

private:
	ULONG				m_uRefCount;
	ULONG				m_uLockCount;
};

