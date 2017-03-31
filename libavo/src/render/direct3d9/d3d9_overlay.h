#pragma once

#include "d3d9_render.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <d3d9.h>

#define FVF_TYPE (D3DFVF_XYZ|D3DFVF_TEX1)

// x/y/w/h video position/size
int d3d9_vertex_create(d3d9_render_t* vo, int idx, int x, int y, int w, int h)
{
	HRESULT hr = S_OK;
	d3d9_vertex_t *v = NULL;

	if (!vo->vertex)
	{
		// use system memory
		// don't need reset device
		hr = vo->device->CreateVertexBuffer(4 * sizeof(d3d9_vertex_t) * N_OSD_TEXT, D3DUSAGE_WRITEONLY, FVF_TYPE, D3DPOOL_DEFAULT, &vo->vertex, NULL);
		if (FAILED(hr)) return hr;
	}

	// lock write
	hr = vo->vertex->Lock(idx * sizeof(d3d9_vertex_t) * 4, sizeof(d3d9_vertex_t) * 4, (void**)&v, 0);
	if (SUCCEEDED(hr))
	{
		// update position
#define VideoX(x) (2.0f*(x)/vo->window_width - 1.0f)
#define VideoY(y) (-2.0f*(y)/vo->window_height + 1.0f)
		v[0].x = VideoX(x);		v[0].y = VideoY(y + h);	v[0].z = 0.0f; v[0].u = 0.0f; v[0].v = 1.0f;
		v[1].x = VideoX(x);		v[1].y = VideoY(y);		v[1].z = 0.0f; v[1].u = 0.0f; v[1].v = 0.0f;
		v[2].x = VideoX(x + w); v[2].y = VideoY(y + h);	v[2].z = 0.0f; v[2].u = 1.0f; v[2].v = 1.0f;
		v[3].x = VideoX(x + w); v[3].y = VideoY(y);		v[3].z = 0.0f; v[3].u = 1.0f; v[3].v = 0.0f;
#undef VideoX
#undef VideoY

		vo->vertex->Unlock();
	}
	return hr;
}

static int d3d9_texture_lockwrite(IDirect3DTexture9* texture, const unsigned char* argb, int width, int height, int x, int y, int w, int h)
{
	D3DLOCKED_RECT rect;
	HRESULT hr = texture->LockRect(0, &rect, NULL, 0);
	if (SUCCEEDED(hr))
	{
		// write ARGB
		assert(rect.Pitch >= w * 4);
		unsigned char *data = (unsigned char *)rect.pBits;
		for (int i = y; i < h && i < height; i++)
		{
			memcpy(data, argb + (x + i * width) * 4, w * 4);
			data += rect.Pitch;
		}

		texture->UnlockRect(0);
	}

	return hr;
}

// x/y/w/h picture position/size
int d3d9_texture_create(d3d9_render_t* vo, int idx, const unsigned char* argb, int width, int height, int x, int y, int w, int h)
{
	HRESULT hr;
	assert(vo && vo->device && idx >= 0 && x >= 0 && y >= 0 && w > 0 && h > 0);
	assert(NULL == vo->textures[idx]);
	hr = vo->device->CreateTexture(w, h, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &vo->textures[idx], NULL);
	if (FAILED(hr)) return hr;

	//D3DSURFACE_DESC desc;
	//hr = vo->textures[idx]->GetLevelDesc(0, &desc);

	return d3d9_texture_lockwrite(vo->textures[idx], argb, width, height, x, y, w, h);
}

int d3d9_overlay_present(d3d9_render_t* vo)
{
	HRESULT hr;
	hr = vo->device->BeginScene();
	for (int i = 0; i < N_OSD_TEXT; i++)
	{
		if (0 == vo->textures[i])
			continue;

		assert(vo->vertex);
		hr = vo->device->SetTexture(0, vo->textures[i]);
		hr = vo->device->SetFVF(FVF_TYPE);
		hr = vo->device->SetStreamSource(0, vo->vertex, 0, sizeof(d3d9_vertex_t));
		hr = vo->device->DrawPrimitive(D3DPT_TRIANGLESTRIP, i * 4, 2);
		hr = vo->device->SetTexture(0, NULL);
	}
	hr = vo->device->EndScene();
	return hr;
}
