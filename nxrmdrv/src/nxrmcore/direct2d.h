#pragma once

#include <d2d1_1.h>
#include <d2d1helper.h>

#define DIRECT2D_D2D1CREATEFACTORY_PROC_NAME	"D2D1CreateFactory"
#define DIRECT2D_D2D1MAKEROTATEMATRIX_PROC_NAME	"D2D1MakeRotateMatrix"

class CoreID2D1Factory : public ID2D1Factory
{
public:
	CoreID2D1Factory();
	CoreID2D1Factory(ID2D1Factory *Factory);
	~CoreID2D1Factory();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppobj);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP ReloadSystemMetrics();

	STDMETHODIMP_(void) GetDesktopDpi(
		_Out_ FLOAT *dpiX,
		_Out_ FLOAT *dpiY 
		);

	STDMETHODIMP CreateRectangleGeometry(
		_In_ CONST D2D1_RECT_F *rectangle,
		_Outptr_ ID2D1RectangleGeometry **rectangleGeometry 
		);

	STDMETHODIMP CreateRoundedRectangleGeometry(
		_In_ CONST D2D1_ROUNDED_RECT *roundedRectangle,
		_Outptr_ ID2D1RoundedRectangleGeometry **roundedRectangleGeometry 
		);

	STDMETHODIMP CreateEllipseGeometry(
		_In_ CONST D2D1_ELLIPSE *ellipse,
		_Outptr_ ID2D1EllipseGeometry **ellipseGeometry 
		);

	STDMETHODIMP CreateGeometryGroup(
		D2D1_FILL_MODE fillMode,
		_In_reads_(geometriesCount) ID2D1Geometry **geometries,
		UINT32 geometriesCount,
		_Outptr_ ID2D1GeometryGroup **geometryGroup 
		);

	STDMETHODIMP CreateTransformedGeometry(
		_In_ ID2D1Geometry *sourceGeometry,
		_In_ CONST D2D1_MATRIX_3X2_F *transform,
		_Outptr_ ID2D1TransformedGeometry **transformedGeometry 
		);

	STDMETHODIMP CreatePathGeometry(
		_Outptr_ ID2D1PathGeometry **pathGeometry 
		);

	STDMETHODIMP CreateStrokeStyle(
		_In_ CONST D2D1_STROKE_STYLE_PROPERTIES *strokeStyleProperties,
		_In_reads_opt_(dashesCount) CONST FLOAT *dashes,
		UINT32 dashesCount,
		_Outptr_ ID2D1StrokeStyle **strokeStyle 
		);

	STDMETHODIMP CreateDrawingStateBlock(
		_In_opt_ CONST D2D1_DRAWING_STATE_DESCRIPTION *drawingStateDescription,
		_In_opt_ IDWriteRenderingParams *textRenderingParams,
		_Outptr_ ID2D1DrawingStateBlock **drawingStateBlock 
		);

	STDMETHODIMP CreateWicBitmapRenderTarget(
		_In_ IWICBitmap *target,
		_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
		_Outptr_ ID2D1RenderTarget **renderTarget 
		);

	STDMETHODIMP CreateHwndRenderTarget(
		_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
		_In_ CONST D2D1_HWND_RENDER_TARGET_PROPERTIES *hwndRenderTargetProperties,
		_Outptr_ ID2D1HwndRenderTarget **hwndRenderTarget 
		);

	STDMETHODIMP CreateDxgiSurfaceRenderTarget(
		_In_ IDXGISurface *dxgiSurface,
		_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
		_Outptr_ ID2D1RenderTarget **renderTarget 
		);

	STDMETHODIMP CreateDCRenderTarget(
		_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties,
		_Outptr_ ID2D1DCRenderTarget **dcRenderTarget 
		);

private:
	ULONG			m_uRefCount;
	ID2D1Factory	*m_pID2D1Factory;
};

class CoreID2D1RenderTarget : public ID2D1RenderTarget
{
public:
	CoreID2D1RenderTarget();
	CoreID2D1RenderTarget(ID2D1RenderTarget *RenderTarget, CoreID2D1Factory *Factory);
	~CoreID2D1RenderTarget();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppobj);

	STDMETHODIMP_(ULONG) AddRef();

	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP_(void) GetFactory(
		_Outptr_ ID2D1Factory **factory 
		) const;

	STDMETHODIMP CreateBitmap(
		D2D1_SIZE_U size,
		_In_opt_ CONST void *srcData,
		UINT32 pitch,
		_In_ CONST D2D1_BITMAP_PROPERTIES *bitmapProperties,
		_Outptr_ ID2D1Bitmap **bitmap 
		);

	STDMETHODIMP CreateBitmapFromWicBitmap(
		_In_ IWICBitmapSource *wicBitmapSource,
		_In_opt_ CONST D2D1_BITMAP_PROPERTIES *bitmapProperties,
		_Outptr_ ID2D1Bitmap **bitmap 
		);

	STDMETHODIMP CreateSharedBitmap(
		_In_ REFIID riid,
		_Inout_ void *data,
		_In_opt_ CONST D2D1_BITMAP_PROPERTIES *bitmapProperties,
		_Outptr_ ID2D1Bitmap **bitmap 
		);

	STDMETHODIMP CreateBitmapBrush(
		_In_opt_ ID2D1Bitmap *bitmap,
		_In_opt_ CONST D2D1_BITMAP_BRUSH_PROPERTIES *bitmapBrushProperties,
		_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
		_Outptr_ ID2D1BitmapBrush **bitmapBrush 
		);

	STDMETHODIMP CreateSolidColorBrush(
		_In_ CONST D2D1_COLOR_F *color,
		_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
		_Outptr_ ID2D1SolidColorBrush **solidColorBrush 
		);

	STDMETHODIMP CreateGradientStopCollection(
		_In_reads_(gradientStopsCount) CONST D2D1_GRADIENT_STOP *gradientStops,
		_In_range_(>=,1) UINT32 gradientStopsCount,
		D2D1_GAMMA colorInterpolationGamma,
		D2D1_EXTEND_MODE extendMode,
		_Outptr_ ID2D1GradientStopCollection **gradientStopCollection 
		);

	STDMETHODIMP CreateLinearGradientBrush(
		_In_ CONST D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES *linearGradientBrushProperties,
		_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
		_In_ ID2D1GradientStopCollection *gradientStopCollection,
		_Outptr_ ID2D1LinearGradientBrush **linearGradientBrush 
		);

	STDMETHODIMP CreateRadialGradientBrush(
		_In_ CONST D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES *radialGradientBrushProperties,
		_In_opt_ CONST D2D1_BRUSH_PROPERTIES *brushProperties,
		_In_ ID2D1GradientStopCollection *gradientStopCollection,
		_Outptr_ ID2D1RadialGradientBrush **radialGradientBrush 
		);

	STDMETHODIMP CreateCompatibleRenderTarget(
		_In_opt_ CONST D2D1_SIZE_F *desiredSize,
		_In_opt_ CONST D2D1_SIZE_U *desiredPixelSize,
		_In_opt_ CONST D2D1_PIXEL_FORMAT *desiredFormat,
		D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS options,
		_Outptr_ ID2D1BitmapRenderTarget **bitmapRenderTarget 
		);

	STDMETHODIMP CreateLayer(
		_In_opt_ CONST D2D1_SIZE_F *size,
		_Outptr_ ID2D1Layer **layer 
		);

	STDMETHODIMP CreateMesh(
		_Outptr_ ID2D1Mesh **mesh 
		);

	STDMETHODIMP_(void) DrawLine(
		D2D1_POINT_2F point0,
		D2D1_POINT_2F point1,
		_In_ ID2D1Brush *brush,
		FLOAT strokeWidth,
		_In_opt_ ID2D1StrokeStyle *strokeStyle 
		);

	STDMETHODIMP_(void) DrawRectangle(
		_In_ CONST D2D1_RECT_F *rect,
		_In_ ID2D1Brush *brush,
		FLOAT strokeWidth,
		_In_opt_ ID2D1StrokeStyle *strokeStyle 
		);

	STDMETHODIMP_(void) FillRectangle(
		_In_ CONST D2D1_RECT_F *rect,
		_In_ ID2D1Brush *brush 
		);

	STDMETHODIMP_(void) DrawRoundedRectangle(
		_In_ CONST D2D1_ROUNDED_RECT *roundedRect,
		_In_ ID2D1Brush *brush,
		FLOAT strokeWidth,
		_In_opt_ ID2D1StrokeStyle *strokeStyle 
		);

	STDMETHODIMP_(void) FillRoundedRectangle(
		_In_ CONST D2D1_ROUNDED_RECT *roundedRect,
		_In_ ID2D1Brush *brush 
		);

	STDMETHODIMP_(void) DrawEllipse(
		_In_ CONST D2D1_ELLIPSE *ellipse,
		_In_ ID2D1Brush *brush,
		FLOAT strokeWidth,
		_In_opt_ ID2D1StrokeStyle *strokeStyle 
		);

	STDMETHODIMP_(void) FillEllipse(
		_In_ CONST D2D1_ELLIPSE *ellipse,
		_In_ ID2D1Brush *brush 
		);

	STDMETHODIMP_(void) DrawGeometry(
		_In_ ID2D1Geometry *geometry,
		_In_ ID2D1Brush *brush,
		FLOAT strokeWidth,
		_In_opt_ ID2D1StrokeStyle *strokeStyle 
		);

	STDMETHODIMP_(void) FillGeometry(
		_In_ ID2D1Geometry *geometry,
		_In_ ID2D1Brush *brush,
		_In_opt_ ID2D1Brush *opacityBrush 
		);

	STDMETHODIMP_(void) FillMesh(
		_In_ ID2D1Mesh *mesh,
		_In_ ID2D1Brush *brush 
		);

	STDMETHODIMP_(void) FillOpacityMask(
		_In_ ID2D1Bitmap *opacityMask,
		_In_ ID2D1Brush *brush,
		D2D1_OPACITY_MASK_CONTENT content,
		_In_opt_ CONST D2D1_RECT_F *destinationRectangle,
		_In_opt_ CONST D2D1_RECT_F *sourceRectangle 
		);

	STDMETHODIMP_(void) DrawBitmap(
		_In_ ID2D1Bitmap *bitmap,
		_In_opt_ CONST D2D1_RECT_F *destinationRectangle,
		FLOAT opacity,
		D2D1_BITMAP_INTERPOLATION_MODE interpolationMode,
		_In_opt_ CONST D2D1_RECT_F *sourceRectangle 
		);

	STDMETHODIMP_(void) DrawText(
		_In_reads_(stringLength) CONST WCHAR *string,
		UINT32 stringLength,
		_In_ IDWriteTextFormat *textFormat,
		_In_ CONST D2D1_RECT_F *layoutRect,
		_In_ ID2D1Brush *defaultForegroundBrush,
		D2D1_DRAW_TEXT_OPTIONS options,
		DWRITE_MEASURING_MODE measuringMode 
		);

	STDMETHODIMP_(void) DrawTextLayout(
		D2D1_POINT_2F origin,
		_In_ IDWriteTextLayout *textLayout,
		_In_ ID2D1Brush *defaultForegroundBrush,
		D2D1_DRAW_TEXT_OPTIONS options 
		);

	STDMETHODIMP_(void) DrawGlyphRun(
		D2D1_POINT_2F baselineOrigin,
		_In_ CONST DWRITE_GLYPH_RUN *glyphRun,
		_In_ ID2D1Brush *foregroundBrush,
		DWRITE_MEASURING_MODE measuringMode 
		);

	STDMETHODIMP_(void) SetTransform(
		_In_ CONST D2D1_MATRIX_3X2_F *transform 
		);

	STDMETHODIMP_(void) GetTransform(
		_Out_ D2D1_MATRIX_3X2_F *transform 
		) const;

	STDMETHODIMP_(void) SetAntialiasMode(
		D2D1_ANTIALIAS_MODE antialiasMode 
		);

	STDMETHODIMP_(D2D1_ANTIALIAS_MODE) GetAntialiasMode(void) const;

	STDMETHODIMP_(void) SetTextAntialiasMode(
		D2D1_TEXT_ANTIALIAS_MODE textAntialiasMode 
		);

	STDMETHODIMP_(D2D1_TEXT_ANTIALIAS_MODE) GetTextAntialiasMode(void) const;

	STDMETHODIMP_(void) SetTextRenderingParams(
		_In_opt_ IDWriteRenderingParams *textRenderingParams 
		);

	STDMETHODIMP_(void) GetTextRenderingParams(
		_Outptr_result_maybenull_ IDWriteRenderingParams **textRenderingParams 
		) const;

	STDMETHODIMP_(void) SetTags(
		D2D1_TAG tag1,
		D2D1_TAG tag2 
		);

	STDMETHODIMP_(void) GetTags(
		_Out_opt_ D2D1_TAG *tag1,
		_Out_opt_ D2D1_TAG *tag2 
		) const;

	STDMETHODIMP_(void) PushLayer(
		_In_ CONST D2D1_LAYER_PARAMETERS *layerParameters,
		_In_opt_ ID2D1Layer *layer 
		);

	STDMETHODIMP_(void) PopLayer(void);

	STDMETHODIMP Flush(
		_Out_opt_ D2D1_TAG *tag1,
		_Out_opt_ D2D1_TAG *tag2 
		);

	STDMETHODIMP_(void) SaveDrawingState(
		_Inout_ ID2D1DrawingStateBlock *drawingStateBlock 
		) const;

	STDMETHODIMP_(void) RestoreDrawingState(
		_In_ ID2D1DrawingStateBlock *drawingStateBlock 
		);

	STDMETHODIMP_(void) PushAxisAlignedClip(
		_In_ CONST D2D1_RECT_F *clipRect,
		D2D1_ANTIALIAS_MODE antialiasMode 
		);

	STDMETHODIMP_(void) PopAxisAlignedClip(void);

	STDMETHODIMP_(void) Clear(
		_In_opt_ CONST D2D1_COLOR_F *clearColor 
		);

	STDMETHODIMP_(void) BeginDraw(void);

	STDMETHODIMP EndDraw(
		_Out_opt_ D2D1_TAG *tag1,
		_Out_opt_ D2D1_TAG *tag2 
		);

	STDMETHODIMP_(D2D1_PIXEL_FORMAT) GetPixelFormat(void) const;

	STDMETHODIMP_(void) SetDpi(
		FLOAT dpiX,
		FLOAT dpiY 
		);

	STDMETHODIMP_(void) GetDpi(
		_Out_ FLOAT *dpiX,
		_Out_ FLOAT *dpiY 
		) const;

	STDMETHODIMP_(D2D1_SIZE_F) GetSize(void) const;

	STDMETHODIMP_(D2D1_SIZE_U) GetPixelSize(void) const;

	STDMETHODIMP_(UINT32) GetMaximumBitmapSize(void) const;

	STDMETHODIMP_(BOOL) IsSupported(
		_In_ CONST D2D1_RENDER_TARGET_PROPERTIES *renderTargetProperties 
		) const;
private:

	ULONG				m_uRefCount;
	ID2D1RenderTarget	*m_pID2D1RenderTarget;
	ID2D1DeviceContext	*m_pID2D1DeviceContext;
	ID2D1Bitmap			*m_pBitMap;
	ID2D1BitmapBrush	*m_pWatermarkBrush;
	CoreID2D1Factory	*m_pCoreID2D1Factory;
	BOOL				m_bIsTextWindow;
};

class CoreID2D1DeviceContext : public ID2D1DeviceContext
{
public:
	CoreID2D1DeviceContext();
	CoreID2D1DeviceContext(ID2D1DeviceContext *pDeviceCtx, CoreID2D1RenderTarget *pRenderTarget);
	~CoreID2D1DeviceContext();

private:
	ULONG					m_uRefCount;
	ID2D1DeviceContext		*m_pID2D1DeviceContext;
	CoreID2D1RenderTarget	*m_pCoreID2D1RenderTarget;
};

typedef HRESULT (WINAPI	*D2D1CREATEFACTORY)(
	_In_ D2D1_FACTORY_TYPE				factoryType,
	_In_ REFIID							riid,
	_In_opt_ CONST D2D1_FACTORY_OPTIONS *pFactoryOptions,
	_Out_ void							**ppIFactory
	);

HRESULT WINAPI Core_D2D1CreateFactory(
	_In_ D2D1_FACTORY_TYPE				factoryType,
	_In_ REFIID							riid,
	_In_opt_ CONST D2D1_FACTORY_OPTIONS *pFactoryOptions,
	_Out_ void							**ppIFactory
	);

typedef void (WINAPI *D2D1MAKEROTATEMATRIX)(
	_In_ FLOAT angle,
	_In_ D2D1_POINT_2F center,
	_Out_ D2D1_MATRIX_3X2_F *matrix
	);

BOOL InitializeD2D1Hook(void);
void CleanupD2D1Hook(void);
