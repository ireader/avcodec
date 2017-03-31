#pragma once

#include "d3d11_render.h"

static PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN fpD3D11CreateDeviceAndSwapChain;

static int d3d11_device_create(d3d11_render_t* vo)
{
	D3D_DRIVER_TYPE driverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE };
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = vo->window_width;
	sd.BufferDesc.Height = vo->window_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.SampleDesc.Count = 1; // The default sampler mode, with no anti-aliasing, has a count of 1 and a quality level of 0
	sd.SampleDesc.Quality = 0;
	sd.OutputWindow = vo->window;
	sd.Windowed = TRUE; /* windowed mode, IDXGISwapChain::SetFullscreenState */
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//	sd.Flags = DXGI_SWAP_CHAIN_FLAG_YUV_VIDEO;

	HRESULT hr = -1;
	unsigned int createFlags = 0;
#if defined(_DEBUG)
	//createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	for (int i = 0; i < ARRAYSIZE(driverTypes) && FAILED(hr); i++)
	{
		hr = fpD3D11CreateDeviceAndSwapChain(
			NULL,
			driverTypes[i], 
			0, 
			createFlags, 
			featureLevels, 
			ARRAYSIZE(featureLevels), 
			D3D11_SDK_VERSION,
			&sd,
			&vo->swapChain,
			&vo->d3dDevice, 
			&vo->featureLevel, 
			&vo->d3dContext);
	}

	return hr;
}
