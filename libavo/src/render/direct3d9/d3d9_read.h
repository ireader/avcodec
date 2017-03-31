#pragma once

#include "picture.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <d3d9.h>

static int d3d9_surface_lockread(IDirect3DSurface9* surface, picture_t* pic)
{
	D3DLOCKED_RECT rect;
	HRESULT hr = surface->LockRect(&rect, NULL, D3DLOCK_READONLY);
	if (SUCCEEDED(hr))
	{
		unsigned char *d0 = pic->data[0];
		unsigned char *data = (unsigned char*)rect.pBits;
		assert(rect.Pitch >= pic->width * 4);
		for (int i = 0; i < pic->height; i++)
		{
			for (int j = 0; j < pic->width; j++)
				memcpy(d0 + j * 3, data + j * 4, 3);

			data += rect.Pitch;
			d0 += pic->linesize[0];
		}

		surface->UnlockRect();
	}

	return hr;
}

int d3d9_read(IDirect3DDevice9* device, IDirect3DSurface9* backbuffer, const RECT* rc, picture_t* pic)
{
	HRESULT hr;
	IDirect3DSurface9* surface;
	
	hr = device->CreateRenderTarget(pic->width, pic->height, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONE, 0, TRUE, &surface, NULL);
	if (FAILED(hr)) return hr;

	hr = device->StretchRect(backbuffer, rc, surface, NULL, D3DTEXF_LINEAR);
	if (SUCCEEDED(hr))
		hr = d3d9_surface_lockread(surface, pic);

	surface->Release();
	return hr;
}
