#pragma once

#include "d3d9_render.h"
#include "video_write.h"
#include <assert.h>

#define D3DFMT_YV12 (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2')
#define D3DFMT_NV12 (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2')

inline D3DFORMAT D3D9Format(int format)
{
	switch (format)
	{
	case PICTURE_RGBA: return D3DFMT_A8R8G8B8;
	case PICTURE_NV12: return D3DFMT_NV12;
	case PICTURE_YUV420: return D3DFMT_YV12;
	default: return D3DFMT_UNKNOWN;
	}
}

// 1-support, 0-unsupport
inline int D3D9CheckFormat(IDirect3D9* d3d9, D3DFORMAT format)
{
	HRESULT hr1, hr2;
	D3DDISPLAYMODE dpymode;
	ZeroMemory(&dpymode, sizeof(dpymode));
	hr1 = d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dpymode);
	hr2 = d3d9->CheckDeviceFormatConversion(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, format, dpymode.Format);
	return (SUCCEEDED(hr1) && SUCCEEDED(hr2)) ? 1 : 0;
}

static int d3d9_surface_release(d3d9_render_t* vo)
{
	if (vo->backbuffer)
	{
		vo->backbuffer->Release();
		vo->backbuffer = NULL;
	}

	if (vo->surface)
	{
		vo->surface->Release();
		vo->surface = NULL;
	}

	return 0;
}

/// @return 0-ok, other-error
static int d3d9_surface_create(d3d9_render_t* vo)
{
	HRESULT hr = S_OK;

	hr = vo->device->CreateOffscreenPlainSurface(vo->video_width, vo->video_height, D3D9Format(vo->video_format), D3DPOOL_DEFAULT, &vo->surface, NULL);
	if (FAILED(hr))
	{
		DxLog("D3D9 CreateOffscreenPlainSurface(%d, %d, 0x%0X) err: %d\n", vo->window_width, vo->window_height, (unsigned int)vo->video_format, (int)hr);
		d3d9_surface_release(vo);
		return -1;
	}

	hr = vo->device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &vo->backbuffer);
	if (FAILED(hr))
	{
		DxLog("D3D9 GetBackBuffer err: %d\n", (int)hr);
		d3d9_surface_release(vo);
		return -1;
	}
	return 0;
}

static int d3d9_surface_lockwrite(d3d9_render_t* vo, const struct avframe_t* pic)
{
	HRESULT hr = S_OK;
	D3DLOCKED_RECT rect;

	hr = vo->surface->LockRect(&rect, NULL, 0);
	if (FAILED(hr))
	{
		DxLog("D3D9 LockRect err: %d\n", (int)hr);
		return -1;
	}

	assert(vo->video_format == pic->format);
	int r = video_write(pic, rect.pBits, rect.Pitch);
	if (0 != r)
		DxLog("D3D9 unsupported video format: %d.\n", vo->video_format);

	vo->surface->UnlockRect();
	return r;
}
