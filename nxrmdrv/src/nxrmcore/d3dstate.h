#pragma once

#include <dxgi.h>
#include <d3d11.h>

class D3D11StateBlock
{
public:
	D3D11StateBlock(ID3D11DeviceContext *pDeviceCtx);
	~D3D11StateBlock();

	STDMETHODIMP Capture(void);

	STDMETHODIMP Apply(void);

	STDMETHODIMP Apply1(void);

private:

	STDMETHODIMP ReleaseObjects(void);

private:

	ID3D11DeviceContext			*m_pDeviceContext;

	//
	// VS
	ID3D11VertexShader			*m_pVertexShader;
	ID3D11ShaderResourceView	*m_pVertexShaderRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	ID3D11Buffer				*m_pVertexConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	ID3D11SamplerState			*m_pVertexSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

	//
	// GS
	ID3D11GeometryShader		*m_pGeometryShader;
	ID3D11ShaderResourceView	*m_pGeometryShaderRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	ID3D11Buffer				*m_pGeometryConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	ID3D11SamplerState			*m_pGeometrySamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

	//
	// PS
	ID3D11PixelShader			*m_pPixelShader;
	ID3D11ShaderResourceView	*m_pPixelShaderRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	ID3D11Buffer				*m_pPixelConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	ID3D11SamplerState			*m_pPixelSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];

	//
	// IA
	ID3D11InputLayout			*m_pInputLayout;
	ID3D11Buffer				*m_pIndexBuffer;
	DXGI_FORMAT					m_DXGIFormat;
	UINT						m_uiOffset;
	D3D11_PRIMITIVE_TOPOLOGY	m_PrimitiveTopology;
	ID3D11Buffer				*m_pVertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT						m_uiStrides[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
	UINT						m_uiVertexBuffersOffsets[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];

	//
	// RS
	ID3D11RasterizerState		*m_pRasterizerState;
	UINT						m_uiNumViewports;
	D3D11_VIEWPORT				m_pViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	UINT						m_uiNumScissorRects;
	D3D11_RECT					m_pScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

	//
	// OM
	ID3D11RenderTargetView		*m_pRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
	ID3D11DepthStencilView		*m_pDepthStencilView;
	ID3D11BlendState			*m_pBlendState;
	float						m_fBlendFactor[4];
	UINT32						m_uiSampleMask;

	//
	// SO Buffer
	ID3D11Buffer				*m_pSOTargets[4];

	//
	// Predication
	BOOL						m_bPredicate;
	ID3D11Predicate				*m_pPredicate;
};

HRESULT WINAPI D3D11CreateDeviceStateBlock(
	_In_ ID3D11DeviceContext	*pDeviceCtx,
	_Out_ D3D11StateBlock		**ppStateBlock);