#include "d3d11_render.h"
#include "d3d11_shader.h"
#include "d3d11_device.h"
#include "d3d11_buffer.h"
#include "d3d11_texture.h"
#include "d3d11_compile.h"
#include "video_output.h"
#include "av_register.h"
#include <d3d11.h>
#include <assert.h>

#define D3DFMT_YV12 (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2')
#define D3DFMT_NV12 (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2')

static int d3d11_shader_create(d3d11_render_t* vo)
{
	HRESULT hr;
	D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (FAILED(hr = vo->d3dDevice->CreatePixelShader(s_shader_pixel, sizeof(s_shader_pixel), NULL, &vo->pixelShader))
		|| FAILED(hr = vo->d3dDevice->CreateVertexShader(s_shader_vertex, sizeof(s_shader_vertex), 0, &vo->vertexShader))
		|| FAILED(hr = vo->d3dDevice->CreateInputLayout(vertexLayout, ARRAYSIZE(vertexLayout), s_shader_vertex, sizeof(s_shader_vertex), &vo->inputLayout)))
		return hr;

	return S_OK;
}

static int d3d11_sampler_state_create(ID3D11Device* device, ID3D11SamplerState** sampler)
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; //D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS; //D3D11_COMPARISON_NEVER;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	return device->CreateSamplerState(&samplerDesc, sampler);
}

static int d3d11_rasterizer_state_create(ID3D11Device* device, ID3D11RasterizerState** rasterizer)
{
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = FALSE;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.ScissorEnable = FALSE;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	return device->CreateRasterizerState(&rasterDesc, rasterizer);
	//vo->d3dContext->RSSetState(mainRasterizer);
}

static int d3d11_yv12_texture_create(d3d11_render_t* vo, int width, int height)
{
	HRESULT hr = S_OK;
	if (FAILED(hr = d3d11_texture_create(vo->d3dDevice, DXGI_FORMAT_R8_UNORM, width, height, &vo->yuv[0], &vo->shaderView[0]))
		|| FAILED(hr = d3d11_texture_create(vo->d3dDevice, DXGI_FORMAT_R8_UNORM, width/2, height/2, &vo->yuv[1], &vo->shaderView[1]))
		|| FAILED(hr = d3d11_texture_create(vo->d3dDevice, DXGI_FORMAT_R8_UNORM, width / 2, height / 2, &vo->yuv[2], &vo->shaderView[2])))
		return hr;
	return S_OK;
}

static int d3d11_render_target_create(d3d11_render_t* vo)
{
	ID3D11Texture2D* backBufferTexture = NULL;
	HRESULT hr = vo->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
	if (SUCCEEDED(hr))
	{
		hr = vo->d3dDevice->CreateRenderTargetView(backBufferTexture, NULL, &vo->backBufferTarget);
		backBufferTexture->Release();
	}
	return hr;
}

static int d3d11_device_resource_create(d3d11_render_t* vo)
{
	HRESULT hr = S_OK;
	if (FAILED(hr = d3d11_device_create(vo))
		|| FAILED(hr = d3d11_shader_create(vo))
		|| FAILED(hr = d3d11_vertex_buffer_create(vo->d3dDevice, &vo->vertexBuffer))
		|| FAILED(hr = d3d11_constant_buffer_create(vo->d3dDevice, &vo->constantBuffer))
		|| FAILED(hr = d3d11_sampler_state_create(vo->d3dDevice, &vo->samplerState))
		//|| FAILED(hr = d3d11_rasterizer_state_create(vo->d3dDevice, &vo->rasterizer))
		|| FAILED(hr = d3d11_yv12_texture_create(vo, vo->width, vo->height))
		|| FAILED(hr = d3d11_render_target_create(vo))
		)
	{
		return hr;
	}

	unsigned int stride = sizeof(TVertexPos);
	unsigned int offset = 0;
	vo->d3dContext->IASetVertexBuffers(0, 1, &vo->vertexBuffer, &stride, &offset);
	vo->d3dContext->IASetInputLayout(vo->inputLayout);
	vo->d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	vo->d3dContext->VSSetShader(vo->vertexShader, 0, 0);
	vo->d3dContext->VSSetConstantBuffers(0, 1, &vo->constantBuffer);
	vo->d3dContext->PSSetShader(vo->pixelShader, 0, 0);
	vo->d3dContext->PSSetShaderResources(0, ARRAYSIZE(vo->shaderView), vo->shaderView);
	vo->d3dContext->PSSetSamplers(0, 1, &vo->samplerState);
	vo->d3dContext->OMSetRenderTargets(1, &vo->backBufferTarget, NULL);
	return S_OK;
}

#define SAFE_RELEASE(p) if(p) { p->Release(); p = NULL; }

static void d3d11_device_resource_release(d3d11_render_t* vo)
{
	for (int i = 0; i < ARRAYSIZE(vo->shaderView); i++)
	{
		SAFE_RELEASE(vo->shaderView[i]);
	}
	for (int i = 0; i < ARRAYSIZE(vo->yuv); i++)
	{
		SAFE_RELEASE(vo->yuv[i]);
	}

	SAFE_RELEASE(vo->constantBuffer);
	SAFE_RELEASE(vo->samplerState);
	SAFE_RELEASE(vo->pixelShader);
	SAFE_RELEASE(vo->vertexShader);
	SAFE_RELEASE(vo->inputLayout);
	SAFE_RELEASE(vo->vertexBuffer);
	SAFE_RELEASE(vo->backBufferTarget);
	SAFE_RELEASE(vo->swapChain);
	SAFE_RELEASE(vo->d3dContext);
	SAFE_RELEASE(vo->d3dDevice);
}

static int Close(void* object)
{
	d3d11_render_t* vo = (d3d11_render_t*)object;
	if(!vo)
		return -1;

	d3d11_device_resource_release(vo);
	free(vo);
	return 0;
}

static void* Open(void* window, int format, int width, int height)
{
	d3d11_render_t* vo = (d3d11_render_t*)malloc(sizeof(d3d11_render_t));
	if(!vo)
		return NULL;
	memset(vo, 0, sizeof(d3d11_render_t));
	vo->width = width;
	vo->height = height;
	vo->format = format;
	vo->window = (HWND)window;
	
	RECT rc;
	GetClientRect(vo->window, &rc);
	vo->window_width = rc.right-rc.left;
	vo->window_height = rc.bottom-rc.top;

	if(FAILED(d3d11_device_resource_create(vo)))
	{
		Close(vo);
		return NULL;
	}

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)vo->window_width;
	viewport.Height = (float)vo->window_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	vo->d3dContext->RSSetViewports(1, &viewport);

	//vo->d3dContext->UpdateSubresource();
	return vo;
}

static int d3d11_yv12_texture_write(d3d11_render_t* vo, const struct avframe_t* pic)
{
	int w[3] = { vo->width, vo->width / 2, vo->width / 2 };
	int h[3] = { vo->height, vo->height / 2, vo->height / 2 };
	uint8_t* planar[3] = { pic->data[0], pic->data[2], pic->data[1] };

	for (int i = 0; i < 3; i++)
	{
		HRESULT hr = d3d11_texture_write(vo->d3dContext, vo->yuv[i], planar[i], w[i], h[i], pic->linesize[i]);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

static int d3d11_handle_device_lost(d3d11_render_t* vo)
{
	d3d11_device_resource_release(vo);
	return d3d11_device_resource_create(vo);
}

static int d3d11_handle_window_size_changed(d3d11_render_t* vo)
{
	RECT rect;
	::GetClientRect(vo->window, &rect);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	if (width == vo->window_width && height == vo->window_height)
		return S_OK;

	SAFE_RELEASE(vo->backBufferTarget);
	vo->d3dContext->OMSetRenderTargets(0, NULL, NULL);

	HRESULT hr = vo->swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (SUCCEEDED(hr))
	{
		hr = d3d11_render_target_create(vo);
		if(SUCCEEDED(hr))
			vo->d3dContext->OMSetRenderTargets(1, &vo->backBufferTarget, NULL);
	}
	else if (hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		d3d11_handle_device_lost(vo);
	}
	return hr;
}

/*
static void UpdateZoom(d3d11_render_t* vo, int src_x, int src_y, int src_w, int src_h, int drw_x, int drw_y, int drw_w, int drw_h)
{
//	bool zoomVideo = vo->zoom.x == src_x   && vo->zoom.y == src_y  && vo->zoom.w == src_w  && vo->zoom.h == src_h;
//	bool zoomWindow = vo->zoom.winX==drw_x && vo->zoom.winY==drw_y && vo->zoom.winW==drw_w && vo->zoom.winH==drw_h;

	RECT rect;
	::GetClientRect(vo->window, &rect);
//	zoomWindow = zoomWindow && vo->zoom.winWW==rect.right-rect.left && vo->zoom.winHH==rect.bottom-rect.top;

//	if(!zoomWindow)
	{
		float scaleX = drw_w*1.0f/(rect.right-rect.left);
		float scaleY = drw_h*1.0f/(rect.bottom-rect.top);
		XMMATRIX scaling = XMMatrixScaling(scaleX, scaleY, 1.0f);
		XMMATRIX rotation = XMMatrixRotationZ(vo->rotation);
		XMMATRIX translation = XMMatrixTranslation(drw_x*1.0f/(rect.right-rect.left)-(1-scaleX), drw_y*-1.0f/(rect.bottom-rect.top)+(1-scaleY), 0.0f);

		XMMATRIX m = XMMatrixTranspose(scaling * rotation * translation);
		//XMMATRIX m = XMMatrixTranspose(scaling * translation);
		//vo->zoom.view = scaling * rotation;
		memcpy(&vo->zoom.view, &m, sizeof(vo->zoom.view));

		//DXGI_MODE_DESC desc;
		//ZeroMemory(&desc, sizeof(desc));
		//desc.Width = drw_w;
		//desc.Height = drw_h;
		//desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		//desc.RefreshRate.Denominator = 1;
		//desc.RefreshRate.Numerator = 60;
		//vo->swapChain->ResizeTarget(&desc);
		//
		////vo->d3dContext->OMSetRenderTargets(0, NULL, NULL);
		////vo->backBufferTarget->Release();
		////HRESULT hr = vo->swapChain->ResizeBuffers(1, drw_w, drw_h, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		////ID3D11Texture2D* backBufferTexture = NULL;
		////hr = vo->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
		////hr = vo->d3dDevice->CreateRenderTargetView(backBufferTexture, NULL, &vo->backBufferTarget);
		////backBufferTexture->Release();
		////vo->d3dContext->OMSetRenderTargets(1, &vo->backBufferTarget, NULL);

		//D3D11_VIEWPORT viewport;
		//viewport.TopLeftX = (float)0;
		//viewport.TopLeftY = (float)0;
		//viewport.Width = (float)drw_w;
		//viewport.Height = (float)drw_h;
		//viewport.MinDepth = 0.0f;
		//viewport.MaxDepth = 1.0f;
		//vo->d3dContext->RSSetViewports(1, &viewport);
	}

	//if(!zoomVideo)
	//{
	//	vo->zoom.tex.x = src_x / vo->width;
	//	vo->zoom.tex.y = src_y / vo->height;
	//	vo->zoom.tex.z = src_w / vo->width;
	//	vo->zoom.tex.w = src_h / vo->height;
	//}

	//if(!zoomVideo || !zoomWindow)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(mappedResource));
		if(SUCCEEDED(vo->d3dContext->Map(vo->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
		{
			TConstantParameter* param = (TConstantParameter*)mappedResource.pData;
			memcpy(param, &vo->zoom, sizeof(TConstantParameter));
			vo->d3dContext->Unmap(vo->constantBuffer, 0);
		}
	}
}
*/

static int Write(void* object, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int drw_x, int drw_y, int drw_w, int drw_h)
{
	d3d11_render_t* vo = (d3d11_render_t*)object;
	if (!vo || NULL == vo->d3dContext || NULL == vo->swapChain)
		return -1;

//	d3d11_handle_window_size_changed(vo);

//	vo->zoom.ww = pic->width;
//	vo->zoom.hh = pic->height;
//	UpdateZoom(vo, src_x, src_y, src_w, src_h, drw_x, drw_y, drw_w, drw_h);
	if(FAILED(d3d11_yv12_texture_write(vo, pic)))
		return -1;

	//float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	//vo->d3dContext->ClearRenderTargetView(vo->backBufferTarget, clearColor);

	vo->d3dContext->Draw(6, 0);	

	HRESULT hr = vo->swapChain->Present(0, 0);
	if (FAILED(hr) && hr != DXGI_ERROR_WAS_STILL_DRAWING)
	{
		if (DXGI_ERROR_DEVICE_REMOVED == hr || DXGI_ERROR_DEVICE_RESET == hr)
		{
			d3d11_handle_device_lost(vo);
		}
	}
	return 0;
}

static int Rotation(void* object, float angle)
{
	d3d11_render_t* vo = (d3d11_render_t*)object;
	vo->rotation = angle;
	return 0;
}

extern "C" int d3d11_render_register()
{
	HMODULE hD3d11 = LoadLibraryEx("d3d11.dll", NULL, 0);
	fpD3D11CreateDeviceAndSwapChain = (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(hD3d11, "D3D11CreateDeviceAndSwapChain");
	if (NULL == fpD3D11CreateDeviceAndSwapChain)
		return -1;

	static video_output_t vo;
	vo.open = Open;
	vo.close = Close;
	vo.write = Write;
	vo.read = NULL;
	vo.control = NULL;
	vo.rotation = Rotation;
	return av_set_class(AV_VIDEO_RENDER, "d3d11", &vo);
}
