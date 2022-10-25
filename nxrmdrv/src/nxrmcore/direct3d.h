#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <DXGI1_2.h>

#define DIRECT3D_CREATEDXGIFACTORY1_PROC_NAME				"CreateDXGIFactory1"
#define DIRECT3D_CREATEDXGIFACTORY_PROC_NAME				"CreateDXGIFactory"
#define DIRECT3D_D2D1CREATEFACTORY_PROC_NAME				"D2D1CreateFactory"
#define DIRECT3D_COCREATEINSTANCE_PROC_NAME					"CoCreateInstance"
#define DIRECT3D_DIRECT3DCREATE9_PROC_NAME					"Direct3DCreate9"
#define DIRECT3D_D3D10CREATEDEVICE1_PROC_NAME				"D3D10CreateDevice1"
#define DIRECT3D_WICCREATEIMAGINGFACTORY_PROXY_PROC_NAME	"WICCreateImagingFactory_Proxy"
#define DIRECT3D_GETREGIONDATA_PROC_NAME					"GetRegionData"
#define DIRECT3D_D3D10CREATESTATEBLOCK_PROC_NAME			"D3D10CreateStateBlock"
#define DIRECT3D_D3D10STATEBLOCKMASKENABLEALL_PROC_NAME		"D3D10StateBlockMaskEnableAll"
#define DIRECT3D_GETWINDOWRECT_PROC_NAME					"GetWindowRect"
#define DIRECT3D_POSTMESSAGE_PROC_NAME						"PostMessageW"
#define DIRECT3D_GETWINDOWTHREADPROCESSID_PROC_NAME			"GetWindowThreadProcessId"
#define DIRECT3D_REDRAWWINDOW_PROC_NAME						"RedrawWindow"

#ifdef _AMD64_

#define DIRECT3D_GETWINDOWLONGPTRW_PROC_NAME				"GetWindowLongPtrW"

#else

#define DIRECT3D_GETWINDOWLONGW_PROC_NAME					"GetWindowLongW"

#endif

typedef HRESULT (WINAPI *CREATEDXGIFACTORY1)(REFIID riid, void **ppFactory);

HRESULT WINAPI Core_CreateDXGIFactory1(REFIID riid, void **ppFactory);

struct IDXGISwapChainDWM : public IDXGIDeviceSubObject
{
	STDMETHOD(Present)(UINT SyncInterval, UINT Flags) = 0;
	STDMETHOD(GetBuffer)(UINT Buffer, REFIID riid, void **ppSurface) = 0;
	STDMETHOD(GetDesc)(DXGI_SWAP_CHAIN_DESC *pDesc) = 0;
	STDMETHOD(ResizeBuffers)(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) = 0;
	STDMETHOD(ResizeTarget)(const DXGI_MODE_DESC *pNewTargetParameters) = 0;
	STDMETHOD(GetContainingOutput)(IDXGIOutput **ppOutput) = 0;
	STDMETHOD(GetFrameStatistics)(DXGI_FRAME_STATISTICS *pStats) = 0;
	STDMETHOD(GetLastPresentCount)(UINT *pLastPresentCount) = 0;
	STDMETHOD(SetFullscreenState)(BOOL Fullscreen, IDXGIOutput *pTarget) = 0;
	STDMETHOD(GetFullscreenState)(BOOL *pFullscreen, IDXGIOutput **ppTarget) = 0;
};

class CoreIDXGISwapChainDWM : public IDXGISwapChainDWM
{
public:
	CoreIDXGISwapChainDWM();
	CoreIDXGISwapChainDWM(IDXGISwapChainDWM *pIDXGISwapChainDWM);
	~CoreIDXGISwapChainDWM();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppobj);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP SetPrivateData(
		REFGUID Name,
		UINT DataSize,
		const void *pData);

	STDMETHODIMP SetPrivateDataInterface(
		REFGUID Name,
		const IUnknown *pUnknown);

	STDMETHODIMP GetPrivateData(
		REFGUID Name,
		UINT*pDataSize,
		void *pData);

	STDMETHODIMP GetParent(
		REFIID riid,
		void **ppParent);

	STDMETHODIMP GetDevice(
		REFIID riid,
		void **ppDevice);

	STDMETHODIMP Present(
		UINT SyncInterval,
		UINT Flags);

	STDMETHODIMP GetBuffer(
		UINT Buffer,
		REFIID riid,
		void **ppSurface);

	STDMETHODIMP SetFullscreenState(
		BOOL Fullscreen,
		IDXGIOutput *pTarget);

	STDMETHODIMP GetFullscreenState(
		BOOL *pFullscreen,
		IDXGIOutput **ppTarget);

	STDMETHODIMP GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc);

	STDMETHODIMP ResizeBuffers(
		UINT BufferCount,
		UINT Width,
		UINT Height,
		DXGI_FORMAT NewFormat,
		UINT SwapChainFlags);

	STDMETHODIMP ResizeTarget(const DXGI_MODE_DESC *pNewTargetParameters);

	STDMETHODIMP GetContainingOutput(IDXGIOutput **ppOutput);

	STDMETHODIMP GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats);

	STDMETHODIMP GetLastPresentCount(UINT *pLastPresentCount);

private:

	ULONG				m_uRefCount;
	IDXGISwapChainDWM	*m_pIDXGISwapChainDWM;
};


struct IDXGISwapChainDWM1 : public IDXGIObject
{
	STDMETHOD(GetDevice)(REFIID riid, void **ppvObject) = 0;

	STDMETHOD(Present)(_In_  UINT SyncInterval, _In_  UINT Flags) = 0;

	STDMETHOD(GetBuffer)(_In_  UINT Buffer,	_In_  REFIID riid, _Out_  void **ppSurface) = 0;

	STDMETHOD(GetDesc)(_Out_  DXGI_SWAP_CHAIN_DESC *pDesc) = 0;

	STDMETHOD(ResizeBuffers)(_In_ UINT BufferCount,	_In_ UINT Width, _In_ UINT Height, _In_ DXGI_FORMAT NewFormat, _In_ UINT SwapChainFlags) = 0;

	STDMETHOD(ResizeTarget)(_In_  const DXGI_MODE_DESC *pNewTargetParameters) = 0;

	STDMETHOD(GetContainingOutput)(_Out_ IDXGIOutput1 **ppOutput) = 0;

	STDMETHOD(GetFrameStatistics)(_Out_ DXGI_FRAME_STATISTICS *pStats) = 0;

	STDMETHOD(GetLastPresentCount)(_Out_ UINT *pLastPresentCount) = 0;

	STDMETHOD(Present1)(_In_  UINT SyncInterval, _In_  UINT Flags, _In_  UINT  DirtyRectsCount, _In_  RECT  *pDirtyRects, _In_  RECT  *pScrollRect,	_In_  POINT *pScrollOffset) = 0;

	STDMETHOD(GetLogicalSurfaceHandle)(LPVOID pVoid) = 0;

	STDMETHOD(CheckDirectFlipSupport)(ULONG uLong, IDXGIResource *pResource, PULONG puLong) = 0;

	STDMETHOD(Present2)(_In_  UINT SyncInterval, _In_  UINT Flags, _In_  UINT  DirtyRectsCount,	_In_  RECT  *pDirtyRects, _In_  RECT  *pScrollRect,	_In_  POINT *pScrollOffset,	_In_  IDXGIResource * pResource) = 0;

	STDMETHOD(GetCompositionSurface)(LPVOID* ppVoid) = 0;
};

typedef HRESULT (WINAPI *WICCREATEIMAGINGFACTORY_PROXY)(
	_In_  UINT               SDKVersion,
	_Out_ IWICImagingFactory **ppIImagingFactory
	);

HRESULT WINAPI Core_WICCreateImagingFactory_Proxy(
	_In_  UINT               SDKVersion,
	_Out_ IWICImagingFactory **ppIImagingFactory
	);

typedef DWORD(WINAPI *GETREGIONDATA)(_In_ HRGN hrgn, _In_ DWORD nCount, _Out_writes_bytes_to_opt_(nCount, return) LPRGNDATA lpRgnData);

DWORD WINAPI Core_GetRegionData(
	_In_ HRGN											hrgn,
	_In_ DWORD											nCount,
	_Out_writes_bytes_to_opt_(nCount, return) LPRGNDATA lpRgnData);

DWORD WINAPI Core_GetRegionData1(
	_In_ HRGN											hrgn,
	_In_ DWORD											nCount,
	_Out_writes_bytes_to_opt_(nCount, return) LPRGNDATA lpRgnData);

typedef HRESULT(WINAPI *CREATEDXGIFACTORY)(REFIID riid, void **ppFactory);

HRESULT WINAPI Core_CreateDXGIFactory(REFIID riid, void **ppFactory);

typedef HRESULT (WINAPI *IDXGIFACTORYDWM_CREATESWAPCHAINDWM)(
	_In_  IDXGIFactory	*pIDXGIFactory,
	_In_  IUnknown *pDevice,
	_In_  DXGI_SWAP_CHAIN_DESC *pDesc,
	_In_  IDXGIOutput *pOutput,
	_Out_  IDXGISwapChainDWM **ppSwapChainDWM);

HRESULT WINAPI Core_CreateSwapChainDWM(
	_In_  IDXGIFactory	*pIDXGIFactory,
	_In_  IUnknown *pDevice,
	_In_  DXGI_SWAP_CHAIN_DESC *pDesc,
	_In_  IDXGIOutput *pOutput,
	_Out_  IDXGISwapChainDWM **ppSwapChainDWM);

typedef HRESULT (WINAPI *IDXGIFACTORYDWM1_CREATESWAPCHAINDWM)(
	_In_ IDXGIFactory1	*pIDXGIFactory1,
	_In_ IUnknown *pDevice,
	_In_ DXGI_SWAP_CHAIN_DESC1 *pSwapChainDesc1,
	_In_ DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pSwapChainFullScreenDesc1,
	_In_ IDXGIOutput * pOutput,
	_Out_ IDXGISwapChainDWM1 ** ppSwapChainDWM1
	);

HRESULT WINAPI Core_CreateSwapChainDWM1(
	_In_ IDXGIFactory1	*pIDXGIFactory1,
	_In_ IUnknown *pDevice,
	_In_ DXGI_SWAP_CHAIN_DESC1 *pSwapChainDesc1,
	_In_ DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pSwapChainFullScreenDesc1,
	_In_ IDXGIOutput * pOutput,
	_Out_ IDXGISwapChainDWM1 ** ppSwapChainDWM1
	);

typedef HRESULT (WINAPI *IDXGISWAPCHAINDWM1_PRESENTDWM)(
	_In_ IDXGISwapChainDWM1 *pSwapChainDWM1, 
	_In_ UINT SyncInterval, 
	_In_ UINT Flags, 
	_In_ UINT  DirtyRectsCount, 
	_In_ RECT  *pDirtyRects, 
	_In_ RECT  *pScrollRect, 
	_In_ POINT *pScrollOffset,
	_In_ IDXGIResource *pResource,
	_In_ PVOID p);

HRESULT WINAPI Core_IDXGISwapChainDWM1_PresentDWM(
	_In_ IDXGISwapChainDWM1 *pSwapChainDWM1,
	_In_ UINT SyncInterval,
	_In_ UINT Flags,
	_In_ UINT  DirtyRectsCount,
	_In_ RECT  *pDirtyRects,
	_In_ RECT  *pScrollRect,
	_In_ POINT *pScrollOffset,
	_In_ IDXGIResource *pResource,
	_In_ PVOID p);

HRESULT WINAPI CreateD3D10TextureFromFile(
	_In_ ID3D10Device1* d3dDevice,
	_In_z_ const WCHAR* FileName,
	_Out_opt_ ID3D10Resource** texture,
	_Out_opt_ ID3D10ShaderResourceView** textureView,
	_Out_opt_ ULONG *Width,
	_Out_opt_ ULONG *Height);

typedef HRESULT (WINAPI *D3D10STATEBLOCKMASKENABLEALL)(
	_Out_ D3D10_STATE_BLOCK_MASK *pMask
	);

typedef HRESULT (WINAPI *D3D10CREATESTATEBLOCK)(
	_In_  ID3D10Device           *pDevice,
	_In_  D3D10_STATE_BLOCK_MASK *pStateBlockMask,
	_Out_ ID3D10StateBlock       **ppStateBlock
	);

typedef BOOL (WINAPI *DIRECT3D_GETWINDOWRECT)(
	_In_ HWND hWnd,
	_Out_ LPRECT lpRect);

typedef BOOL(WINAPI *DIRECT3D_POSTMESSAGEW)(
	__in_opt	HWND	hWnd,
	__in		UINT	Msg,
	__in		WPARAM	wParam,
	__in		LPARAM	lParam);

typedef DWORD (WINAPI *DIRECT3D_GETWINDOWTHREADPROCESSID)(
	_In_      HWND    hWnd,
	_Out_opt_ LPDWORD lpdwProcessId
	);

typedef BOOL (WINAPI *DIRECT3D_REDRAWWINDOW)(
	_In_       HWND hWnd,
	_In_ const RECT *lprcUpdate,
	_In_       HRGN hrgnUpdate,
	_In_       UINT flags
	);

#ifdef _AMD64_

typedef LONG_PTR (WINAPI *DIRECT3D_GETWINDOWLONGPTRW)(
	_In_ HWND hWnd,
	_In_ int  nIndex
	);

#else

typedef LONG (WINAPI *DIRECT3D_GETWINDOWLONGW)(
	_In_ HWND hWnd,
	_In_ int  nIndex
	);

#endif
BOOL WINAPI Core_PostMessageW(
	__in_opt	HWND	hWnd,
	__in		UINT	Msg,
	__in		WPARAM	wParam,
	__in		LPARAM	lParam);

BOOL InitializeDWMHook(void);
void CleanupDWMHook(void);

BOOL InitializedWindowsCodesHook(void);
void CleanupWindowsCodesHook(void);