#pragma once

#include "d3d9_render.h"
#include "d3d9_surface.h"

static int D3D9Close(d3d9_render_t* vo);
static UINT D3D9GetAdapter(IDirect3D9* d3d9, HWND window);
static void D3D9FillPresentParams(d3d9_render_t* vo, D3DPRESENT_PARAMETERS& d3dpp);

/// @return 0-ok, other-error
static int d3d9_device_create(IDirect3D9* d3d9, d3d9_render_t* vo)
{
	int flags = 0;
	HRESULT hr = S_OK;
	D3DPRESENT_PARAMETERS d3dpp;

	vo->adapter = D3D9GetAdapter(d3d9, vo->window);

	// check caps
	ZeroMemory(&vo->caps, sizeof(vo->caps));
	d3d9->GetDeviceCaps(vo->adapter, D3DDEVTYPE_HAL, &vo->caps);
	DxLog("D3D9 device %d AdapterOrdinal: %u, MasterAdapterOrdinal: %u, NumberOfAdaptersInGroup: %u, AdapterOrdinalInGroup: %u\n", vo->adapter, vo->caps.AdapterOrdinal, vo->caps.MasterAdapterOrdinal, vo->caps.NumberOfAdaptersInGroup, vo->caps.AdapterOrdinalInGroup);

	if (0 == (vo->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
	{
		DxLog("D3D9 warning: use software vertex processing.\n");
		flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else
	{
		flags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	//flags |= D3DCREATE_MULTITHREADED;
	//flags |= D3DCREATE_FPU_PRESERVE;

	D3D9FillPresentParams(vo, d3dpp);
	hr = d3d9->CreateDevice(vo->adapter, D3DDEVTYPE_HAL, vo->window, flags, &d3dpp, &vo->device);
	DxLog("D3D9 CreateDevice(window:%p(%d/%d), flags:%d): %d\n", vo->window, vo->window_width, vo->window_height, flags, int(hr));
	return SUCCEEDED(hr) ? 0 : -1;
}

/// @return 0-ok, other-error
static int d3d9_device_reset(d3d9_render_t* vo)
{
	// release resource before reset device
	d3d9_surface_release(vo);

	D3DPRESENT_PARAMETERS d3dpp;
	D3D9FillPresentParams(vo, d3dpp);
	HRESULT hr = vo->device->Reset(&d3dpp);
	DxLog("D3D9 Reset(window:%p(%d/%d)): %d\n", vo->window, vo->window_width, vo->window_height, int(hr));
	return SUCCEEDED(hr) ? 0 : -1;
}

/// @return 0-ok, other-error
static int d3d9_device_preparse(d3d9_render_t* vo)
{
	HRESULT hr = vo->device->TestCooperativeLevel();
	if (SUCCEEDED(hr))
	{
		return 0;
	}
	else if (D3DERR_DEVICENOTRESET == hr)
	{
		DxLog("D3D9 device not reset\n");
		//attempt to reset the device
		if (0 == d3d9_device_reset(vo))
			return d3d9_surface_release(vo);
	}
	else if (D3DERR_DEVICELOST == hr)
	{
		DxLog("D3D9 device lost\n");
		// the device is lost but cannot be restored at the current time
		D3D9Close(vo); // MD880V(WinXP) must release device
	}
	else if (D3DERR_DRIVERINTERNALERROR == hr)
	{
		DxLog("D3D9 driver interval error\n");
	}
	else
	{
		DxLog("D3D9 : %0x\n", hr);
	}

	DxLog("D3D9 TestCooperativeLevel err: %d\n", (int)hr);
	return -1;
}

static int d3d9_device_pipeline(d3d9_render_t* vo)
{
	// set sampler
	HRESULT hr = S_OK;
	hr = vo->device->SetRenderState(D3DRS_ZENABLE, FALSE);
	hr = vo->device->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = vo->device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	//hr = vo->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//hr = vo->device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	//hr = vo->device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	hr = vo->device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	hr = vo->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	hr = vo->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//if(vo->caps.AlphaCmpCaps & D3DCMP_NOTEQUAL)
	//{
	//	hr = vo->device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	//	hr = vo->device->SetRenderState(D3DRS_ALPHAREF, 0xFF);
	//	hr = vo->device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
	//}

	//hr = vo->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	//hr = vo->device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//hr = vo->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	//hr = vo->device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	//hr = vo->device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//hr = vo->device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	hr = vo->device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	hr = vo->device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	return hr;
}

static void D3D9FillPresentParams(d3d9_render_t* vo, D3DPRESENT_PARAMETERS& d3dpp)
{
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO | D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.hDeviceWindow = vo->window;
	//if(!d3dpp.Windowed)
	//{
	//	D3DDISPLAYMODE mode;
	//	g_d3d9.d3d9->GetAdapterDisplayMode(vo->adapter, &mode);
	//	d3dpp.BackBufferWidth = mode.Width;
	//	d3dpp.BackBufferHeight = mode.Height;
	//	d3dpp.BackBufferFormat = mode.Format;
	//	d3dpp.FullScreen_RefreshRateInHz = mode.RefreshRate;
	//}
	//else
	{
		//d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferWidth = vo->window_width;
		d3dpp.BackBufferHeight = vo->window_height;
	}
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.EnableAutoDepthStencil = FALSE;
#if 0
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
#else
	// check present interval
	if (0 == (vo->caps.PresentationIntervals & D3DPRESENT_INTERVAL_IMMEDIATE))
		DxLog("D3D9FillPresentParams caps: 0x%0x\n", vo->caps.PresentationIntervals);
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#endif
}

static UINT D3D9GetAdapter(IDirect3D9* d3d9, HWND window)
{
	UINT adapter = D3DADAPTER_DEFAULT;
	UINT n = d3d9->GetAdapterCount();
	for (UINT i = 0; i < n; i++)
	{
		HMONITOR monitor = d3d9->GetAdapterMonitor(i);
		if (monitor == MonitorFromWindow(window, MONITOR_DEFAULTTONULL))
		{
			DxLog("D3D9 window adapter: %d/%d.\n", i, n);
			adapter = i;
			break;
		}
	}
	return adapter;
}
