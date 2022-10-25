#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "direct3d.h"
#include "d3d11.h"
#include "PixelShader.h"
#include "VertexShader.h"
#include <directxmath.h>
#include "d3dstate.h"
#include "PixelShader1.h"

#ifndef MAX_ULONGLONG
#define MAX_ULONGLONG                   0xFFFFFFFFFFFFFFFF
#endif

#ifdef __cplusplus
extern "C" {
#endif

	extern CORE_GLOBAL_DATA		Global;

	const IID IID_IDXGISwapChain1 = {0x790a45f7,0x0d42,0x4876,0x98,0x3a,0x0a,0x55,0xcf,0xe6,0xf4,0xaa};

	const IID IID_IDXGIFactoryDWM = {0x713f394e,0x92ca,0x47e7,0xab,0x81,0x11,0x59,0xc2,0x79,0x1e,0x54};

	const IID IID_IDXGIFactoryDWM1 = {0x1ddd77aa,0x9a4a,0x4cc8,0x9e,0x55,0x98,0xc1,0x96,0xba,0xfc,0x8f};

#ifdef __cplusplus
}
#endif

static CREATEDXGIFACTORY					g_fnorg_CreateDXGIFactory = NULL;
static CREATEDXGIFACTORY					g_fn_CreateDXGIFactory_trampoline = NULL;
static CREATEDXGIFACTORY1					g_fnorg_CreateDXGIFactory1 = NULL;
static CREATEDXGIFACTORY1					g_fn_CreateDXGIFactory1_trampoline = NULL;

static IDXGIFACTORYDWM_CREATESWAPCHAINDWM	g_fnorg_IDXGIFactoryDWM_CreateSwapChain = NULL;
static IDXGIFACTORYDWM_CREATESWAPCHAINDWM	g_fn_IDXGIFactoryDWM_CreateSwapChain_trampoline = NULL;

static IDXGIFACTORYDWM1_CREATESWAPCHAINDWM	g_fnorg_IDXGIFactoryDWM1_CreateSwapChain = NULL;
static IDXGIFACTORYDWM1_CREATESWAPCHAINDWM	g_fn_IDXGIFactoryDWM1_CreateSwapChain_trampoline = NULL;

static IDXGISWAPCHAINDWM1_PRESENTDWM		g_fnorg_IDXGISwapChainDWM1_PresentDWM = NULL;
static IDXGISWAPCHAINDWM1_PRESENTDWM		g_fn_IDXGISwapChainDWM1_PresentDWM_trampoline = NULL;

static DIRECT3D_GETWINDOWRECT				g_fnorg_GetWindowRect = NULL;
static DIRECT3D_GETWINDOWTHREADPROCESSID	g_fnorg_GetWindowThreadProcessId = NULL;

#ifdef _AMD64_
static DIRECT3D_GETWINDOWLONGPTRW			g_fnorg_GetWindowLongPtrW = NULL;
#else
static DIRECT3D_GETWINDOWLONGW				g_fnorg_GetWindowLongW = NULL;
#endif

static DIRECT3D_REDRAWWINDOW			g_fnorg_RedrawWindow = NULL;
static DIRECT3D_POSTMESSAGEW			g_fnorg_PostMessageW = NULL;
static DIRECT3D_POSTMESSAGEW			g_fn_PostMessageW_trampoline = NULL;

static WICCREATEIMAGINGFACTORY_PROXY	g_fnorg_WICCreateImagingFactory_Proxy = NULL;
static WICCREATEIMAGINGFACTORY_PROXY	g_fn_WICCreateImagingFactory_Proxy_trampoline = NULL;
static D3D10STATEBLOCKMASKENABLEALL		g_fnorg_D3D10StateBlockMaskEnableAll = NULL;
static D3D10CREATESTATEBLOCK			g_fnorg_D3D10CreateStateBlock = NULL;

static GETREGIONDATA					g_fnorg_GetRegionData = NULL;
static GETREGIONDATA					g_fn_GetRegionData_trampoline = NULL;

typedef struct _D3D10Ctx
{
	ID3D10VertexShader*			pVertexShader;

	ID3D10PixelShader*			pPixelShader;
	
	ID3D10InputLayout*			pVertexLayout;
	
	ID3D10Buffer*				pVertexBuffer;
	
	ID3D10RenderTargetView*		pRenderTargetView;
	
	ID3D10ShaderResourceView*	pTextureRV;
	
	ID3D10SamplerState*			pSamplerLinear;
	
	ID3D10BlendState1*			palphaEnableBlendingState;
	
	ID3D10Buffer*				pOverLayPixelParameters;
	
	ID3D10Buffer*				pOverLayVertexParameters;
	
	ID3D10Device1*				pD3D10Device1;
	
	IDXGISwapChainDWM*			pIDXGISwapChainDWM;
	
	ID3D10StateBlock*			pOrignalStateBlock;
	
	ID3D10StateBlock*			pCoreStateBlock;
	
	UINT						ScreenWidth;
	
	UINT						ScreenHeight;

	BOOL						Initialized;

	LIST_ENTRY					Link;

}D3D10CTX, *PD3D10CTX;

LIST_ENTRY			D3D10GlobalCtxList = { 0 };

typedef struct _D3D11Ctx
{
	ID3D11VertexShader*				pVertexShader;

	ID3D11PixelShader*				pPixelShader;

	ID3D11InputLayout*				pVertexLayout;

	ID3D11Buffer*					pVertexBuffer;

	ID3D11RenderTargetView*         pRenderTargetView;

	ID3D11ShaderResourceView*       pTextureRV;

	ID3D11SamplerState*             pSamplerLinear;

	ID3D11BlendState*				palphaEnableBlendingState;

	ID3D11Buffer*                   pOverLayPixelParameters;

	ID3D11Buffer*					pOverLayVertexParameters;

	ID3D11Device*					pd3d11Device;

	IDXGISwapChainDWM1*				pIDXISwapChainDWM1;

	ID3D11DeviceContext*			pImmediateContext;

	D3D11StateBlock*				pOrignalStateBlock;

	D3D11StateBlock*				pCoreStateBlock;

	UINT							ScreenWidth;

	UINT							ScreenHeight;

	BOOL							Initialized;

	LIST_ENTRY						Link;

} D3D11CTX, *PD3D11CTX;

LIST_ENTRY			D3D11GlobalCtxList = { 0 };

struct OverLayVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 Tex;
};

struct OverLayVertexParameters
{
	DirectX::XMFLOAT4 Tex2Screen;
};

struct OverLayPixelParameters
{
	DirectX::XMFLOAT4 ClipRect;
	DirectX::XMFLOAT4 Opacity;
};

struct OverLayPixelParameters1
{
	DirectX::XMFLOAT4 ClipRect;
	DirectX::XMFLOAT4 Opacity;
	DirectX::XMFLOAT4 DirtyRects[8];
};

struct _D3D11NODE;

static HRESULT InitializeD3D11Ctx(D3D11CTX *pCtx, const WCHAR *OverLayFileName);
static void CleanupD3D11Ctx(D3D11CTX *pCtx);
static D3D11CTX* CreateD3d11Ctx(IDXGISwapChainDWM1 *pSwapChain, ID3D11Device *pd3dDevice);
static void UpdateAllD3d11CtxIfServiceIsReady(void);

static void myrender1(
	_In_ D3D11CTX *pCtx,
	_In_ UINT  DirtyRectsCount,
	_In_ RECT  *pDirtyRects
);

static HRESULT InitializeD3D10Ctx(D3D10CTX *pCtx, const WCHAR *OverLayFileName);
static void CleanupD3D10Ctx(D3D10CTX *pCtx);
static D3D10CTX* CreateD3d10Ctx(IDXGISwapChainDWM *pSwapChain, ID3D10Device1 *pd3dDevice);
static void UpdateAllD3d10CtxIfServiceIsReady(void);

static void myrender(
	_In_ D3D10CTX *pCtx
);

static BOOL IsMainWindow(HWND hWnd);
static BOOL IsWndVisible(ULONG hWnd);
static void forcast_dwm_window_to_engine(HWND hWnd, ULONG ProcessId, ULONG ThreadId);
static nudf::util::CObligations *query_engine(void);

static BOOL InitializeDWMHookWin7(void);
static BOOL InitializeDWMHookWin10(void);

static D3D11CTX* CreateD3d11Ctx(IDXGISwapChainDWM1 *pSwapChain, ID3D11Device *pd3dDevice);

BOOL InitializeDWMHook(void)
{
	if (Global.IsWin7)
	{
		return InitializeDWMHookWin7();
	}
	else if (Global.IsWin10)
	{
		return InitializeDWMHookWin10();
	}
	else
	{
		return FALSE;
	}
}

static BOOL InitializeDWMHookWin7(void)
{
	BOOL bRet = TRUE;

	HMODULE hUser32 = NULL;
	HMODULE hGdi32 = NULL;

	do
	{
		if (!Global.D3DHandle)
		{
			Global.D3DHandle = GetModuleHandleW(D3D10_1_MODULE_NAME);
		}

		if (!Global.D3DHandle)
		{
			bRet = FALSE;
			break;
		}

		if (!Global.WindowscodecsHandle)
		{
			Global.WindowscodecsHandle = GetModuleHandleW(WINDOWSCODECS_MODULE_NAME);
		}

		if (!Global.WindowscodecsHandle)
		{
			bRet = FALSE;
			break;
		}

		if (!Global.dxgiHandle)
		{
			Global.dxgiHandle = GetModuleHandleW(DXGI_MODULE_NAME);
		}

		if (!Global.dxgiHandle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.DWMHooksInitialized)
		{
			break;
		}

		hUser32 = GetModuleHandleW(USER32_MODULE_NAME);

		if (!hUser32)
		{
			break;
		}

		hGdi32 = GetModuleHandleW(GDI32_MODULE_NAME);

		if (!hGdi32)
		{
			break;
		}

		InitializeListHead(&D3D10GlobalCtxList);

		g_fnorg_WICCreateImagingFactory_Proxy = (WICCREATEIMAGINGFACTORY_PROXY)GetProcAddress(Global.WindowscodecsHandle, DIRECT3D_WICCREATEIMAGINGFACTORY_PROXY_PROC_NAME);

		if (g_fnorg_WICCreateImagingFactory_Proxy)
		{
			if (!install_hook(g_fnorg_WICCreateImagingFactory_Proxy, (PVOID*)&g_fn_WICCreateImagingFactory_Proxy_trampoline, Core_WICCreateImagingFactory_Proxy))
			{
				bRet = FALSE;
				break;
			}
		}

		g_fnorg_CreateDXGIFactory = (CREATEDXGIFACTORY)GetProcAddress(Global.dxgiHandle, DIRECT3D_CREATEDXGIFACTORY_PROC_NAME);

		if (g_fnorg_CreateDXGIFactory)
		{
			if (!install_hook(g_fnorg_CreateDXGIFactory, (PVOID*)&g_fn_CreateDXGIFactory_trampoline, Core_CreateDXGIFactory))
			{
				bRet = FALSE;
				break;
			}
		}

		g_fnorg_D3D10CreateStateBlock = (D3D10CREATESTATEBLOCK)GetProcAddress(Global.D3DHandle, DIRECT3D_D3D10CREATESTATEBLOCK_PROC_NAME);

		g_fnorg_D3D10StateBlockMaskEnableAll = (D3D10STATEBLOCKMASKENABLEALL)GetProcAddress(Global.D3DHandle, DIRECT3D_D3D10STATEBLOCKMASKENABLEALL_PROC_NAME);

		g_fnorg_GetWindowRect = (DIRECT3D_GETWINDOWRECT)GetProcAddress(hUser32, DIRECT3D_GETWINDOWRECT_PROC_NAME);

		g_fnorg_GetWindowThreadProcessId = (DIRECT3D_GETWINDOWTHREADPROCESSID)GetProcAddress(hUser32, DIRECT3D_GETWINDOWTHREADPROCESSID_PROC_NAME);

		g_fnorg_RedrawWindow = (DIRECT3D_REDRAWWINDOW)GetProcAddress(hUser32, DIRECT3D_REDRAWWINDOW_PROC_NAME);

#ifdef _AMD64_

		g_fnorg_GetWindowLongPtrW = (DIRECT3D_GETWINDOWLONGPTRW)GetProcAddress(hUser32, DIRECT3D_GETWINDOWLONGPTRW_PROC_NAME);

#else
		g_fnorg_GetWindowLongW = (DIRECT3D_GETWINDOWLONGW)GetProcAddress(hUser32, DIRECT3D_GETWINDOWLONGW_PROC_NAME);

#endif
		g_fnorg_PostMessageW = (DIRECT3D_POSTMESSAGEW)GetProcAddress(hUser32, DIRECT3D_POSTMESSAGE_PROC_NAME);

		if (g_fnorg_PostMessageW)
		{
			if (!install_hook(g_fnorg_PostMessageW, (PVOID*)&g_fn_PostMessageW_trampoline, Core_PostMessageW))
			{
				bRet = FALSE;
				break;
			}
		}

		g_fnorg_GetRegionData = (GETREGIONDATA)GetProcAddress(hGdi32, DIRECT3D_GETREGIONDATA_PROC_NAME);

		if (g_fnorg_GetRegionData)
		{
			if (!install_hook(g_fnorg_GetRegionData, (PVOID*)&g_fn_GetRegionData_trampoline, Core_GetRegionData))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.DWMHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;

}

static BOOL InitializeDWMHookWin10(void)
{
	BOOL bRet = TRUE;

	HMODULE hUser32 = NULL;
	HMODULE hGdi32 = NULL;

	do
	{
		if (!Global.dxgiHandle)
		{
			Global.dxgiHandle = GetModuleHandleW(DXGI_MODULE_NAME);
		}

		if (!Global.dxgiHandle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.DWMHooksInitialized)
		{
			break;
		}

		hUser32 = GetModuleHandleW(USER32_MODULE_NAME);

		if (!hUser32)
		{
			break;
		}

		hGdi32 = GetModuleHandleW(GDI32_MODULE_NAME);

		if (!hGdi32)
		{
			break;
		}

		InitializeListHead(&D3D11GlobalCtxList);

		g_fnorg_CreateDXGIFactory1 = (CREATEDXGIFACTORY1)GetProcAddress(Global.dxgiHandle, DIRECT3D_CREATEDXGIFACTORY1_PROC_NAME);

		if (g_fnorg_CreateDXGIFactory1)
		{
			if (!install_hook(g_fnorg_CreateDXGIFactory1, (PVOID*)&g_fn_CreateDXGIFactory1_trampoline, Core_CreateDXGIFactory1))
			{
				bRet = FALSE;
				break;
			}
		}

		g_fnorg_GetWindowRect = (DIRECT3D_GETWINDOWRECT)GetProcAddress(hUser32, DIRECT3D_GETWINDOWRECT_PROC_NAME);

		g_fnorg_GetWindowThreadProcessId = (DIRECT3D_GETWINDOWTHREADPROCESSID)GetProcAddress(hUser32, DIRECT3D_GETWINDOWTHREADPROCESSID_PROC_NAME);

		g_fnorg_RedrawWindow = (DIRECT3D_REDRAWWINDOW)GetProcAddress(hUser32, DIRECT3D_REDRAWWINDOW_PROC_NAME);

#ifdef _AMD64_

		g_fnorg_GetWindowLongPtrW = (DIRECT3D_GETWINDOWLONGPTRW)GetProcAddress(hUser32, DIRECT3D_GETWINDOWLONGPTRW_PROC_NAME);

#else
		g_fnorg_GetWindowLongW = (DIRECT3D_GETWINDOWLONGW)GetProcAddress(hUser32, DIRECT3D_GETWINDOWLONGW_PROC_NAME);

#endif
		g_fnorg_PostMessageW = (DIRECT3D_POSTMESSAGEW)GetProcAddress(hUser32, DIRECT3D_POSTMESSAGE_PROC_NAME);

		if (g_fnorg_PostMessageW)
		{
			if (!install_hook(g_fnorg_PostMessageW, (PVOID*)&g_fn_PostMessageW_trampoline, Core_PostMessageW))
			{
				bRet = FALSE;
				break;
			}
		}

		g_fnorg_GetRegionData = (GETREGIONDATA)GetProcAddress(hGdi32, DIRECT3D_GETREGIONDATA_PROC_NAME);

		if (g_fnorg_GetRegionData)
		{
			if (!install_hook(g_fnorg_GetRegionData, (PVOID*)&g_fn_GetRegionData_trampoline, Core_GetRegionData1))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.DWMHooksInitialized = TRUE;

	} while (FALSE);

	return bRet;

}

void CleanupDWMHook(void)
{
	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	if (g_fn_WICCreateImagingFactory_Proxy_trampoline)
	{
		remove_hook(g_fn_WICCreateImagingFactory_Proxy_trampoline);
		g_fn_WICCreateImagingFactory_Proxy_trampoline = NULL;
	}

	if (Global.IsDWM)
	{
		//
		// release the WICFactory we reference
		//
		if (Global.WICFactory)
		{
			Global.WICFactory->Release();
			Global.WICFactory = NULL;
		}

		if (g_fn_CreateDXGIFactory_trampoline)
		{
			remove_hook(g_fn_CreateDXGIFactory_trampoline);
			g_fn_CreateDXGIFactory_trampoline = NULL;
		}

		if (g_fn_IDXGIFactoryDWM_CreateSwapChain_trampoline)
		{
			remove_hook(g_fn_IDXGIFactoryDWM_CreateSwapChain_trampoline);
			g_fn_IDXGIFactoryDWM_CreateSwapChain_trampoline = NULL;
		}

		if (g_fn_CreateDXGIFactory1_trampoline)
		{
			remove_hook(g_fn_CreateDXGIFactory1_trampoline);
			g_fn_CreateDXGIFactory1_trampoline = NULL;
		}

		if (g_fn_IDXGIFactoryDWM1_CreateSwapChain_trampoline)
		{
			remove_hook(g_fn_IDXGIFactoryDWM1_CreateSwapChain_trampoline);
			g_fn_IDXGIFactoryDWM1_CreateSwapChain_trampoline = NULL;
		}

		if (g_fn_PostMessageW_trampoline)
		{
			remove_hook(g_fn_PostMessageW_trampoline);
			g_fn_PostMessageW_trampoline = NULL;
		}

		if (g_fn_GetRegionData_trampoline)
		{
			remove_hook(g_fn_GetRegionData_trampoline);
			g_fn_GetRegionData_trampoline = NULL;
		}

		FOR_EACH_LIST_SAFE(ite, tmp, &D3D10GlobalCtxList)
		{
			D3D10CTX *pCtx = CONTAINING_RECORD(ite, D3D10CTX, Link);

			RemoveEntryList(ite);

			CleanupD3D10Ctx(pCtx);

			free(pCtx);
			pCtx = NULL;
		}

		FOR_EACH_LIST_SAFE(ite, tmp, &D3D11GlobalCtxList)
		{
			D3D11CTX *pCtx = CONTAINING_RECORD(ite, D3D11CTX, Link);

			RemoveEntryList(ite);

			CleanupD3D11Ctx(pCtx);

			free(pCtx);
			pCtx = NULL;
		}
	}
}

HRESULT WINAPI Core_WICCreateImagingFactory_Proxy(
	_In_  UINT               SDKVersion,
	_Out_ IWICImagingFactory **ppIImagingFactory
	)
{
	HRESULT hr = S_OK;

	hr = g_fn_WICCreateImagingFactory_Proxy_trampoline(SDKVersion, ppIImagingFactory);

	if (SUCCEEDED(hr))
	{
		if (!InterlockedCompareExchangePointer((PVOID*)&Global.WICFactory, *ppIImagingFactory, NULL))
		{
			//
			// reference once to make sure nobody can free it
			//
			Global.WICFactory->AddRef();
		}
	}

	return hr;
}


HRESULT WINAPI CreateD3D10TextureFromFile(
	_In_ ID3D10Device1* d3dDevice,
	_In_z_ const WCHAR* FileName,
	_Out_opt_ ID3D10Resource** texture,
	_Out_opt_ ID3D10ShaderResourceView** textureView,
	_Out_opt_ ULONG *Width,
	_Out_opt_ ULONG *Height)
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder		*pDecoder = NULL;
	IWICBitmapFrameDecode	*pSource = NULL;

	UINT uiWidth = 0;
	UINT uiHeight = 0;

	UCHAR *pRawPixels = NULL;

	UINT uiRowPitch = 0;
	UINT uiImageSize = 0;

	ID3D10Texture2D* tex = NULL;

	D3D10_TEXTURE2D_DESC desc = { 0 };

	D3D10_SUBRESOURCE_DATA initData = { 0 };

	do 
	{
		if (!Global.WICFactory)
		{
			hr = E_UNEXPECTED;
			break;
		}

		hr = Global.WICFactory->CreateDecoderFromFilename(FileName,
														  NULL,
														  GENERIC_READ,
														  WICDecodeMetadataCacheOnLoad,
														  &pDecoder);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pDecoder->GetFrame(0, &pSource);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pSource->GetSize(&uiWidth, &uiHeight);

		if (uiWidth > 4096 || uiHeight > 4096)
		{
			float ar = static_cast<float>(uiHeight) / static_cast<float>(uiWidth);

			if (uiWidth > uiHeight)
			{
				uiWidth = static_cast<UINT>(4096);
				uiHeight = static_cast<UINT>(static_cast<float>(4096) * ar);
			}
			else
			{
				uiHeight = static_cast<UINT>(4096);
				uiWidth = static_cast<UINT>(static_cast<float>(4096) / ar);
			}
		}

		uiRowPitch = (uiWidth * 32 + 7) / 8;
		uiImageSize = uiRowPitch * uiHeight;

		pRawPixels = (UCHAR*)malloc(uiImageSize);

		if (!pRawPixels)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		hr = pSource->CopyPixels(0, 
								 uiRowPitch, 
								 uiImageSize, 
								 pRawPixels);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		// Create texture

		desc.Width				= uiWidth;
		desc.Height				= uiHeight;
		desc.MipLevels			= 1;
		desc.ArraySize			= 1;
		desc.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count	= 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage				= D3D10_USAGE_DEFAULT;
		desc.BindFlags			= D3D10_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags		= 0;
		desc.MiscFlags			= 0;


		initData.pSysMem			= pRawPixels;
		initData.SysMemPitch		= uiRowPitch;
		initData.SysMemSlicePitch	= uiImageSize;


		hr = d3dDevice->CreateTexture2D(&desc,
										&initData, 
										&tex);

		if (!SUCCEEDED(hr) || tex == 0)
		{
			break;
		}

		if (textureView != 0)
		{
			D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;

			memset(&SRVDesc, 0, sizeof(SRVDesc));

			SRVDesc.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
			SRVDesc.ViewDimension		= D3D10_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = 1;

			hr = d3dDevice->CreateShaderResourceView(tex, 
													 &SRVDesc, 
													 textureView);

			if (!SUCCEEDED(hr))
			{
				tex->Release();
				tex = NULL;
				break;
			}
		}

		if (texture != 0)
		{
			*texture = tex;
			tex = NULL;		// prevent tex from being released
		}

		if (Width)
		{
			*Width = uiWidth;
		}

		if (Height)
		{
			*Height = uiHeight;
		}

	} while (FALSE);

	if (pSource)
	{
		pSource->Release();
		pSource = NULL;
	}

	if (pDecoder)
	{
		pDecoder->Release();
		pDecoder = NULL;
	}
	
	if (pRawPixels)
	{
		free(pRawPixels);
		pRawPixels = NULL;
	}

	if (tex)
	{
		tex->Release();
		tex = NULL;
	}
	
	return hr;
}

static HRESULT InitializeD3D10Ctx(D3D10CTX *pCtx, const WCHAR *OverLayFileName)
{
	HRESULT hr = S_OK;

	// Define the input layout
	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	OverLayVertex vertices[] =
	{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f),  DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
	};

	D3D10_BUFFER_DESC bd = { 0 };
	D3D10_SUBRESOURCE_DATA InitData = { 0 };

	D3D10_SAMPLER_DESC sampDesc;

	D3D10_BLEND_DESC1 blendStateDescription = { 0 };

	OverLayPixelParameters PixelParams;

	OverLayVertexParameters VertexParams = { DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) };

	D3D10_VIEWPORT vp = { 0 };

	float blendFactor[4] = { 0.0f };

	D3D10_STATE_BLOCK_MASK Mask = { 0 };

	ID3D10Texture2D *pBackBuffer = NULL;

	D3D10_TEXTURE2D_DESC backBufferSurfaceDesc = { 0 };

	ULONG TexWidth = 0;
	ULONG TexHeight = 0;

	IDXGISwapChainDWM *pSwapChain = pCtx->pIDXGISwapChainDWM;
	ID3D10Device1 *pd3dDevice = pCtx->pD3D10Device1;

	do
	{
		g_fnorg_D3D10StateBlockMaskEnableAll(&Mask);

		hr = g_fnorg_D3D10CreateStateBlock(pd3dDevice, &Mask, &pCtx->pOrignalStateBlock);

		if (FAILED(hr))
		{
			break;
		}

		pCtx->pOrignalStateBlock->Capture();

		hr = g_fnorg_D3D10CreateStateBlock(pd3dDevice, &Mask, &pCtx->pCoreStateBlock);

		if (FAILED(hr))
		{
			break;
		}

		hr = pSwapChain->GetBuffer(0, __uuidof(*pBackBuffer), (LPVOID*)&pBackBuffer);
		
		if (FAILED(hr))
		{
			break;
		}

		pd3dDevice->ClearState();

		pBackBuffer->GetDesc(&backBufferSurfaceDesc);

		pCtx->ScreenWidth = backBufferSurfaceDesc.Width;
		pCtx->ScreenHeight = backBufferSurfaceDesc.Height;

		// Set view port
		ZeroMemory(&vp, sizeof(vp));
		vp.Width = pCtx->ScreenWidth;
		vp.Height = pCtx->ScreenHeight;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		pd3dDevice->RSSetViewports(1, &vp);

		hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &pCtx->pRenderTargetView);

		if (FAILED(hr))
		{
			break;
		}

		pd3dDevice->OMSetRenderTargets(1, &pCtx->pRenderTargetView, NULL);

		// Create the vertex shader
		hr = pd3dDevice->CreateVertexShader(g_OverlayVsshader, sizeof(g_OverlayVsshader), &pCtx->pVertexShader);
		
		if (FAILED(hr))
		{
			break;
		}

		// Create the input layout
		hr = pd3dDevice->CreateInputLayout(layout, 
										   numElements,
										   g_OverlayVsshader,
										   sizeof(g_OverlayVsshader),
										   &pCtx->pVertexLayout);

		if (FAILED(hr))
		{
			break;
		}

		// Create the pixel shader
		hr = pd3dDevice->CreatePixelShader(g_Overlaypsshader, sizeof(g_Overlaypsshader), &pCtx->pPixelShader);

		if (FAILED(hr))
		{
			break;
		}

		// Create vertex buffer
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage			= D3D10_USAGE_DEFAULT;
		bd.ByteWidth		= sizeof(OverLayVertex) * 6;
		bd.BindFlags		= D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags	= 0;

		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertices;

		hr = pd3dDevice->CreateBuffer(&bd, &InitData, &pCtx->pVertexBuffer);

		if (FAILED(hr))
		{
			break;
		}

		// Load the Texture
		hr = CreateD3D10TextureFromFile(pd3dDevice, OverLayFileName, NULL, &pCtx->pTextureRV, &TexWidth, &TexHeight);

		if (FAILED(hr))
		{
			break;
		}

		// Create the sample state
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter			= D3D10_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU		= D3D10_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV		= D3D10_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW		= D3D10_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D10_COMPARISON_NEVER;
		sampDesc.MinLOD			= 0;
		sampDesc.MaxLOD			= D3D10_FLOAT32_MAX;

		hr = pd3dDevice->CreateSamplerState(&sampDesc, &pCtx->pSamplerLinear);

		if (FAILED(hr))
		{
			break;
		}

		ZeroMemory(&blendStateDescription, sizeof(D3D10_BLEND_DESC1));

		blendStateDescription.AlphaToCoverageEnable = FALSE;
		blendStateDescription.IndependentBlendEnable = FALSE;

		// only have one RenderTarget
		blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
		blendStateDescription.RenderTarget[0].SrcBlend = D3D10_BLEND_ONE;
		blendStateDescription.RenderTarget[0].DestBlend = D3D10_BLEND_INV_SRC_ALPHA;
		blendStateDescription.RenderTarget[0].BlendOp = D3D10_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D10_BLEND_ONE;
		blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D10_BLEND_INV_SRC_ALPHA;
		blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D10_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

		hr = pd3dDevice->CreateBlendState1(&blendStateDescription, &pCtx->palphaEnableBlendingState);

		if (FAILED(hr))
		{
			break;
		}

		// Create the constant buffers
		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(OverLayPixelParameters);
		bd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		
		hr = pd3dDevice->CreateBuffer(&bd, NULL, &pCtx->pOverLayPixelParameters);

		if (FAILED(hr))
		{
			break;
		}

		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D10_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(OverLayVertexParameters);
		bd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;

		hr = pd3dDevice->CreateBuffer(&bd, NULL, &pCtx->pOverLayVertexParameters);

		if (FAILED(hr))
		{
			break;
		}

		memset(&PixelParams, 0, sizeof(PixelParams));

		PixelParams.ClipRect = DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, -1.0f);
		PixelParams.Opacity = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, ((float)Global.Opacity / 100.0f));

		pd3dDevice->UpdateSubresource(pCtx->pOverLayPixelParameters, 0, NULL, &PixelParams, 0, 0);
		
		if (TexHeight == 0 || TexWidth == 0)
		{
			VertexParams.Tex2Screen = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			VertexParams.Tex2Screen = DirectX::XMFLOAT4((float)pCtx->ScreenWidth / (float)TexWidth, 
														(float)pCtx->ScreenHeight / (float)TexHeight, 
														0.0f, 
														0.0f);
		}

		pd3dDevice->UpdateSubresource(pCtx->pOverLayVertexParameters, 0, NULL, &VertexParams, 0, 0);

		// Set the input layout
		pd3dDevice->IASetInputLayout(pCtx->pVertexLayout);

		// Set primitive topology 
		pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Turn on the alpha blending.
		pd3dDevice->OMSetBlendState(pCtx->palphaEnableBlendingState, blendFactor, 0xffffffff);

		// Set vertex buffer
		UINT stride = sizeof(OverLayVertex);
		UINT offset = 0;
		pd3dDevice->IASetVertexBuffers(0, 1, &pCtx->pVertexBuffer, &stride, &offset);

		// Render a triangle
		pd3dDevice->VSSetShader(pCtx->pVertexShader);
		pd3dDevice->PSSetShader(pCtx->pPixelShader);

		pd3dDevice->PSSetShaderResources(0, 1, &pCtx->pTextureRV);
		pd3dDevice->PSSetConstantBuffers(1, 1, &pCtx->pOverLayPixelParameters);

		pd3dDevice->VSSetConstantBuffers(0, 1, &pCtx->pOverLayVertexParameters);

		pd3dDevice->PSSetSamplers(0, 1, &pCtx->pSamplerLinear);

		pCtx->pCoreStateBlock->Capture();

	} while (FALSE);

	if (pBackBuffer)
	{
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}

	if (pCtx->pOrignalStateBlock)
	{
		pCtx->pOrignalStateBlock->Apply();
	}

	return hr;
}

static void CleanupD3D10Ctx(D3D10CTX *pCtx)
{
	if (pCtx->pOrignalStateBlock)
	{
		pCtx->pOrignalStateBlock->Release();
		pCtx->pOrignalStateBlock = NULL;
	}

	if (pCtx->pCoreStateBlock)
	{
		pCtx->pCoreStateBlock->Release();
		pCtx->pCoreStateBlock = NULL;
	}
	
	if (pCtx->pRenderTargetView)
	{
		pCtx->pRenderTargetView->Release();
		pCtx->pRenderTargetView = NULL;
	}

	if (pCtx->pVertexShader)
	{
		pCtx->pVertexShader->Release();
		pCtx->pVertexShader = NULL;
	}

	if (pCtx->pPixelShader)
	{
		pCtx->pPixelShader->Release();
		pCtx->pPixelShader = NULL;
	}

	if (pCtx->pVertexLayout)
	{
		pCtx->pVertexLayout->Release();
		pCtx->pVertexLayout = NULL;
	}

	if (pCtx->pVertexBuffer)
	{
		pCtx->pVertexBuffer->Release();
		pCtx->pVertexBuffer = NULL;
	}

	if (pCtx->pOverLayPixelParameters)
	{
		pCtx->pOverLayPixelParameters->Release();
		pCtx->pOverLayPixelParameters = NULL;
	}

	if (pCtx->pOverLayVertexParameters)
	{
		pCtx->pOverLayVertexParameters->Release();
		pCtx->pOverLayVertexParameters = NULL;
	}

	if (pCtx->pTextureRV)
	{
		pCtx->pTextureRV->Release();
		pCtx->pTextureRV = NULL;
	}

	if (pCtx->pSamplerLinear)
	{
		pCtx->pSamplerLinear->Release();
		pCtx->pSamplerLinear = NULL;
	}

	if (pCtx->palphaEnableBlendingState)
	{
		pCtx->palphaEnableBlendingState->Release();
		pCtx->palphaEnableBlendingState = NULL;
	}
}

static void myrender(_In_ D3D10CTX *pCtx)
{
	RECT rc = { 0 };
	OverLayPixelParameters PixelParams;

	BOOL bInvalidAllWindows = FALSE;

	IDXGIOutput *pDXGIOutput = NULL;

	HRESULT hr = S_OK;

	DXGI_OUTPUT_DESC OutputDesc = { 0 };

	do 
	{
		if (pCtx->palphaEnableBlendingState == NULL ||
			pCtx->pVertexLayout == NULL ||
			pCtx->pVertexBuffer == NULL ||
			pCtx->pVertexShader == NULL ||
			pCtx->pPixelShader == NULL ||
			pCtx->pTextureRV == NULL ||
			pCtx->pSamplerLinear == NULL ||
			pCtx->pOrignalStateBlock == NULL ||
			pCtx->pCoreStateBlock == NULL ||
			pCtx->pD3D10Device1 == NULL ||
			pCtx->pIDXGISwapChainDWM == NULL ||
			g_fnorg_GetWindowRect == NULL ||
			Global.hWnds[0] == NULL /* don't pay performance tax if there is no dirty window */)	
		{
			break;
		}

		hr = pCtx->pIDXGISwapChainDWM->GetContainingOutput(&pDXGIOutput);

		if (!SUCCEEDED(hr) || pDXGIOutput == NULL)
		{
			break;
		}

		hr = pDXGIOutput->GetDesc(&OutputDesc);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pCtx->pOrignalStateBlock->Capture();

		pCtx->pCoreStateBlock->Apply();

		for (ULONG i = 0; i < sizeof(Global.hWnds) / sizeof(Global.hWnds[0]); i++)
		{
			if (Global.hWnds[i] == NULL)
			{
				break;
			}

			if (Global.WndsAttr[i] == 0xffffffff)
			{
				continue;
			}

			if (!g_fnorg_GetWindowRect((HWND)(ULONG_PTR)Global.hWnds[i], &rc))
			{
				Global.WndsAttr[i] = 0xffffffff;

				if (!bInvalidAllWindows)
					bInvalidAllWindows = TRUE;

				continue;
			}

			rc.top -= OutputDesc.DesktopCoordinates.top;
			rc.left -= OutputDesc.DesktopCoordinates.left;
			rc.bottom = pCtx->ScreenHeight - (OutputDesc.DesktopCoordinates.bottom - rc.bottom);
			rc.right = pCtx->ScreenWidth - (OutputDesc.DesktopCoordinates.right - rc.right);

			rc.top += 5;
			rc.left += 5;
			rc.bottom -= 5;
			rc.right -= 5;

			memset(&PixelParams, 0, sizeof(PixelParams));

			PixelParams.ClipRect = DirectX::XMFLOAT4(((float)(rc.left * 2) / (float)pCtx->ScreenWidth) - 1.0f,
													 1.0f - ((float)(rc.top * 2) / (float)pCtx->ScreenHeight),
													 ((float)(rc.right * 2) / (float)pCtx->ScreenWidth) - 1.0f,
													 1.0f - ((float)(rc.bottom * 2) / (float)pCtx->ScreenHeight));

			PixelParams.Opacity = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, ((float)Global.Opacity / 100.0f));

			pCtx->pD3D10Device1->UpdateSubresource(pCtx->pOverLayPixelParameters, 0, NULL, &PixelParams, 0, 0);

			pCtx->pD3D10Device1->Draw(6, 0);
		}

		pCtx->pOrignalStateBlock->Apply();

	} while (FALSE);

	if (bInvalidAllWindows && g_fnorg_RedrawWindow)
	{
		g_fnorg_RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
	}

	if (pDXGIOutput)
	{
		pDXGIOutput->Release();
		pDXGIOutput = NULL;
	}

	return;
}

HRESULT WINAPI Core_CreateDXGIFactory(REFIID riid, void **ppFactory)
{
	HRESULT hr = S_OK;

	IDXGIFactory *pFactory = NULL;

	IUnknown *pIDXGIFactoryDWM = NULL;

	PVOID pIDXGIFactoryDWMVtbl = NULL;

	ULONG_PTR fn_CreateSwapChain = 0;

	do 
	{
		hr = g_fn_CreateDXGIFactory_trampoline(riid, (void**)&pFactory);

		if (SUCCEEDED(hr) && pFactory)
		{
			hr = pFactory->QueryInterface(IID_IDXGIFactoryDWM, (void**)&pIDXGIFactoryDWM);

			if (SUCCEEDED(hr) && pIDXGIFactoryDWM)
			{
				if (g_fnorg_IDXGIFactoryDWM_CreateSwapChain == NULL && g_fn_IDXGIFactoryDWM_CreateSwapChain_trampoline == NULL)
				{
					pIDXGIFactoryDWMVtbl = *((void**)pIDXGIFactoryDWM);

					fn_CreateSwapChain = *(ULONG_PTR*)((UCHAR*)pIDXGIFactoryDWMVtbl + sizeof(ULONG_PTR) * 3);

					g_fnorg_IDXGIFactoryDWM_CreateSwapChain = (IDXGIFACTORYDWM_CREATESWAPCHAINDWM)(fn_CreateSwapChain);

					if (g_fnorg_IDXGIFactoryDWM_CreateSwapChain)
					{
						if (!install_hook(g_fnorg_IDXGIFactoryDWM_CreateSwapChain, (PVOID*)&g_fn_IDXGIFactoryDWM_CreateSwapChain_trampoline, Core_CreateSwapChainDWM))
						{
							g_fnorg_IDXGIFactoryDWM_CreateSwapChain = NULL;
						}
					}
				}

				pIDXGIFactoryDWM->Release();
				pIDXGIFactoryDWM = NULL;
			}
		}

		*ppFactory = pFactory;

	} while (FALSE);

	return hr;
}

HRESULT WINAPI Core_CreateSwapChainDWM(
	_In_  IDXGIFactory	*pIDXGIFactory,
	_In_  IUnknown *pDevice,
	_In_  DXGI_SWAP_CHAIN_DESC *pDesc,
	_In_  IDXGIOutput *pOutput,
	_Out_  IDXGISwapChainDWM **ppSwapChainDWM)
{
	HRESULT hr = S_OK;

	IDXGISwapChainDWM *pIDXGISwapChainDWM = NULL;
	CoreIDXGISwapChainDWM *pCorepIDXGISwapChainDWM = NULL;

	ULONG_PTR fn_Present = 0;

	ID3D10Device1 *p3d10Device1 = NULL;

	do 
	{
		hr = g_fn_IDXGIFactoryDWM_CreateSwapChain_trampoline(pIDXGIFactory, 
															 pDevice, 
															 pDesc, 
															 pOutput,
															 &pIDXGISwapChainDWM);

		if (SUCCEEDED(hr) && pIDXGISwapChainDWM)
		{
			pDevice->QueryInterface(__uuidof(ID3D10Device1), (void**)&p3d10Device1);

			if (p3d10Device1)
			{
				D3D10CTX *pCtx = NULL;
				LIST_ENTRY *ite = NULL;

				pCtx = CreateD3d10Ctx(pIDXGISwapChainDWM, p3d10Device1);

				if (pCtx)
				{
					EnterCriticalSection(&Global.D3D10GlobalLock);
					FOR_EACH_LIST(ite, &D3D10GlobalCtxList)
					{
						D3D10CTX *pNode = CONTAINING_RECORD(ite, D3D10CTX, Link);

						if (pNode->pIDXGISwapChainDWM == pIDXGISwapChainDWM)
						{
							CleanupD3D10Ctx(pCtx);

							free(pCtx);
							pCtx = NULL;
							break;
						}
					}

					if (pCtx) {
						InsertHeadList(&D3D10GlobalCtxList, &pCtx->Link);
					}
					LeaveCriticalSection(&Global.D3D10GlobalLock);

					UpdateAllD3d10CtxIfServiceIsReady();
				}
			}

			pCorepIDXGISwapChainDWM = new CoreIDXGISwapChainDWM(pIDXGISwapChainDWM);

			*ppSwapChainDWM = (IDXGISwapChainDWM*)pCorepIDXGISwapChainDWM;

			break;
		}

		//
		// only error case runs here
		*ppSwapChainDWM = pIDXGISwapChainDWM;

	} while (FALSE);

	if (p3d10Device1)
	{
		p3d10Device1->Release();
		p3d10Device1 = NULL;
	}

	return hr;
}

CoreIDXGISwapChainDWM::CoreIDXGISwapChainDWM()
{
	m_uRefCount = 1;
	m_pIDXGISwapChainDWM = NULL;
}

CoreIDXGISwapChainDWM::CoreIDXGISwapChainDWM(IDXGISwapChainDWM *pIDXGISwapChainDWM)
{
	m_uRefCount = 1;
	m_pIDXGISwapChainDWM = pIDXGISwapChainDWM;
}

CoreIDXGISwapChainDWM::~CoreIDXGISwapChainDWM()
{
	LIST_ENTRY *ite = NULL;
	LIST_ENTRY *tmp = NULL;

	if (m_pIDXGISwapChainDWM)
	{
		EnterCriticalSection(&Global.D3D10GlobalLock);

		FOR_EACH_LIST_SAFE(ite, tmp, &D3D10GlobalCtxList)
		{
			D3D10CTX *pCtx = CONTAINING_RECORD(ite, D3D10CTX, Link);

			if (pCtx->pIDXGISwapChainDWM == m_pIDXGISwapChainDWM)
			{
				RemoveEntryList(ite);

				CleanupD3D10Ctx(pCtx);

				free(pCtx);
				pCtx = NULL;
				break;
			}
		}

		LeaveCriticalSection(&Global.D3D10GlobalLock);

		m_pIDXGISwapChainDWM->Release();
		m_pIDXGISwapChainDWM = NULL;
	}
}

STDMETHODIMP CoreIDXGISwapChainDWM::QueryInterface(REFIID riid, void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do
	{
		if (IID_IUnknown == riid)
		{
			punk = (IUnknown *)this;
		}
		else
		{
			hRet = m_pIDXGISwapChainDWM->QueryInterface(riid, ppobj);
			break;
		}

		AddRef();

		*ppobj = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP_(ULONG) CoreIDXGISwapChainDWM::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreIDXGISwapChainDWM::Release()
{
	ULONG uCount = 0;

	if (m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if (!uCount)
	{
		delete this;
	}

	return uCount;
}

STDMETHODIMP CoreIDXGISwapChainDWM::SetPrivateData(
	REFGUID Name,
	UINT DataSize,
	const void *pData)
{
	return m_pIDXGISwapChainDWM->SetPrivateData(Name, DataSize, pData);
}

STDMETHODIMP CoreIDXGISwapChainDWM::SetPrivateDataInterface(
	REFGUID Name,
	const IUnknown *pUnknown)
{
	return m_pIDXGISwapChainDWM->SetPrivateDataInterface(Name, pUnknown);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetPrivateData(
	REFGUID Name,
	UINT*pDataSize,
	void *pData)
{
	return m_pIDXGISwapChainDWM->GetPrivateData(Name, pDataSize, pData);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetParent(
	REFIID riid,
	void **ppParent)
{
	return m_pIDXGISwapChainDWM->GetParent(riid, ppParent);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetDevice(
	REFIID riid,
	void **ppDevice)
{
	return m_pIDXGISwapChainDWM->GetDevice(riid, ppDevice);
}

STDMETHODIMP CoreIDXGISwapChainDWM::Present(
	UINT SyncInterval,
	UINT Flags)
{
	D3D10CTX *pNode = NULL;
	LIST_ENTRY *ite = NULL;

	do 
	{
		if (Flags & DXGI_PRESENT_TEST)
		{
			break;
		}

		EnterCriticalSection(&Global.D3D10GlobalLock);

		FOR_EACH_LIST(ite, &D3D10GlobalCtxList)
		{
			pNode = CONTAINING_RECORD(ite, D3D10CTX, Link);

			if (pNode->pIDXGISwapChainDWM == m_pIDXGISwapChainDWM)
			{
				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		LeaveCriticalSection(&Global.D3D10GlobalLock);

		//
		// it's OK to do it outside the lock
		//
		if (pNode)
		{
			myrender(pNode);
		}

	} while (FALSE);

	return m_pIDXGISwapChainDWM->Present(SyncInterval, Flags);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetBuffer(
	UINT Buffer,
	REFIID riid,
	void **ppSurface)
{
	return m_pIDXGISwapChainDWM->GetBuffer(Buffer, riid, ppSurface);
}

STDMETHODIMP CoreIDXGISwapChainDWM::SetFullscreenState(
	BOOL Fullscreen,
	IDXGIOutput *pTarget)
{
	return m_pIDXGISwapChainDWM->SetFullscreenState(Fullscreen, pTarget);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetFullscreenState(
	BOOL *pFullscreen,
	IDXGIOutput **ppTarget)
{
	return m_pIDXGISwapChainDWM->GetFullscreenState(pFullscreen, ppTarget);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetDesc(DXGI_SWAP_CHAIN_DESC *pDesc)
{
	return m_pIDXGISwapChainDWM->GetDesc(pDesc);
}

STDMETHODIMP CoreIDXGISwapChainDWM::ResizeBuffers(
	UINT BufferCount,
	UINT Width,
	UINT Height,
	DXGI_FORMAT NewFormat,
	UINT SwapChainFlags)
{
	HRESULT hr = S_OK;

	hr = m_pIDXGISwapChainDWM->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);

	return hr;
}

STDMETHODIMP CoreIDXGISwapChainDWM::ResizeTarget(const DXGI_MODE_DESC *pNewTargetParameters)
{
	HRESULT hr = S_OK;

	hr = m_pIDXGISwapChainDWM->ResizeTarget(pNewTargetParameters);

	return hr;
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetContainingOutput(IDXGIOutput **ppOutput)
{
	return m_pIDXGISwapChainDWM->GetContainingOutput(ppOutput);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetFrameStatistics(DXGI_FRAME_STATISTICS *pStats)
{
	return m_pIDXGISwapChainDWM->GetFrameStatistics(pStats);
}

STDMETHODIMP CoreIDXGISwapChainDWM::GetLastPresentCount(UINT *pLastPresentCount)
{
	return m_pIDXGISwapChainDWM->GetLastPresentCount(pLastPresentCount);
}

BOOL WINAPI Core_PostMessageW(
	__in_opt	HWND	hWnd,
	__in		UINT	Msg,
	__in		WPARAM	wParam,
	__in		LPARAM	lParam)
{
	ULONG Pid = 0;
	ULONG Tid = 0;

	if (Msg == WM_DWMNCRENDERINGCHANGED)
	{
		if (IsMainWindow(hWnd))
		{
			do 
			{
				if (!init_rm_section_safe())
				{
					break;
				}

				if (get_dwm_active_session(Global.Section) != Global.SessionId)
				{
					set_dwm_active_session(Global.Section, Global.SessionId);
				}

				Tid = g_fnorg_GetWindowThreadProcessId(hWnd, &Pid);

				forcast_dwm_window_to_engine(hWnd, Pid, Tid);

			} while (FALSE);
		}
	}

	return g_fn_PostMessageW_trampoline(hWnd, Msg, wParam, lParam);
}

static BOOL IsMainWindow(HWND hWnd)
{
	BOOL bRet = FALSE;

#ifdef _AMD64_
	ULONG_PTR Attr = 0;
#else
	ULONG Attr = 0;
#endif

	do
	{
#ifdef _AMD64_

		Attr = g_fnorg_GetWindowLongPtrW(hWnd, GWL_STYLE);

#else

		Attr = g_fnorg_GetWindowLongW(hWnd, GWL_STYLE);
#endif

		if (Attr == 0 || Attr & WS_CHILD)
		{
			break;
		}

		bRet = TRUE;

	} while (FALSE);

	return bRet;
}

static void forcast_dwm_window_to_engine(HWND hWnd, ULONG ProcessId, ULONG ThreadId)
{
	UPDATE_DWM_WINDOW_REQUEST req = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do
	{
		req.ProcessId	= ProcessId;
		req.ThreadId	= ThreadId;
		req.SessionId	= Global.SessionId;
		req.Op			= NXRMDRV_DWM_WINDOW_ADD;

		req.hWnd = (ULONG)(ULONG_PTR)hWnd;

		submit_notify(Global.Section, NXRMDRV_MSG_TYPE_UPDATE_DWM_WINDOW, &req, sizeof(req));

	} while (FALSE);

	return;
}

DWORD WINAPI Core_GetRegionData(
	_In_ HRGN											hrgn,
	_In_ DWORD											nCount,
	_Out_writes_bytes_to_opt_(nCount, return) LPRGNDATA lpRgnData)
{
	ULONG cbSize = sizeof(Global.hWnds);

	LONG PolicySN = 0;

	BOOL bIsPreviousWndListEmpty = TRUE;

	BOOL bInvalidDesktopWnd = FALSE;

	do
	{
		if (lpRgnData == NULL)
		{
			break;
		}

		if (!init_rm_section_safe())
		{
			break;
		}
		
		if (get_dwm_active_session(Global.Section) != Global.SessionId)
		{
			set_dwm_active_session(Global.Section, Global.SessionId);
		}

		UpdateAllD3d10CtxIfServiceIsReady();

		PolicySN = get_rm_policy_sn(Global.Section);

		if (Global.LastPolicySN != PolicySN)
		{
			//
			// no lock required because DWM use single thread to get region data and render
			// the place (myrender) that need to access Global.hWnds is in the same thread
			//
			bIsPreviousWndListEmpty = (Global.hWnds[0] == NULL) ? TRUE : FALSE;

			memset(Global.hWnds, 0, sizeof(Global.hWnds));

			memset(Global.WndsAttr, 0, sizeof(Global.WndsAttr));

			if (get_overlay_windows(Global.Section, Global.hWnds, &cbSize))
			{
				Global.LastPolicySN = PolicySN;
			}

			if (!bIsPreviousWndListEmpty && Global.hWnds[0] == NULL && g_fnorg_RedrawWindow)
			{
				bInvalidDesktopWnd = TRUE;
			}

			//
			// scan all hWnds to make sure there is no hidden Window
			//
			for (ULONG i = 0; i < sizeof(Global.hWnds) / sizeof(Global.hWnds[0]); i++)
			{
				if (Global.hWnds[i])
				{
					if (!IsWndVisible(Global.hWnds[i]))
					{
						Global.WndsAttr[i] = 0xffffffff;
					}
					else
					{
						Global.WndsAttr[i] = 0;
					}
				}
				else
				{
					Global.WndsAttr[i] = 0xffffffff;
				}
			}

			if (bInvalidDesktopWnd)
			{
				g_fnorg_RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
			}
		}
		else
		{
			//
			// scan all hWnds to make sure there is no hidden Window
			//
			for (ULONG i = 0; i < sizeof(Global.hWnds) / sizeof(Global.hWnds[0]); i++)
			{
				if (Global.hWnds[i] && Global.WndsAttr[i] == 0xffffffff)
				{
					//
					// check again on those previously hidden 
					//
					if (!IsWndVisible(Global.hWnds[i]))
					{
						Global.WndsAttr[i] = 0xffffffff;
					}
					else
					{
						Global.WndsAttr[i] = 0;
					}
				}
			}
		}

	} while (FALSE);

	return g_fn_GetRegionData_trampoline(hrgn, nCount, lpRgnData);
}

static void UpdateAllD3d10CtxIfServiceIsReady(void)
{
	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	HRESULT hr = S_OK;

	D3D10CTX *pCtx = NULL;

	LIST_ENTRY *ite = NULL;

	do
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (!is_overlay_bitmap_ready(Global.Section))
		{
			break;
		}

		EnterCriticalSection(&Global.D3D10GlobalLock);
		FOR_EACH_LIST(ite, &D3D10GlobalCtxList)
		{
			pCtx = CONTAINING_RECORD(ite, D3D10CTX, Link);

			if (!pCtx->Initialized)
			{
				LeaveCriticalSection(&Global.D3D10GlobalLock);
				if (!BitmapFileName.length())
				{
					do
					{
						Obligations = query_engine();

						if (!Obligations)
						{
							break;
						}

						if (Obligations->IsEmpty())
						{
							break;
						}

						OverlayOb = (Obligations->GetObligations())[OB_ID_OVERLAY];

						BitmapFileName = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_IMAGE];

						if (!BitmapFileName.length())
						{
							break;
						}

						Opacity = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_TRANSPARENCY];

						if (Opacity.length())
						{
							Global.Opacity = std::wcstol(Opacity.c_str(), NULL, 10);
						}
						else
						{
							Global.Opacity = 10;
						}

					} while (FALSE);
				}
				EnterCriticalSection(&Global.D3D10GlobalLock);

				if (BitmapFileName.length() && !pCtx->Initialized)//make sure initialized flag is not changed after leaveCriticialSection
				{
					hr = InitializeD3D10Ctx(pCtx, BitmapFileName.c_str());

					if (SUCCEEDED(hr))
					{
						pCtx->Initialized = TRUE;
					}
				}
			}
		}
		LeaveCriticalSection(&Global.D3D10GlobalLock);

	} while (FALSE);

	return;
}

static D3D10CTX* CreateD3d10Ctx(IDXGISwapChainDWM *pSwapChain, ID3D10Device1 *pd3dDevice)
{
	D3D10CTX *pCtx = NULL;

	do
	{
		pCtx = (D3D10CTX*)malloc(sizeof(D3D10CTX));

		if (pCtx)
		{
			memset(pCtx, 0, sizeof(D3D10CTX));

			pCtx->pD3D10Device1 = pd3dDevice;
			pCtx->pIDXGISwapChainDWM = pSwapChain;
			pCtx->Initialized = FALSE;
		}

	} while (FALSE);

	return pCtx;
}

static nudf::util::CObligations *query_engine(void)
{
	nudf::util::CObligations *Obs = NULL;

	CHECK_OBLIGATION_REQUEST req = { 0 };

	UCHAR ObsBuf[4096] = { 0 };

	NXCONTEXT Ctx = NULL;

	ULONG bytesret = 0;

	do 
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		req.ProcessId = GetCurrentProcessId();
		req.ThreadId = GetCurrentThreadId();

		GetTempPathW(sizeof(req.TempPath) / sizeof(WCHAR), req.TempPath);

		GetLongPathNameW(req.TempPath, req.TempPath, sizeof(req.TempPath) / sizeof(WCHAR));

		req.EvaluationId = MAX_ULONGLONG;

		Ctx = submit_request(Global.Section, NXRMDRV_MSG_TYPE_CHECKOBLIGATION, &req, sizeof(req));

		if (!Ctx)
		{
			break;
		}

		if (!wait_for_response(Ctx, Global.Section, (PVOID)ObsBuf, sizeof(ObsBuf), &bytesret))
		{
			Ctx = NULL;

			break;
		}

		try
		{
			Obs = new nudf::util::CObligations;
		}
		catch (std::bad_alloc exec)
		{
			Obs = NULL;
		}

		if (Obs)
			Obs->FromBlob(ObsBuf, bytesret);

		set_dwm_active_session(Global.Section, Global.SessionId);

	} while (FALSE);

	return Obs;
}

static BOOL IsWndVisible(ULONG hWnd)
{
	BOOL bRet = FALSE;

	ULONG_PTR WndAttr = 0;

	do 
	{
#ifdef _AMD64_
		WndAttr = g_fnorg_GetWindowLongPtrW((HWND)(ULONG_PTR)hWnd, GWL_STYLE);
#else
		WndAttr = g_fnorg_GetWindowLongW((HWND)(ULONG_PTR)hWnd, GWL_STYLE);
#endif
		if (WndAttr & WS_VISIBLE)
		{
			bRet = TRUE;
		}

	} while (FALSE);

	return bRet;
}

//////////////////////////////////////////////////////////////////////////
//
// Windows 10 related
//
//////////////////////////////////////////////////////////////////////////

HRESULT WINAPI Core_CreateDXGIFactory1(REFIID riid, void **ppFactory)
{
	HRESULT hr = S_OK;

	IDXGIFactory *pFactory = NULL;

	IUnknown *pIDXGIFactoryDWM1 = NULL;

	PVOID pIDXGIFactoryDWMVtbl = NULL;

	ULONG_PTR fn_CreateSwapChain = 0;

	do
	{
		hr = g_fn_CreateDXGIFactory1_trampoline(riid, (void**)&pFactory);

		if (SUCCEEDED(hr) && pFactory)
		{
			hr = pFactory->QueryInterface(IID_IDXGIFactoryDWM1, (void**)&pIDXGIFactoryDWM1);

			if (SUCCEEDED(hr) && pIDXGIFactoryDWM1)
			{
				pIDXGIFactoryDWMVtbl = *((void**)pIDXGIFactoryDWM1);

				fn_CreateSwapChain = *(ULONG_PTR*)((UCHAR*)pIDXGIFactoryDWMVtbl + sizeof(ULONG_PTR) * 3);

				EnterCriticalSection(&Global.CreateSwapChainLock);

				do 
				{
					//
					// g_fnorg_IDXGIFactoryDWM1_CreateSwapChain is NOT NULL means we hooked
					// CreateSwapChain already
					//
					if (g_fnorg_IDXGIFactoryDWM1_CreateSwapChain)
					{
						break;
					}

					g_fnorg_IDXGIFactoryDWM1_CreateSwapChain = (IDXGIFACTORYDWM1_CREATESWAPCHAINDWM)(fn_CreateSwapChain);

					if (g_fnorg_IDXGIFactoryDWM1_CreateSwapChain)
					{
						if (!install_hook(g_fnorg_IDXGIFactoryDWM1_CreateSwapChain, (PVOID*)&g_fn_IDXGIFactoryDWM1_CreateSwapChain_trampoline, Core_CreateSwapChainDWM1))
						{
							g_fnorg_IDXGIFactoryDWM1_CreateSwapChain = NULL;
						}
					}

				} while (FALSE);

				LeaveCriticalSection(&Global.CreateSwapChainLock);

				pIDXGIFactoryDWM1->Release();
				pIDXGIFactoryDWM1 = NULL;
			}
		}

		*ppFactory = pFactory;

	} while (FALSE);

	return hr;
}

HRESULT WINAPI Core_CreateSwapChainDWM1(
	_In_ IDXGIFactory1	*pIDXGIFactory1,
	_In_ IUnknown *pDevice,
	_In_ DXGI_SWAP_CHAIN_DESC1 *pSwapChainDesc1,
	_In_ DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pSwapChainFullScreenDesc1,
	_In_ IDXGIOutput * pOutput,
	_Out_ IDXGISwapChainDWM1 ** ppSwapChainDWM1
	)
{
	HRESULT hr = S_OK;

	IDXGISwapChainDWM1 *pSwapChainDWM1 = NULL;

	PVOID pIDXGISwapChainDWM1Vtbl = NULL;

	ULONG_PTR fn_PresentDWM = 0;

	ID3D11Device *p3d11Device = NULL;

	hr = g_fn_IDXGIFactoryDWM1_CreateSwapChain_trampoline(pIDXGIFactory1, pDevice, pSwapChainDesc1, pSwapChainFullScreenDesc1, pOutput, &pSwapChainDWM1);

	do 
	{
		if (SUCCEEDED(hr) && pSwapChainDWM1)
		{
			pIDXGISwapChainDWM1Vtbl = *((void**)pSwapChainDWM1);

			fn_PresentDWM = *(ULONG_PTR*)((UCHAR*)pIDXGISwapChainDWM1Vtbl + sizeof(ULONG_PTR) * 16);

			EnterCriticalSection(&Global.PresentDWMLock);

			do 
			{
				if (g_fnorg_IDXGISwapChainDWM1_PresentDWM)
				{
					break;
				}

				g_fnorg_IDXGISwapChainDWM1_PresentDWM = (IDXGISWAPCHAINDWM1_PRESENTDWM)fn_PresentDWM;

				if (g_fnorg_IDXGISwapChainDWM1_PresentDWM)
				{
					if (!install_hook(g_fnorg_IDXGISwapChainDWM1_PresentDWM, (PVOID*)&g_fn_IDXGISwapChainDWM1_PresentDWM_trampoline, Core_IDXGISwapChainDWM1_PresentDWM))
					{
						g_fnorg_IDXGISwapChainDWM1_PresentDWM = NULL;
					}
				}

			} while (FALSE);

			LeaveCriticalSection(&Global.PresentDWMLock);

			pDevice->QueryInterface(__uuidof(ID3D11Device), (void**)&p3d11Device);

			if (p3d11Device)
			{
				D3D11CTX *pCtx = NULL;
				LIST_ENTRY *ite = NULL;

				pCtx = CreateD3d11Ctx(pSwapChainDWM1, p3d11Device);

				if (pCtx)
				{
					EnterCriticalSection(&Global.D3D11GlobalLock);
					FOR_EACH_LIST(ite, &D3D11GlobalCtxList)
					{
						D3D11CTX *pNode = CONTAINING_RECORD(ite, D3D11CTX, Link);

						if (pNode->pIDXISwapChainDWM1 == pSwapChainDWM1)
						{
							CleanupD3D11Ctx(pCtx);

							free(pCtx);
							pCtx = NULL;
							break;
						}
					}

					if (pCtx) {
						InsertHeadList(&D3D11GlobalCtxList, &pCtx->Link);
					}
					LeaveCriticalSection(&Global.D3D11GlobalLock);

					UpdateAllD3d11CtxIfServiceIsReady();
				}
			}
		}

		*ppSwapChainDWM1 = pSwapChainDWM1;

	} while (FALSE);

	if (p3d11Device)
	{
		p3d11Device->Release();
		p3d11Device = NULL;
	}

	return hr;
}

HRESULT WINAPI Core_IDXGISwapChainDWM1_PresentDWM(
	_In_ IDXGISwapChainDWM1 *pSwapChainDWM1,
	_In_ UINT SyncInterval,
	_In_ UINT Flags,
	_In_ UINT  DirtyRectsCount,
	_In_ RECT  *pDirtyRects,
	_In_ RECT  *pScrollRect,
	_In_ POINT *pScrollOffset,
	_In_ IDXGIResource *pResource,
	_In_ PVOID p)
{
	D3D11CTX *pNode = NULL;
	LIST_ENTRY *ite = NULL;

	static ULONG c = 0;

	do 
	{
		if (Flags & DXGI_PRESENT_TEST)
		{
			break;
		}

		EnterCriticalSection(&Global.D3D11GlobalLock);

		FOR_EACH_LIST(ite, &D3D11GlobalCtxList)
		{
			pNode = CONTAINING_RECORD(ite, D3D11CTX, Link);

			if (pNode->pIDXISwapChainDWM1 == pSwapChainDWM1)
			{
				break;
			}
			else
			{
				pNode = NULL;
			}
		}

		LeaveCriticalSection(&Global.D3D11GlobalLock);

		//
		// it's OK to do it outside the lock
		//
		if (pNode)
		{
			myrender1(pNode, DirtyRectsCount, pDirtyRects);
		}

	} while (FALSE);

	return g_fn_IDXGISwapChainDWM1_PresentDWM_trampoline(pSwapChainDWM1, SyncInterval, Flags, DirtyRectsCount, pDirtyRects, pScrollRect, pScrollOffset, pResource, p);
}

HRESULT WINAPI CreateD3D11TextureFromFile(
	_In_ ID3D11Device* d3dDevice,
	_In_z_ const WCHAR* FileName,
	_Out_opt_ ID3D11Resource** texture,
	_Out_opt_ ID3D11ShaderResourceView** textureView,
	_Out_opt_ ULONG *Width,
	_Out_opt_ ULONG *Height)
{
	HRESULT hr = S_OK;

	IWICBitmapDecoder		*pDecoder = NULL;
	IWICBitmapFrameDecode	*pSource = NULL;

	UINT uiWidth = 0;
	UINT uiHeight = 0;

	UCHAR *pRawPixels = NULL;

	UINT uiRowPitch = 0;
	UINT uiImageSize = 0;

	ID3D11Texture2D* tex = NULL;

	D3D11_TEXTURE2D_DESC desc = { 0 };

	D3D11_SUBRESOURCE_DATA initData = { 0 };

	do
	{
		if (!Global.WICFactory)
		{
			hr = E_UNEXPECTED;
			break;
		}

		hr = Global.WICFactory->CreateDecoderFromFilename(FileName,
														  NULL,
														  GENERIC_READ,
														  WICDecodeMetadataCacheOnLoad,
														  &pDecoder);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = pDecoder->GetFrame(0, &pSource);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pSource->GetSize(&uiWidth, &uiHeight);

		if (uiWidth > 4096 || uiHeight > 4096)
		{
			float ar = static_cast<float>(uiHeight) / static_cast<float>(uiWidth);

			if (uiWidth > uiHeight)
			{
				uiWidth = static_cast<UINT>(4096);
				uiHeight = static_cast<UINT>(static_cast<float>(4096) * ar);
			}
			else
			{
				uiHeight = static_cast<UINT>(4096);
				uiWidth = static_cast<UINT>(static_cast<float>(4096) / ar);
			}
		}

		uiRowPitch = (uiWidth * 32 + 7) / 8;
		uiImageSize = uiRowPitch * uiHeight;

		pRawPixels = (UCHAR*)malloc(uiImageSize);

		if (!pRawPixels)
		{
			hr = E_OUTOFMEMORY;
			break;
		}

		hr = pSource->CopyPixels(0,
								 uiRowPitch,
								 uiImageSize,
								 pRawPixels);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		// Create texture

		desc.Width = uiWidth;
		desc.Height = uiHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;


		initData.pSysMem = pRawPixels;
		initData.SysMemPitch = uiRowPitch;
		initData.SysMemSlicePitch = uiImageSize;


		hr = d3dDevice->CreateTexture2D(&desc,
										&initData,
										&tex);

		if (!SUCCEEDED(hr) || tex == 0)
		{
			break;
		}

		if (textureView != 0)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;

			memset(&SRVDesc, 0, sizeof(SRVDesc));

			SRVDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			SRVDesc.Texture2D.MipLevels = 1;

			hr = d3dDevice->CreateShaderResourceView(tex,
													 &SRVDesc,
													 textureView);

			if (!SUCCEEDED(hr))
			{
				tex->Release();
				tex = NULL;
				break;
			}
		}

		if (texture != 0)
		{
			*texture = tex;
			tex = NULL;		// prevent tex from being released
		}

		if (Width)
		{
			*Width = uiWidth;
		}

		if (Height)
		{
			*Height = uiHeight;
		}

	} while (FALSE);

	if (pSource)
	{
		pSource->Release();
		pSource = NULL;
	}

	if (pDecoder)
	{
		pDecoder->Release();
		pDecoder = NULL;
	}

	if (pRawPixels)
	{
		free(pRawPixels);
		pRawPixels = NULL;
	}

	if (tex)
	{
		tex->Release();
		tex = NULL;
	}

	return hr;
}

static HRESULT InitializeD3D11Ctx(D3D11CTX *pCtx, const WCHAR *OverLayFileName)
{
	HRESULT hr = S_OK;

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	OverLayVertex vertices[] =
	{
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f),  DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC bd = { 0 };
	D3D11_SUBRESOURCE_DATA InitData = { 0 };

	D3D11_SAMPLER_DESC sampDesc;

	D3D11_BLEND_DESC blendStateDescription = { 0 };

	OverLayPixelParameters1 PixelParams1;

	OverLayVertexParameters VertexParams = { DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) };

	D3D11_VIEWPORT vp = { 0 };

	float blendFactor[4] = { 0.0f };

	ID3D11Texture2D *pBackBuffer = NULL;

	D3D11_TEXTURE2D_DESC backBufferSurfaceDesc = { 0 };

	ULONG TexWidth = 0;
	ULONG TexHeight = 0;

	IDXGISwapChainDWM1 *pSwapChain = pCtx->pIDXISwapChainDWM1; 
	
	ID3D11Device *pd3dDevice = pCtx->pd3d11Device;
	
	do
	{
		pd3dDevice->GetImmediateContext(&pCtx->pImmediateContext);

		hr = D3D11CreateDeviceStateBlock(pCtx->pImmediateContext, &pCtx->pOrignalStateBlock);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		hr = D3D11CreateDeviceStateBlock(pCtx->pImmediateContext, &pCtx->pCoreStateBlock);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pCtx->pOrignalStateBlock->Capture();

		hr = pSwapChain->GetBuffer(0, __uuidof(*pBackBuffer), (LPVOID*)&pBackBuffer);

		if (FAILED(hr))
		{
			break;
		}

		pCtx->pImmediateContext->ClearState();

		pBackBuffer->GetDesc(&backBufferSurfaceDesc);

		pCtx->ScreenWidth = backBufferSurfaceDesc.Width;
		pCtx->ScreenHeight = backBufferSurfaceDesc.Height;

		// Set view port
		ZeroMemory(&vp, sizeof(vp));
		vp.Width = (FLOAT)pCtx->ScreenWidth;
		vp.Height = (FLOAT)pCtx->ScreenHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;

		pCtx->pImmediateContext->RSSetViewports(1, &vp);

		hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &pCtx->pRenderTargetView);

		if (FAILED(hr))
		{
			break;
		}

		pCtx->pImmediateContext->OMSetRenderTargets(1, &pCtx->pRenderTargetView, NULL);

		// Create the vertex shader
		hr = pd3dDevice->CreateVertexShader(g_OverlayVsshader, sizeof(g_OverlayVsshader), NULL, &pCtx->pVertexShader);

		if (FAILED(hr))
		{
			break;
		}

		// Create the input layout
		hr = pd3dDevice->CreateInputLayout(layout,
										   numElements,
										   g_OverlayVsshader,
										   sizeof(g_OverlayVsshader),
										   &pCtx->pVertexLayout);

		if (FAILED(hr))
		{
			break;
		}

		// Create the pixel shader
		hr = pd3dDevice->CreatePixelShader(g_Overlaypsshader1, sizeof(g_Overlaypsshader1), NULL, &pCtx->pPixelShader);

		if (FAILED(hr))
		{
			break;
		}

		// Create vertex buffer
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(OverLayVertex) * 6;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = vertices;

		hr = pd3dDevice->CreateBuffer(&bd, &InitData, &pCtx->pVertexBuffer);

		if (FAILED(hr))
		{
			break;
		}

		// Load the Texture
		hr = CreateD3D11TextureFromFile(pd3dDevice, OverLayFileName, NULL, &pCtx->pTextureRV, &TexWidth, &TexHeight);

		if (FAILED(hr))
		{
			break;
		}

		// Create the sample state
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = pd3dDevice->CreateSamplerState(&sampDesc, &pCtx->pSamplerLinear);

		if (FAILED(hr))
		{
			break;
		}

		ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

		blendStateDescription.AlphaToCoverageEnable = FALSE;
		blendStateDescription.IndependentBlendEnable = FALSE;

		// only have one RenderTarget
		blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
		blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		hr = pd3dDevice->CreateBlendState(&blendStateDescription, &pCtx->palphaEnableBlendingState);

		if (FAILED(hr))
		{
			break;
		}

		// Create the constant buffers
		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(OverLayPixelParameters1);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;

		hr = pd3dDevice->CreateBuffer(&bd, NULL, &pCtx->pOverLayPixelParameters);

		if (FAILED(hr))
		{
			break;
		}

		ZeroMemory(&bd, sizeof(bd));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(OverLayVertexParameters);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;

		hr = pd3dDevice->CreateBuffer(&bd, NULL, &pCtx->pOverLayVertexParameters);

		if (FAILED(hr))
		{
			break;
		}

		memset(&PixelParams1, 0, sizeof(PixelParams1));

		PixelParams1.ClipRect = DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, -1.0f);
		PixelParams1.Opacity = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, ((float)Global.Opacity / 100.0f));

		for (ULONG i = 0; i < 8; i++)
		{
			PixelParams1.DirtyRects[i] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}

		PixelParams1.DirtyRects[0] = DirectX::XMFLOAT4(-1.0f, 1.0f, 1.0f, -1.0f);

		pCtx->pImmediateContext->UpdateSubresource(pCtx->pOverLayPixelParameters, 0, NULL, &PixelParams1, 0, 0);

		if (TexHeight == 0 || TexWidth == 0)
		{
			VertexParams.Tex2Screen = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		else
		{
			VertexParams.Tex2Screen = DirectX::XMFLOAT4((float)pCtx->ScreenWidth / (float)TexWidth,
														(float)pCtx->ScreenHeight / (float)TexHeight,
														0.0f,
														0.0f);
		}

		pCtx->pImmediateContext->UpdateSubresource(pCtx->pOverLayVertexParameters, 0, NULL, &VertexParams, 0, 0);

		// Set the input layout
		pCtx->pImmediateContext->IASetInputLayout(pCtx->pVertexLayout);

		// Set primitive topology 
		pCtx->pImmediateContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Turn on the alpha blending.
		pCtx->pImmediateContext->OMSetBlendState(pCtx->palphaEnableBlendingState, blendFactor, 0xffffffff);

		// Set vertex buffer
		UINT stride = sizeof(OverLayVertex);
		UINT offset = 0;
		pCtx->pImmediateContext->IASetVertexBuffers(0, 1, &pCtx->pVertexBuffer, &stride, &offset);

		// Render a triangle
		pCtx->pImmediateContext->VSSetShader(pCtx->pVertexShader, NULL, 0);
		pCtx->pImmediateContext->PSSetShader(pCtx->pPixelShader, NULL, 0);

		pCtx->pImmediateContext->PSSetShaderResources(0, 1, &pCtx->pTextureRV);
		pCtx->pImmediateContext->PSSetConstantBuffers(1, 1, &pCtx->pOverLayPixelParameters);

		pCtx->pImmediateContext->VSSetConstantBuffers(0, 1, &pCtx->pOverLayVertexParameters);

		pCtx->pImmediateContext->PSSetSamplers(0, 1, &pCtx->pSamplerLinear);

		pCtx->pCoreStateBlock->Capture();

	} while (FALSE);

	if (pBackBuffer)
	{
		pBackBuffer->Release();
		pBackBuffer = NULL;
	}

	if (pCtx->pOrignalStateBlock)
	{
		pCtx->pOrignalStateBlock->Apply();
	}

	return hr;
}

static void CleanupD3D11Ctx(D3D11CTX *pCtx)
{
	if (pCtx->pOrignalStateBlock)
	{
		delete pCtx->pOrignalStateBlock;
		pCtx->pOrignalStateBlock = NULL;
	}

	if (pCtx->pCoreStateBlock)
	{
		delete pCtx->pCoreStateBlock;
		pCtx->pCoreStateBlock = NULL;
	}

	if (pCtx->pImmediateContext)
	{
		pCtx->pImmediateContext->Release();
		pCtx->pImmediateContext = NULL;
	}

	if (pCtx->pRenderTargetView)
	{
		pCtx->pRenderTargetView->Release();
		pCtx->pRenderTargetView = NULL;
	}

	if (pCtx->pVertexShader)
	{
		pCtx->pVertexShader->Release();
		pCtx->pVertexShader = NULL;
	}

	if (pCtx->pPixelShader)
	{
		pCtx->pPixelShader->Release();
		pCtx->pPixelShader = NULL;
	}

	if (pCtx->pVertexLayout)
	{
		pCtx->pVertexLayout->Release();
		pCtx->pVertexLayout = NULL;
	}

	if (pCtx->pVertexBuffer)
	{
		pCtx->pVertexBuffer->Release();
		pCtx->pVertexBuffer = NULL;
	}

	if (pCtx->pOverLayPixelParameters)
	{
		pCtx->pOverLayPixelParameters->Release();
		pCtx->pOverLayPixelParameters = NULL;
	}

	if (pCtx->pOverLayVertexParameters)
	{
		pCtx->pOverLayVertexParameters->Release();
		pCtx->pOverLayVertexParameters = NULL;
	}

	if (pCtx->pTextureRV)
	{
		pCtx->pTextureRV->Release();
		pCtx->pTextureRV = NULL;
	}

	if (pCtx->pSamplerLinear)
	{
		pCtx->pSamplerLinear->Release();
		pCtx->pSamplerLinear = NULL;
	}

	if (pCtx->palphaEnableBlendingState)
	{
		pCtx->palphaEnableBlendingState->Release();
		pCtx->palphaEnableBlendingState = NULL;
	}
}

BOOL InitializedWindowsCodesHook(void)
{
	BOOL bRet = TRUE;
	
	do 
	{
		if (!Global.WindowscodecsHandle)
		{
			Global.WindowscodecsHandle = GetModuleHandleW(WINDOWSCODECS_MODULE_NAME);
		}

		if (!Global.WindowscodecsHandle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.WindowscodecsHookInitialized)
		{
			break;
		}

		g_fnorg_WICCreateImagingFactory_Proxy = (WICCREATEIMAGINGFACTORY_PROXY)GetProcAddress(Global.WindowscodecsHandle, DIRECT3D_WICCREATEIMAGINGFACTORY_PROXY_PROC_NAME);

		if (g_fnorg_WICCreateImagingFactory_Proxy)
		{
			if (!install_hook(g_fnorg_WICCreateImagingFactory_Proxy, (PVOID*)&g_fn_WICCreateImagingFactory_Proxy_trampoline, Core_WICCreateImagingFactory_Proxy))
			{
				bRet = FALSE;
				break;
			}
		}

		Global.WindowscodecsHookInitialized = TRUE;

	} while (FALSE);
	
	return bRet;
}

void CleanupWindowsCodesHook(void)
{
	//
	// DWM cleanup should take care of these already
	//
	if (g_fn_WICCreateImagingFactory_Proxy_trampoline)
	{
		remove_hook(g_fn_WICCreateImagingFactory_Proxy_trampoline);
		g_fn_WICCreateImagingFactory_Proxy_trampoline = NULL;
	}
}

static void myrender1(
	_In_ D3D11CTX				*pCtx,
	_In_ UINT					DirtyRectsCount,
	_In_ RECT					*pDirtyRects
	)
{
	RECT rc = { 0 };
	OverLayPixelParameters1 PixelParams1;

	BOOL bInvalidAllWindows = FALSE;

	IDXGIOutput1 *pDXGIOutput1 = NULL;

	HRESULT hr = S_OK;

	DXGI_OUTPUT_DESC OutputDesc = { 0 };

	do
	{
		if (pCtx->palphaEnableBlendingState == NULL ||
			pCtx->pVertexLayout == NULL ||
			pCtx->pVertexBuffer == NULL ||
			pCtx->pVertexShader == NULL ||
			pCtx->pPixelShader == NULL ||
			pCtx->pTextureRV == NULL ||
			pCtx->pSamplerLinear == NULL ||
			pCtx->pOrignalStateBlock == NULL ||
			pCtx->pCoreStateBlock == NULL ||
			pCtx->pImmediateContext == NULL ||
			pCtx->pIDXISwapChainDWM1 == NULL ||
			g_fnorg_GetWindowRect == NULL ||
			DirtyRectsCount == 0 ||
			Global.hWnds[0] == NULL /* don't pay performance tax if there is no dirty window */)
		{
			break;
		}

		hr = pCtx->pIDXISwapChainDWM1->GetContainingOutput(&pDXGIOutput1);

		if (!SUCCEEDED(hr) || pDXGIOutput1 == NULL)
		{
			break;
		}

		hr = pDXGIOutput1->GetDesc(&OutputDesc);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pCtx->pOrignalStateBlock->Capture();

		pCtx->pCoreStateBlock->Apply1();

		for (ULONG i = 0; i < sizeof(Global.hWnds) / sizeof(Global.hWnds[0]); i++)
		{
			if (Global.hWnds[i] == NULL)
			{
				break;
			}

			if (Global.WndsAttr[i] == 0xffffffff)
			{
				continue;
			}

			if (!g_fnorg_GetWindowRect((HWND)(ULONG_PTR)Global.hWnds[i], &rc))
			{
				Global.WndsAttr[i] = 0xffffffff;
				
				if (!bInvalidAllWindows) 
					bInvalidAllWindows = TRUE;

				continue;
			}

			rc.top -= OutputDesc.DesktopCoordinates.top;
			rc.left -= OutputDesc.DesktopCoordinates.left;
			rc.bottom = pCtx->ScreenHeight - (OutputDesc.DesktopCoordinates.bottom - rc.bottom);
			rc.right = pCtx->ScreenWidth - (OutputDesc.DesktopCoordinates.right - rc.right);

			rc.top += 5;
			rc.left += 5;
			rc.bottom -= 5;
			rc.right -= 5;

			memset(&PixelParams1, 0, sizeof(PixelParams1));

			PixelParams1.ClipRect = DirectX::XMFLOAT4(((float)(rc.left * 2) / (float)pCtx->ScreenWidth) - 1.0f,
													 1.0f - ((float)(rc.top * 2) / (float)pCtx->ScreenHeight),
													 ((float)(rc.right * 2) / (float)pCtx->ScreenWidth) - 1.0f,
													 1.0f - ((float)(rc.bottom * 2) / (float)pCtx->ScreenHeight));

			PixelParams1.Opacity = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, ((float)Global.Opacity / 100.0f));
			
			for (ULONG j = 0; j < 8; j++)
			{
				PixelParams1.DirtyRects[j] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			}

			for (ULONG k = 0; k < min(DirtyRectsCount, 8); k++)
			{
				PixelParams1.DirtyRects[k] = DirectX::XMFLOAT4(((float)(pDirtyRects[k].left * 2) / (float)pCtx->ScreenWidth) - 1.0f,
															   1.0f - ((float)(pDirtyRects[k].top * 2) / (float)pCtx->ScreenHeight),
															   ((float)(pDirtyRects[k].right * 2) / (float)pCtx->ScreenWidth) - 1.0f,
															   1.0f - ((float)(pDirtyRects[k].bottom * 2) / (float)pCtx->ScreenHeight));
			}

			pCtx->pImmediateContext->UpdateSubresource(pCtx->pOverLayPixelParameters, 0, NULL, &PixelParams1, 0, 0);

			pCtx->pImmediateContext->Draw(6, 0);
		}

		pCtx->pOrignalStateBlock->Apply();

	} while (FALSE);

	if (bInvalidAllWindows && g_fnorg_RedrawWindow)
	{
		g_fnorg_RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
	}

	if (pDXGIOutput1)
	{
		pDXGIOutput1->Release();
		pDXGIOutput1 = NULL;
	}

	return;
}

DWORD WINAPI Core_GetRegionData1(
	_In_ HRGN											hrgn,
	_In_ DWORD											nCount,
	_Out_writes_bytes_to_opt_(nCount, return) LPRGNDATA lpRgnData)
{
	ULONG cbSize = sizeof(Global.hWnds);

	LONG PolicySN = 0;

	BOOL bIsPreviousWndListEmpty = TRUE;

	BOOL bInvalidDesktopWnd = FALSE;

	do
	{
		if (lpRgnData == NULL)
		{
			break;
		}

		if (!init_rm_section_safe())
		{
			break;
		}

		if (get_dwm_active_session(Global.Section) != Global.SessionId)
		{
			set_dwm_active_session(Global.Section, Global.SessionId);
		}

		UpdateAllD3d11CtxIfServiceIsReady();

		PolicySN = get_rm_policy_sn(Global.Section);

		if (Global.LastPolicySN != PolicySN)
		{
			//
			// no lock required because DWM use single thread to get region data and render
			// the place (myrender) that need to access Global.hWnds is in the same thread
			//
			bIsPreviousWndListEmpty = (Global.hWnds[0] == NULL) ? TRUE : FALSE;

			memset(Global.hWnds, 0, sizeof(Global.hWnds));

			memset(Global.WndsAttr, 0, sizeof(Global.WndsAttr));

			if (get_overlay_windows(Global.Section, Global.hWnds, &cbSize))
			{
				Global.LastPolicySN = PolicySN;
			}

			if (!bIsPreviousWndListEmpty && Global.hWnds[0] == NULL && g_fnorg_RedrawWindow)
			{
				bInvalidDesktopWnd = TRUE;
			}

			//
			// scan all hWnds to make sure there is no hidden Window
			//
			for (ULONG i = 0; i < sizeof(Global.hWnds) / sizeof(Global.hWnds[0]); i++)
			{
				if (Global.hWnds[i])
				{
					if (!IsWndVisible(Global.hWnds[i]))
					{
						Global.WndsAttr[i] = 0xffffffff;
					}
					else
					{
						Global.WndsAttr[i] = 0;
					}
				}
				else
				{
					Global.WndsAttr[i] = 0xffffffff;
				}
			}

			if (bInvalidDesktopWnd)
			{
				g_fnorg_RedrawWindow(NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
			}
		}
		else
		{
			//
			// scan all hWnds to make sure there is no hidden Window
			//
			for (ULONG i = 0; i < sizeof(Global.hWnds) / sizeof(Global.hWnds[0]); i++)
			{
				if (Global.hWnds[i] && Global.WndsAttr[i] == 0xffffffff)
				{
					//
					// check again on those previously hidden 
					//
					if (!IsWndVisible(Global.hWnds[i]))
					{
						Global.WndsAttr[i] = 0xffffffff;
					}
					else
					{
						Global.WndsAttr[i] = 0;
					}
				}
			}
		}

	} while (FALSE);

	return g_fn_GetRegionData_trampoline(hrgn, nCount, lpRgnData);
}

static void UpdateAllD3d11CtxIfServiceIsReady(void)
{
	nudf::util::CObligations *Obligations = NULL;
	nudf::util::CObligation OverlayOb;

	std::wstring BitmapFileName;
	std::wstring Opacity;

	HRESULT hr = S_OK;

	D3D11CTX *pCtx = NULL;

	LIST_ENTRY *ite = NULL;

	do
	{
		if (!init_rm_section_safe())
		{
			break;
		}

		if (!is_overlay_bitmap_ready(Global.Section))
		{
			break;
		}


		EnterCriticalSection(&Global.D3D11GlobalLock);
		FOR_EACH_LIST(ite, &D3D11GlobalCtxList)
		{
			pCtx = CONTAINING_RECORD(ite, D3D11CTX, Link);
				
			if (!pCtx->Initialized)
			{
				LeaveCriticalSection(&Global.D3D11GlobalLock);
				if (!BitmapFileName.length())
				{
					do
					{
						Obligations = query_engine();

						if (!Obligations)
						{
							break;
						}

						if (Obligations->IsEmpty())
						{
							break;
						}

						OverlayOb = (Obligations->GetObligations())[OB_ID_OVERLAY];

						BitmapFileName = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_IMAGE];

						if (!BitmapFileName.length())
						{
							break;
						}

						Opacity = (OverlayOb.GetParams())[OB_OVERLAY_PARAM_TRANSPARENCY];

						if (Opacity.length())
						{
							Global.Opacity = std::wcstol(Opacity.c_str(), NULL, 10);
						}
						else
						{
							Global.Opacity = 10;
						}

					} while (FALSE);
				}
				EnterCriticalSection(&Global.D3D11GlobalLock);
				if (BitmapFileName.length() && !pCtx->Initialized)//make sure initialized flag is not changed after leaveCriticialSection
				{
					hr = InitializeD3D11Ctx(pCtx, BitmapFileName.c_str());

					if (SUCCEEDED(hr))
					{
						pCtx->Initialized = TRUE;
					}
				}
			}
		}
		LeaveCriticalSection(&Global.D3D11GlobalLock);

	} while (FALSE);

	return;
}

static D3D11CTX* CreateD3d11Ctx(IDXGISwapChainDWM1 *pSwapChain, ID3D11Device *pd3dDevice)
{
	D3D11CTX *pCtx = NULL;

	do 
	{
		pCtx = (D3D11CTX*)malloc(sizeof(D3D11CTX));

		if (pCtx)
		{
			memset(pCtx, 0, sizeof(D3D11CTX));

			pCtx->pd3d11Device			= pd3dDevice;
			pCtx->pIDXISwapChainDWM1	= pSwapChain;
			pCtx->Initialized			= FALSE;
		}

	} while (FALSE);

	return pCtx;
}
