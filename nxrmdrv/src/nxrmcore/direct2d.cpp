#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "nxrmdrv.h"
#include "nxrmcorehlp.h"
#include "detour.h"
#include "direct2d.h"

#ifdef __cplusplus
extern "C" {
#endif

	extern						CORE_GLOBAL_DATA Global;

#ifdef __cplusplus
}
#endif

static D2D1CREATEFACTORY	g_fnorg_D2D1CreateFactory = NULL;
static D2D1CREATEFACTORY	g_fn_D2D1CreateFactory_trampoline = NULL;
static D2D1MAKEROTATEMATRIX g_fnorg_D2D1MakeRotateMatrix = NULL;

static D2D1FORCEINLINE
	D2D1::Matrix3x2F
	Core_Rotation(
	FLOAT angle,
	D2D1_POINT_2F center = D2D1::Point2F()
	);

BOOL InitializeD2D1Hook(void)
{
	BOOL bRet = TRUE;

	do 
	{
		if (!Global.d2d1Handle)
		{
			Global.d2d1Handle = GetModuleHandleW(D2D1_MODULE_NAME);
		}

		if (!Global.d2d1Handle)
		{
			bRet = FALSE;
			break;
		}

		if (Global.D2D1HooksInitialized)
		{
			break;
		}

		g_fnorg_D2D1CreateFactory = (D2D1CREATEFACTORY)GetProcAddress(Global.d2d1Handle, DIRECT2D_D2D1CREATEFACTORY_PROC_NAME);

		if (g_fnorg_D2D1CreateFactory)
		{
			if (!install_hook(g_fnorg_D2D1CreateFactory, (PVOID*)&g_fn_D2D1CreateFactory_trampoline, Core_D2D1CreateFactory))
			{
				bRet = FALSE;
				break;
			}
		}

		g_fnorg_D2D1MakeRotateMatrix = (D2D1MAKEROTATEMATRIX)GetProcAddress(Global.d2d1Handle, DIRECT2D_D2D1MAKEROTATEMATRIX_PROC_NAME);

		Global.D2D1HooksInitialized = TRUE;

	} while (FALSE);

	return bRet;

}

void CleanupD2D1Hook(void)
{
	if (g_fn_D2D1CreateFactory_trampoline)
	{
		remove_hook(g_fn_D2D1CreateFactory_trampoline);
		g_fn_D2D1CreateFactory_trampoline = NULL;
	}
}

HRESULT WINAPI Core_D2D1CreateFactory(
	_In_ D2D1_FACTORY_TYPE factoryType, 
	_In_ REFIID riid, 
	_In_opt_ CONST D2D1_FACTORY_OPTIONS *pFactoryOptions, 
	_Out_ void **ppIFactory)
{
	HRESULT hr = S_OK;

	ID2D1Factory *pFactory = NULL;
	CoreID2D1Factory *pCoreFactory = NULL;

	do 
	{
		hr = g_fn_D2D1CreateFactory_trampoline(factoryType, riid, pFactoryOptions, (void**)&pFactory);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		if (riid == IID_ID2D1Factory)
		{
			pCoreFactory = new CoreID2D1Factory(pFactory);

			*ppIFactory = (ID2D1Factory*)pCoreFactory;
		}
		else
		{
			*ppIFactory = pFactory;
		}

	} while (FALSE);

	return hr;
}

CoreID2D1Factory::CoreID2D1Factory()
{
	m_pID2D1Factory = NULL;
	m_uRefCount = 1;
}

CoreID2D1Factory::CoreID2D1Factory(ID2D1Factory *Factory)
{
	m_pID2D1Factory = Factory;
	m_uRefCount = 1;
}

CoreID2D1Factory::~CoreID2D1Factory()
{
	if (m_pID2D1Factory)
	{
		m_pID2D1Factory->Release();
		m_pID2D1Factory = NULL;
	}
}

STDMETHODIMP CoreID2D1Factory::QueryInterface(REFIID riid, void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if(IID_IUnknown == riid)
		{
			punk = (IUnknown *)this;
		}
		else if (IID_ID2D1Factory == riid)
		{
			punk = (ID2D1Factory*)this;
		}
		else
		{
			hRet = m_pID2D1Factory->QueryInterface(riid, ppobj);
			break;
		}

		AddRef();

		*ppobj = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP_(ULONG) CoreID2D1Factory::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreID2D1Factory::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount)
	{
		delete this;
	}

	return uCount;
}

STDMETHODIMP CoreID2D1Factory::ReloadSystemMetrics()
{
	return m_pID2D1Factory->ReloadSystemMetrics();
}

STDMETHODIMP_(void) CoreID2D1Factory::GetDesktopDpi(
	_Out_ FLOAT *dpiX,
	_Out_ FLOAT *dpiY 
	)
{
	m_pID2D1Factory->GetDesktopDpi(dpiX, dpiY);
}

STDMETHODIMP CoreID2D1Factory::CreateRectangleGeometry(
	_In_ CONST D2D1_RECT_F *rectangle,
	_Outptr_ ID2D1RectangleGeometry **rectangleGeometry 
	)
{
	return m_pID2D1Factory->CreateRectangleGeometry(rectangle, rectangleGeometry);
}

STDMETHODIMP CoreID2D1Factory::CreateRoundedRectangleGeometry(
	_In_ CONST D2D1_ROUNDED_RECT *roundedRectangle,
	_Outptr_ ID2D1RoundedRectangleGeometry **roundedRectangleGeometry 
	)
{
	return m_pID2D1Factory->CreateRoundedRectangleGeometry(roundedRectangle, roundedRectangleGeometry);
}

STDMETHODIMP CoreID2D1Factory::CreateEllipseGeometry(
	_In_ CONST D2D1_ELLIPSE *ellipse,
	_Outptr_ ID2D1EllipseGeometry **ellipseGeometry 
	)
{
	return m_pID2D1Factory->CreateEllipseGeometry(ellipse, ellipseGeometry);
}

STDMETHODIMP CoreID2D1Factory::CreateGeometryGroup(
	D2D1_FILL_MODE fillMode,
	_In_reads_(geometriesCount) ID2D1Geometry **geometries,
	UINT32 geometriesCount,
	_Outptr_ ID2D1GeometryGroup **geometryGroup 
	)
{
	return m_pID2D1Factory->CreateGeometryGroup(fillMode, geometries, geometriesCount, geometryGroup);
}

STDMETHODIMP CoreID2D1Factory::CreateTransformedGeometry(
	_In_ ID2D1Geometry *sourceGeometry,
	_In_ CONST D2D1_MATRIX_3X2_F *transform,
	_Outptr_ ID2D1TransformedGeometry **transformedGeometry 
	)
{
	return m_pID2D1Factory->CreateTransformedGeometry(sourceGeometry, transform, transformedGeometry);
}

STDMETHODIMP CoreID2D1Factory::CreatePathGeometry(
	_Outptr_ ID2D1PathGeometry **pathGeometry 
	)
{
	return m_pID2D1Factory->CreatePathGeometry(pathGeometry);
}

STDMETHODIMP CoreID2D1Factory::CreateStrokeStyle(
	_In_ CONST D2D1_STROKE_STYLE_PROPERTIES *strokeStyleProperties,
	_In_reads_opt_(dashesCount) CONST FLOAT *dashes,
	UINT32 dashesCount,
	_Outptr_ ID2D1StrokeStyle **strokeStyle 
	)
{
	return m_pID2D1Factory->CreateStrokeStyle(strokeStyleProperties, dashes, dashesCount, strokeStyle);
}

STDMETHODIMP CoreID2D1Factory::CreateDrawingStateBlock(
	_In_opt_ CONST D2D1_DRAWING_STATE_DESCRIPTION *drawingStateDescription,
	_In_opt_ IDWriteRenderingParams *textRenderingParams,
	_Outptr_ ID2D1DrawingStateBlock **drawingStateBlock 
	)
{
	return m_pID2D1Factory->CreateDrawingStateBlock(drawingStateDescription, textRenderingParams, drawingStateBlock);
}

STDMETHODIMP CoreID2D1Factory::CreateWicBitmapRenderTarget(
	_In_ IWICBitmap *target,
	_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
	_Outptr_ ID2D1RenderTarget **renderTarget 
	)
{
	HRESULT hr = S_OK;
	
	ID2D1RenderTarget *pRenderTarget = NULL;
	CoreID2D1RenderTarget *pCoreRenderTarget = NULL;

	do 
	{
		hr = m_pID2D1Factory->CreateWicBitmapRenderTarget(target, renderTargetProperties, &pRenderTarget);

		if (!SUCCEEDED(hr))
		{
			break;
		}

		pCoreRenderTarget = new CoreID2D1RenderTarget(pRenderTarget, this);

		*renderTarget = pCoreRenderTarget;

	} while (FALSE);

	return hr;
}

STDMETHODIMP CoreID2D1Factory::CreateHwndRenderTarget(
	_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
	_In_ CONST D2D1_HWND_RENDER_TARGET_PROPERTIES *hwndRenderTargetProperties,
	_Outptr_ ID2D1HwndRenderTarget **hwndRenderTarget 
	)
{
	return m_pID2D1Factory->CreateHwndRenderTarget(renderTargetProperties, hwndRenderTargetProperties, hwndRenderTarget);
}

STDMETHODIMP CoreID2D1Factory::CreateDxgiSurfaceRenderTarget(
	_In_ IDXGISurface *dxgiSurface,
	_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
	_Outptr_ ID2D1RenderTarget **renderTarget 
	)
{
	return m_pID2D1Factory->CreateDxgiSurfaceRenderTarget(dxgiSurface, renderTargetProperties, renderTarget);
}

STDMETHODIMP CoreID2D1Factory::CreateDCRenderTarget(
	_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
	_Outptr_ ID2D1DCRenderTarget **dcRenderTarget 
	)
{
	return m_pID2D1Factory->CreateDCRenderTarget(renderTargetProperties, dcRenderTarget);
}

CoreID2D1RenderTarget::CoreID2D1RenderTarget()
{
	m_uRefCount = 1;
	m_pID2D1RenderTarget = NULL;
	m_pBitMap = NULL;
	m_pCoreID2D1Factory = NULL;
	m_bIsTextWindow = FALSE;
	m_pWatermarkBrush = NULL;
}

CoreID2D1RenderTarget::CoreID2D1RenderTarget(ID2D1RenderTarget *RenderTarget, CoreID2D1Factory *Factory)
{
	HRESULT hr = S_OK;

	double dpiX = 96.0f; 
	double dpiY = 96.0f; 

	m_uRefCount = 1;
	m_pID2D1RenderTarget = RenderTarget;
	m_pCoreID2D1Factory = Factory;
	m_pBitMap = NULL;
	m_bIsTextWindow = FALSE;
	m_pWatermarkBrush = NULL;

	EnterCriticalSection(&Global.WICCoverterLock);

	do 
	{
		if (!Global.WICCoverter)
		{
			break;
		}

		Global.WICCoverter->GetResolution(&dpiX, &dpiY);

		hr = RenderTarget->CreateBitmapFromWicBitmap(Global.WICCoverter,
													 D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), static_cast<float>(dpiX), static_cast<float>(dpiY)),
													 &m_pBitMap);
		
		if (FAILED(hr))
		{
			break;
		}

		// Create the bitmap brush.
		hr = m_pID2D1RenderTarget->CreateBitmapBrush(m_pBitMap,
													 D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR),
													 D2D1::BrushProperties(0.3f),
													 &m_pWatermarkBrush);

	} while (FALSE);

	LeaveCriticalSection(&Global.WICCoverterLock);

	m_pID2D1RenderTarget->QueryInterface(IID_ID2D1DeviceContext, (void**)&m_pID2D1DeviceContext);
}

CoreID2D1RenderTarget::~CoreID2D1RenderTarget()
{
	if (m_pID2D1RenderTarget)
	{
		m_pID2D1RenderTarget->Release();
		m_pID2D1RenderTarget = NULL;
	}

	if (m_pBitMap)
	{
		m_pBitMap->Release();
		m_pBitMap = NULL;
	}

	if (m_pWatermarkBrush)
	{
		m_pWatermarkBrush->Release();
		m_pWatermarkBrush = NULL;
	}

	if (m_pID2D1DeviceContext)
	{
		m_pID2D1DeviceContext->Release();
		m_pID2D1DeviceContext = NULL;
	}

	m_pCoreID2D1Factory = NULL;
}

STDMETHODIMP CoreID2D1RenderTarget::QueryInterface(REFIID riid, void **ppobj)
{
	HRESULT hRet = S_OK;

	void *punk = NULL;

	*ppobj = NULL;

	do 
	{
		if(IID_IUnknown == riid)
		{
			punk = (IUnknown *)this;
		}
		else if (IID_ID2D1RenderTarget == riid)
		{
			punk = (ID2D1RenderTarget*)this;
		}
		else
		{
			hRet = m_pID2D1RenderTarget->QueryInterface(riid, ppobj);
			break;
		}

		AddRef();

		*ppobj = punk;

	} while (FALSE);

	return hRet;
}

STDMETHODIMP_(ULONG) CoreID2D1RenderTarget::AddRef()
{
	m_uRefCount++;

	return m_uRefCount;
}

STDMETHODIMP_(ULONG) CoreID2D1RenderTarget::Release()
{
	ULONG uCount = 0;

	if(m_uRefCount)
		m_uRefCount--;

	uCount = m_uRefCount;

	if(!uCount)
	{
		delete this;
	}

	return uCount;
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::GetFactory(
	_Outptr_ ID2D1Factory **factory 
	) const
{
	//m_pCoreID2D1Factory->AddRef();

	//*factory = (ID2D1Factory*)m_pCoreID2D1Factory;
	m_pID2D1RenderTarget->GetFactory(factory);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateBitmap(
	D2D1_SIZE_U size,
	_In_opt_ CONST void *srcData,
	UINT32 pitch,
	_In_ CONST D2D1_BITMAP_PROPERTIES *bitmapProperties,
	_Outptr_ ID2D1Bitmap **bitmap 
	)
{
	return m_pID2D1RenderTarget->CreateBitmap(size, srcData, pitch, bitmapProperties, bitmap);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateBitmapFromWicBitmap(
	_In_ IWICBitmapSource *wicBitmapSource,
	_In_opt_ CONST D2D1_BITMAP_PROPERTIES *bitmapProperties,
	_Outptr_ ID2D1Bitmap **bitmap 
	)
{
	return m_pID2D1RenderTarget->CreateBitmapFromWicBitmap(wicBitmapSource, bitmapProperties, bitmap);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateSharedBitmap(
	_In_ REFIID riid,
	_Inout_ void *data,
	_In_opt_ CONST D2D1_BITMAP_PROPERTIES *bitmapProperties,
	_Outptr_ ID2D1Bitmap **bitmap 
	)
{
	return m_pID2D1RenderTarget->CreateSharedBitmap(riid, data, bitmapProperties, bitmap);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateBitmapBrush(
	_In_opt_ ID2D1Bitmap *bitmap,
	_In_opt_ CONST D2D1_BITMAP_BRUSH_PROPERTIES *bitmapBrushProperties,
	_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
	_Outptr_ ID2D1BitmapBrush **bitmapBrush 
	)
{
	return m_pID2D1RenderTarget->CreateBitmapBrush(bitmap, bitmapBrushProperties, brushProperties, bitmapBrush);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateSolidColorBrush(
	_In_ CONST D2D1_COLOR_F *color,
	_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
	_Outptr_ ID2D1SolidColorBrush **solidColorBrush 
	)
{
	return m_pID2D1RenderTarget->CreateSolidColorBrush(color, brushProperties, solidColorBrush);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateGradientStopCollection(
	_In_reads_(gradientStopsCount) CONST D2D1_GRADIENT_STOP *gradientStops,
	_In_range_(>=,1) UINT32 gradientStopsCount,
	D2D1_GAMMA colorInterpolationGamma,
	D2D1_EXTEND_MODE extendMode,
	_Outptr_ ID2D1GradientStopCollection **gradientStopCollection 
	)
{
	return m_pID2D1RenderTarget->CreateGradientStopCollection(gradientStops, gradientStopsCount, colorInterpolationGamma, extendMode, gradientStopCollection);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateLinearGradientBrush(
	_In_ CONST D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES *linearGradientBrushProperties,
	_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
	_In_ ID2D1GradientStopCollection *gradientStopCollection,
	_Outptr_ ID2D1LinearGradientBrush **linearGradientBrush 
	)
{
	return m_pID2D1RenderTarget->CreateLinearGradientBrush(linearGradientBrushProperties, brushProperties, gradientStopCollection, linearGradientBrush);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateRadialGradientBrush(
	_In_ CONST D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES *radialGradientBrushProperties,
	_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
	_In_ ID2D1GradientStopCollection *gradientStopCollection,
	_Outptr_ ID2D1RadialGradientBrush **radialGradientBrush 
	)
{
	return m_pID2D1RenderTarget->CreateRadialGradientBrush(radialGradientBrushProperties, brushProperties, gradientStopCollection, radialGradientBrush);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateCompatibleRenderTarget(
	_In_opt_ CONST D2D1_SIZE_F *desiredSize,
	_In_opt_ CONST D2D1_SIZE_U *desiredPixelSize,
	_In_opt_ CONST D2D1_PIXEL_FORMAT *desiredFormat,
	D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS options,
	_Outptr_ ID2D1BitmapRenderTarget **bitmapRenderTarget 
	)
{
	return m_pID2D1RenderTarget->CreateCompatibleRenderTarget(desiredSize, desiredPixelSize, desiredFormat, options, bitmapRenderTarget);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateLayer(
	_In_opt_ CONST D2D1_SIZE_F *size,
	_Outptr_ ID2D1Layer **layer 
	)
{
	return m_pID2D1RenderTarget->CreateLayer(size, layer);
}

STDMETHODIMP CoreID2D1RenderTarget::CreateMesh(
	_Outptr_ ID2D1Mesh **mesh 
	)
{
	return m_pID2D1RenderTarget->CreateMesh(mesh);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawLine(
	D2D1_POINT_2F point0,
	D2D1_POINT_2F point1,
	_In_ ID2D1Brush *brush,
	FLOAT strokeWidth,
	_In_opt_ ID2D1StrokeStyle *strokeStyle 
	)
{
	m_pID2D1RenderTarget->DrawLine(point0, point1, brush, strokeWidth, strokeStyle);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawRectangle(
	_In_ CONST D2D1_RECT_F *rect,
	_In_ ID2D1Brush *brush,
	FLOAT strokeWidth,
	_In_opt_ ID2D1StrokeStyle *strokeStyle 
	)
{
	m_pID2D1RenderTarget->DrawRectangle(rect, brush, strokeWidth, strokeStyle);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::FillRectangle(
	_In_ CONST D2D1_RECT_F *rect,
	_In_ ID2D1Brush *brush 
	)
{
	m_pID2D1RenderTarget->FillRectangle(rect, brush);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawRoundedRectangle(
	_In_ CONST D2D1_ROUNDED_RECT *roundedRect,
	_In_ ID2D1Brush *brush,
	FLOAT strokeWidth,
	_In_opt_ ID2D1StrokeStyle *strokeStyle 
	)
{
	m_pID2D1RenderTarget->DrawRoundedRectangle(roundedRect, brush, strokeWidth, strokeStyle);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::FillRoundedRectangle(
	_In_ CONST D2D1_ROUNDED_RECT *roundedRect,
	_In_ ID2D1Brush *brush 
	)
{
	m_pID2D1RenderTarget->FillRoundedRectangle(roundedRect, brush);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawEllipse(
	_In_ CONST D2D1_ELLIPSE *ellipse,
	_In_ ID2D1Brush *brush,
	FLOAT strokeWidth,
	_In_opt_ ID2D1StrokeStyle *strokeStyle 
	)
{
	m_pID2D1RenderTarget->DrawEllipse(ellipse, brush, strokeWidth, strokeStyle);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::FillEllipse(
	_In_ CONST D2D1_ELLIPSE *ellipse,
	_In_ ID2D1Brush *brush 
	)
{
	m_pID2D1RenderTarget->FillEllipse(ellipse, brush);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawGeometry(
	_In_ ID2D1Geometry *geometry,
	_In_ ID2D1Brush *brush,
	FLOAT strokeWidth,
	_In_opt_ ID2D1StrokeStyle *strokeStyle 
	)
{
	m_pID2D1RenderTarget->DrawGeometry(geometry, brush, strokeWidth, strokeStyle);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::FillGeometry(
	_In_ ID2D1Geometry *geometry,
	_In_ ID2D1Brush *brush,
	_In_opt_ ID2D1Brush *opacityBrush 
	)
{
	m_pID2D1RenderTarget->FillGeometry(geometry, brush, opacityBrush);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::FillMesh(
	_In_ ID2D1Mesh *mesh,
	_In_ ID2D1Brush *brush 
	)
{
	m_pID2D1RenderTarget->FillMesh(mesh, brush);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::FillOpacityMask(
	_In_ ID2D1Bitmap *opacityMask,
	_In_ ID2D1Brush *brush,
	D2D1_OPACITY_MASK_CONTENT content,
	_In_opt_ CONST D2D1_RECT_F *destinationRectangle,
	_In_opt_ CONST D2D1_RECT_F *sourceRectangle 
	)
{
	m_pID2D1RenderTarget->FillOpacityMask(opacityMask, brush, content, destinationRectangle, sourceRectangle);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawBitmap(
	_In_ ID2D1Bitmap *bitmap,
	_In_opt_ CONST D2D1_RECT_F *destinationRectangle,
	FLOAT opacity,
	D2D1_BITMAP_INTERPOLATION_MODE interpolationMode,
	_In_opt_ CONST D2D1_RECT_F *sourceRectangle 
	)
{
	m_pID2D1RenderTarget->DrawBitmap(bitmap, destinationRectangle, opacity, interpolationMode, sourceRectangle);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawText(
	_In_reads_(stringLength) CONST WCHAR *string,
	UINT32 stringLength,
	_In_ IDWriteTextFormat *textFormat,
	_In_ CONST D2D1_RECT_F *layoutRect,
	_In_ ID2D1Brush *defaultForegroundBrush,
	D2D1_DRAW_TEXT_OPTIONS options,
	DWRITE_MEASURING_MODE measuringMode 
	)
{
	m_pID2D1RenderTarget->DrawText(string, stringLength, textFormat, layoutRect, defaultForegroundBrush, options, measuringMode);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawTextLayout(
	D2D1_POINT_2F origin,
	_In_ IDWriteTextLayout *textLayout,
	_In_ ID2D1Brush *defaultForegroundBrush,
	D2D1_DRAW_TEXT_OPTIONS options 
	)
{
	m_pID2D1RenderTarget->DrawTextLayout(origin, textLayout, defaultForegroundBrush, options);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::DrawGlyphRun(
	D2D1_POINT_2F baselineOrigin,
	_In_ CONST DWRITE_GLYPH_RUN *glyphRun,
	_In_ ID2D1Brush *foregroundBrush,
	DWRITE_MEASURING_MODE measuringMode 
	)
{
	if (!m_bIsTextWindow)
		m_bIsTextWindow = TRUE;

	m_pID2D1RenderTarget->DrawGlyphRun(baselineOrigin, glyphRun, foregroundBrush, measuringMode);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::SetTransform(
	_In_ CONST D2D1_MATRIX_3X2_F *transform 
	)
{
	m_pID2D1RenderTarget->SetTransform(transform);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::GetTransform(
	_Out_ D2D1_MATRIX_3X2_F *transform 
	) const
{
	m_pID2D1RenderTarget->GetTransform(transform);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::SetAntialiasMode(
	D2D1_ANTIALIAS_MODE antialiasMode 
	)
{
	m_pID2D1RenderTarget->SetAntialiasMode(antialiasMode);
}

STDMETHODIMP_(D2D1_ANTIALIAS_MODE) CoreID2D1RenderTarget::GetAntialiasMode(void) const
{
	return m_pID2D1RenderTarget->GetAntialiasMode();
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::SetTextAntialiasMode(
	D2D1_TEXT_ANTIALIAS_MODE textAntialiasMode 
	)
{
	m_pID2D1RenderTarget->SetTextAntialiasMode(textAntialiasMode);
}

STDMETHODIMP_(D2D1_TEXT_ANTIALIAS_MODE) CoreID2D1RenderTarget::GetTextAntialiasMode(void) const
{
	return m_pID2D1RenderTarget->GetTextAntialiasMode();
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::SetTextRenderingParams(
	_In_opt_ IDWriteRenderingParams *textRenderingParams 
	)
{
	m_pID2D1RenderTarget->SetTextRenderingParams(textRenderingParams);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::GetTextRenderingParams(
	_Outptr_result_maybenull_ IDWriteRenderingParams **textRenderingParams 
	) const
{
	m_pID2D1RenderTarget->GetTextRenderingParams(textRenderingParams);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::SetTags(
	D2D1_TAG tag1,
	D2D1_TAG tag2 
	)
{
	m_pID2D1RenderTarget->SetTags(tag1, tag2);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::GetTags(
	_Out_opt_ D2D1_TAG *tag1,
	_Out_opt_ D2D1_TAG *tag2 
	) const
{
	m_pID2D1RenderTarget->GetTags(tag1, tag2);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::PushLayer(
	_In_ CONST D2D1_LAYER_PARAMETERS *layerParameters,
	_In_opt_ ID2D1Layer *layer 
	)
{
	m_pID2D1RenderTarget->PushLayer(layerParameters, layer);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::PopLayer(void)
{
	m_pID2D1RenderTarget->PopLayer();
}

STDMETHODIMP CoreID2D1RenderTarget::Flush(
	_Out_opt_ D2D1_TAG *tag1,
	_Out_opt_ D2D1_TAG *tag2 
	)
{
	return m_pID2D1RenderTarget->Flush(tag1, tag2);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::SaveDrawingState(
	_Inout_ ID2D1DrawingStateBlock *drawingStateBlock 
	) const
{
	m_pID2D1RenderTarget->SaveDrawingState(drawingStateBlock);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::RestoreDrawingState(
	_In_ ID2D1DrawingStateBlock *drawingStateBlock 
	)
{
	m_pID2D1RenderTarget->RestoreDrawingState(drawingStateBlock);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::PushAxisAlignedClip(
	_In_ CONST D2D1_RECT_F *clipRect,
	D2D1_ANTIALIAS_MODE antialiasMode 
	)
{
	m_pID2D1RenderTarget->PushAxisAlignedClip(clipRect, antialiasMode);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::PopAxisAlignedClip(void)
{
	m_pID2D1RenderTarget->PopAxisAlignedClip();
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::Clear(
	_In_opt_ CONST D2D1_COLOR_F *clearColor 
	)
{
	m_pID2D1RenderTarget->Clear(clearColor);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::BeginDraw(void)
{
	m_pID2D1RenderTarget->BeginDraw();
}

STDMETHODIMP CoreID2D1RenderTarget::EndDraw(
	_Out_opt_ D2D1_TAG *tag1,
	_Out_opt_ D2D1_TAG *tag2 
	)
{
	HRESULT hr = S_OK;

	hr = m_pID2D1RenderTarget->EndDraw(tag1, tag2);

	if (m_pWatermarkBrush && m_bIsTextWindow && m_pID2D1DeviceContext)
	{
		// Retrieve the size of the render target.
		D2D1_SIZE_F renderTargetSize = m_pID2D1DeviceContext->GetSize();

		m_pID2D1DeviceContext->BeginDraw();

		m_pID2D1DeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

		// Paint a grid background.
		m_pID2D1DeviceContext->FillRectangle(D2D1::RectF(0.f, 0.f, renderTargetSize.width, renderTargetSize.height),
											 m_pWatermarkBrush);
			
		m_pID2D1DeviceContext->EndDraw();
	}

	return hr;
}

STDMETHODIMP_(D2D1_PIXEL_FORMAT) CoreID2D1RenderTarget::GetPixelFormat(void) const
{
	return m_pID2D1RenderTarget->GetPixelFormat();
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::SetDpi(
	FLOAT dpiX,
	FLOAT dpiY 
	)
{
	m_pID2D1RenderTarget->SetDpi(dpiX, dpiY);
}

STDMETHODIMP_(void) CoreID2D1RenderTarget::GetDpi(
	_Out_ FLOAT *dpiX,
	_Out_ FLOAT *dpiY 
	) const
{
	m_pID2D1RenderTarget->GetDpi(dpiX, dpiY);
}

STDMETHODIMP_(D2D1_SIZE_F) CoreID2D1RenderTarget::GetSize(void) const
{
	return m_pID2D1RenderTarget->GetSize();
}

STDMETHODIMP_(D2D1_SIZE_U) CoreID2D1RenderTarget::GetPixelSize(void) const
{
	return m_pID2D1RenderTarget->GetPixelSize();
}

STDMETHODIMP_(UINT32) CoreID2D1RenderTarget::GetMaximumBitmapSize(void) const
{
	return m_pID2D1RenderTarget->GetMaximumBitmapSize();
}

STDMETHODIMP_(BOOL) CoreID2D1RenderTarget::IsSupported(
	_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties 
	) const
{
	return m_pID2D1RenderTarget->IsSupported(renderTargetProperties);
}

static D2D1FORCEINLINE
	D2D1::Matrix3x2F
	Core_Rotation(
	FLOAT angle,
	D2D1_POINT_2F center)
{
	D2D1::Matrix3x2F rotation;

	g_fnorg_D2D1MakeRotateMatrix(angle, center, &rotation);

	return rotation;
}

//CoreID2D1DeviceContext::CoreID2D1DeviceContext()
//{
//	m_pCoreID2D1RenderTarget = NULL;
//	m_pID2D1DeviceContext = NULL;
//	m_uRefCount = 1;
//}
//
//CoreID2D1DeviceContext::CoreID2D1DeviceContext(ID2D1DeviceContext *pDeviceCtx, CoreID2D1RenderTarget *pRenderTarget)
//{
//	m_pCoreID2D1RenderTarget	= pRenderTarget;
//	m_pID2D1DeviceContext		= pDeviceCtx;
//	m_uRefCount					= 1;
//}
//
//CoreID2D1DeviceContext::~CoreID2D1DeviceContext()
//{
//	if (m_pID2D1DeviceContext)
//	{
//		m_pID2D1DeviceContext->Release();
//		m_pID2D1DeviceContext = NULL;
//	}
//
//	m_pCoreID2D1RenderTarget = NULL;
//}
//
//STDMETHODIMP CoreID2D1DeviceContext::QueryInterface(REFIID riid, void **ppobj)
//{
//	HRESULT hRet = S_OK;
//
//	void *punk = NULL;
//
//	*ppobj = NULL;
//
//	do 
//	{
//		if(IID_IUnknown == riid)
//		{
//			punk = (IUnknown *)this;
//		}
//		else if (IID_ID2D1DeviceContext == riid)
//		{
//			punk = (ID2D1DeviceContext*)this;
//		}
//		else
//		{
//			hRet = m_pID2D1DeviceContext->QueryInterface(riid, ppobj);
//			break;
//		}
//
//		AddRef();
//
//		*ppobj = punk;
//
//	} while (FALSE);
//
//	return hRet;
//}
//
//STDMETHODIMP_(ULONG) CoreID2D1DeviceContext::AddRef()
//{
//	m_uRefCount++;
//
//	return m_uRefCount;
//}
//
//STDMETHODIMP_(ULONG) CoreID2D1DeviceContext::Release()
//{
//	ULONG uCount = 0;
//
//	if(m_uRefCount)
//		m_uRefCount--;
//
//	uCount = m_uRefCount;
//
//	if(!uCount)
//	{
//		delete this;
//	}
//
//	return uCount;
//}