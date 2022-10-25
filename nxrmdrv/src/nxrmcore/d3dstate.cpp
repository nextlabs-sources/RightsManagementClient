#include "stdafx.h"
#include "nxrmcoreglobal.h"
#include "d3dstate.h"

D3D11StateBlock::D3D11StateBlock(ID3D11DeviceContext *pDeviceCtx) : m_pRasterizerState(NULL),\
																	m_uiSampleMask(0),\
																	m_DXGIFormat(DXGI_FORMAT_UNKNOWN),\
																	m_PrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED),\
																	m_pBlendState(NULL),\
																	m_pIndexBuffer(NULL),\
																	m_pInputLayout(NULL),\
																	m_uiOffset(0),\
																	m_pDepthStencilView(NULL),\
																	m_uiNumViewports(D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE),\
																	m_pVertexShader(NULL),\
																	m_pPixelShader(NULL),\
																	m_pGeometryShader(NULL),\
																	m_bPredicate(FALSE),\
																	m_pPredicate(NULL),\
																	m_uiNumScissorRects(0)
{
	ZeroMemory(m_pViewports, sizeof(m_pViewports));
	ZeroMemory(m_pRenderTargetViews, sizeof(m_pRenderTargetViews));
	ZeroMemory(m_fBlendFactor, sizeof(m_fBlendFactor));
	ZeroMemory(m_pVertexBuffers, sizeof(m_pVertexBuffers));
	ZeroMemory(m_uiStrides, sizeof(m_uiStrides));
	ZeroMemory(m_uiVertexBuffersOffsets, sizeof(m_uiVertexBuffersOffsets));
	ZeroMemory(m_pPixelShaderRVs, sizeof(m_pPixelShaderRVs));
	ZeroMemory(m_pVertexShaderRVs, sizeof(m_pVertexShaderRVs));
	ZeroMemory(m_pPixelConstantBuffers, sizeof(m_pPixelConstantBuffers));
	ZeroMemory(m_pVertexConstantBuffers, sizeof(m_pVertexConstantBuffers));
	ZeroMemory(m_pPixelSamplers, sizeof(m_pPixelSamplers));
	ZeroMemory(m_pVertexSamplers, sizeof(m_pVertexSamplers));
	ZeroMemory(m_pScissorRects, sizeof(m_pScissorRects));
	ZeroMemory(m_pGeometryShaderRVs, sizeof(m_pGeometryShaderRVs));
	ZeroMemory(m_pGeometryConstantBuffers, sizeof(m_pGeometryConstantBuffers));
	ZeroMemory(m_pGeometrySamplers, sizeof(m_pGeometrySamplers));
	ZeroMemory(m_pSOTargets, sizeof(m_pSOTargets));

	m_pDeviceContext = pDeviceCtx;
	m_pDeviceContext->AddRef();
}

D3D11StateBlock::~D3D11StateBlock()
{
	ReleaseObjects();
	m_pDeviceContext->Release();
}

STDMETHODIMP D3D11StateBlock::Capture(void)
{
	HRESULT hr = S_OK;

	ReleaseObjects();
	//
	// VS
	m_pDeviceContext->VSGetShader(&m_pVertexShader, NULL, NULL);
	m_pDeviceContext->VSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, m_pVertexShaderRVs);
	m_pDeviceContext->VSGetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, m_pVertexConstantBuffers);
	m_pDeviceContext->VSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_pVertexSamplers);

	//
	// GS
	//m_pDeviceContext->GSGetShader(&m_pGeometryShader, NULL, NULL);
	//m_pDeviceContext->GSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, m_pGeometryShaderRVs);
	//m_pDeviceContext->GSGetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, m_pGeometryConstantBuffers);
	//m_pDeviceContext->GSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_pGeometrySamplers);

	//
	// PS
	m_pDeviceContext->PSGetShader(&m_pPixelShader, NULL, NULL);
	m_pDeviceContext->PSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, m_pPixelShaderRVs);
	m_pDeviceContext->PSGetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, m_pPixelConstantBuffers);
	m_pDeviceContext->PSGetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_pPixelSamplers);

	//
	// IA
	m_pDeviceContext->IAGetInputLayout(&m_pInputLayout);
	m_pDeviceContext->IAGetIndexBuffer(&m_pIndexBuffer, &m_DXGIFormat, &m_uiOffset);
	m_pDeviceContext->IAGetPrimitiveTopology(&m_PrimitiveTopology);
	m_pDeviceContext->IAGetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, m_pVertexBuffers, m_uiStrides, m_uiVertexBuffersOffsets);

	//
	// RS
	m_pDeviceContext->RSGetState(&m_pRasterizerState);
	m_uiNumViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	m_pDeviceContext->RSGetViewports(&m_uiNumViewports, m_pViewports);
	//m_uiNumScissorRects = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	//m_pDeviceContext->RSGetScissorRects(&m_uiNumScissorRects, m_pScissorRects);

	//
	// OM
	m_pDeviceContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, m_pRenderTargetViews, &m_pDepthStencilView);
	m_pDeviceContext->OMGetBlendState(&m_pBlendState, m_fBlendFactor, &m_uiSampleMask);

	//
	// SO targets
	//m_pDeviceContext->SOGetTargets(4, m_pSOTargets);

	//
	// Predication
	//m_pDeviceContext->GetPredication(&m_pPredicate, &m_bPredicate);

	return hr;
}

STDMETHODIMP D3D11StateBlock::Apply(void)
{
	HRESULT hr = S_OK;

	hr = Apply1();

	ReleaseObjects();

	return hr;
}

STDMETHODIMP D3D11StateBlock::Apply1(void)
{
	HRESULT hr = S_OK;

	//
	// VS
	m_pDeviceContext->VSSetShader(m_pVertexShader, NULL, NULL);
	m_pDeviceContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, m_pVertexShaderRVs);
	m_pDeviceContext->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, m_pVertexConstantBuffers);
	m_pDeviceContext->VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_pVertexSamplers);

	//
	// GS
	//m_pDeviceContext->GSSetShader(m_pGeometryShader, NULL, NULL);
	//m_pDeviceContext->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, m_pGeometryShaderRVs);
	//m_pDeviceContext->GSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, m_pGeometryConstantBuffers);
	//m_pDeviceContext->GSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_pGeometrySamplers);

	//
	// PS
	m_pDeviceContext->PSSetShader(m_pPixelShader, NULL, NULL);
	m_pDeviceContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, m_pPixelShaderRVs);
	m_pDeviceContext->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, m_pPixelConstantBuffers);
	m_pDeviceContext->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, m_pPixelSamplers);

	//
	// IA
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, m_DXGIFormat, m_uiOffset);
	m_pDeviceContext->IASetPrimitiveTopology(m_PrimitiveTopology);
	m_pDeviceContext->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, m_pVertexBuffers, m_uiStrides, m_uiVertexBuffersOffsets);

	//
	// RS
	m_pDeviceContext->RSSetState(m_pRasterizerState);
	m_pDeviceContext->RSSetViewports(m_uiNumViewports, m_pViewports);
	//m_pDeviceContext->RSSetScissorRects(m_uiNumScissorRects, m_pScissorRects);

	//
	// OM
	m_pDeviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, m_pRenderTargetViews, m_pDepthStencilView);
	m_pDeviceContext->OMSetBlendState(m_pBlendState, m_fBlendFactor, m_uiSampleMask);

	//
	// SO targets
	//m_pDeviceContext->SOSetTargets(4, m_pSOTargets, NULL);

	//
	// Predication
	//m_pDeviceContext->SetPredication(m_pPredicate, m_bPredicate);

	return hr;
}

STDMETHODIMP D3D11StateBlock::ReleaseObjects(void)
{
	HRESULT hr = S_OK;

	if (m_pRasterizerState)
	{
		m_pRasterizerState->Release();
		m_pRasterizerState = NULL;
	}

	for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		if (m_pRenderTargetViews[i])
		{
			m_pRenderTargetViews[i]->Release();
			m_pRenderTargetViews[i] = NULL;
		}
	}

	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = NULL;
	}

	if (m_pBlendState)
	{
		m_pBlendState->Release();
		m_pBlendState = NULL;
	}
		
	if (m_pInputLayout)
	{
		m_pInputLayout->Release();
		m_pInputLayout = NULL;
	}

	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = NULL;
	}

	for (int i = 0; i < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT; i++)
	{
		if (m_pVertexBuffers[i])
		{
			m_pVertexBuffers[i]->Release();
			m_pVertexBuffers[i] = NULL;
		}
	}

	for (int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
	{
		if (m_pPixelShaderRVs[i])
		{
			m_pPixelShaderRVs[i]->Release();
			m_pPixelShaderRVs[i] = NULL;
		}

		if (m_pVertexShaderRVs[i])
		{
			m_pVertexShaderRVs[i]->Release();
			m_pVertexShaderRVs[i] = NULL;
		}

		if (m_pGeometryShaderRVs[i])
		{
			m_pGeometryShaderRVs[i]->Release();
			m_pGeometryShaderRVs[i] = NULL;
		}
	}

	for (int i = 0; i < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT; i++)
	{
		if (m_pPixelConstantBuffers[i])
		{
			m_pPixelConstantBuffers[i]->Release();
			m_pPixelConstantBuffers[i] = NULL;
		}

		if (m_pVertexConstantBuffers[i])
		{
			m_pVertexConstantBuffers[i]->Release();
			m_pVertexConstantBuffers[i] = NULL;
		}

		if (m_pGeometryConstantBuffers[i])
		{
			m_pGeometryConstantBuffers[i]->Release();
			m_pGeometryConstantBuffers[i] = NULL;
		}
	}

	for (int i = 0; i < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT; i++)
	{
		if (m_pPixelSamplers[i])
		{
			m_pPixelSamplers[i]->Release();
			m_pPixelSamplers[i] = NULL;
		}

		if (m_pVertexSamplers[i])
		{
			m_pVertexSamplers[i]->Release();
			m_pVertexSamplers[i] = NULL;
		}

		if (m_pGeometrySamplers[i])
		{
			m_pGeometrySamplers[i]->Release();
			m_pGeometrySamplers[i] = NULL;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		if (m_pSOTargets[i])
		{
			m_pSOTargets[i]->Release();
			m_pSOTargets[i] = NULL;
		}
	}

	if (m_pVertexShader)
	{
		m_pVertexShader->Release();
		m_pVertexShader = NULL;
	}

	if (m_pPixelShader)
	{
		m_pPixelShader->Release();
		m_pPixelShader = NULL;
	}

	if (m_pGeometryShader)
	{
		m_pGeometryShader->Release();
		m_pGeometryShader = NULL;
	}

	if (m_pPredicate)
	{
		m_pPredicate->Release();
		m_pPredicate = NULL;
	}

	ZeroMemory(m_pViewports, sizeof(m_pViewports));
	ZeroMemory(m_fBlendFactor, sizeof(m_fBlendFactor));
	ZeroMemory(m_uiStrides, sizeof(m_uiStrides));
	ZeroMemory(m_uiVertexBuffersOffsets, sizeof(m_uiVertexBuffersOffsets));
	ZeroMemory(m_pScissorRects, sizeof(m_pScissorRects));
	ZeroMemory(m_pViewports, sizeof(m_pViewports));
	ZeroMemory(m_fBlendFactor, sizeof(m_fBlendFactor));

	m_uiNumScissorRects = 0;
	m_uiNumViewports = 0;
	m_uiSampleMask = 0;

	m_bPredicate = FALSE;

	return hr;
}

HRESULT WINAPI D3D11CreateDeviceStateBlock(
	_In_ ID3D11DeviceContext	*pDeviceCtx,
	_Out_ D3D11StateBlock		**ppStateBlock)
{
	HRESULT hr = S_OK;

	D3D11StateBlock *pStateBlock = NULL;

	try
	{
		pStateBlock = new D3D11StateBlock(pDeviceCtx);
	}
	catch (std::bad_alloc exec)
	{
		pStateBlock = NULL;

		hr = E_OUTOFMEMORY;
	}
	
	*ppStateBlock = pStateBlock;

	return hr;
}
