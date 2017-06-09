#include "video_output.h"
#include "av_register.h"
#include "video_write.h"

#if defined(_DEBUG)
#define D3D_DEBUG_INFO
#endif
#include "d3d9_render.h"
#include "d3d9_device.h"
#include "d3d9_surface.h"
#include <assert.h>

// https://blogs.msdn.microsoft.com/chuckw/2012/04/24/wheres-dxerr-lib/
// Where¡¯s DXERR.LIB?
//#include <dxerr.h>
//#pragma comment(lib, "dxerr.lib")

static IDirect3D9* g_d3d9;

static int D3D9Open(IDirect3D9* d3d9, d3d9_render_t* vo)
{
	int r = d3d9_device_create(d3d9, vo);
	if(0 != r)
		return r;

	r = d3d9_surface_create(vo);
	if(0 != r)
		return r;

	r = d3d9_device_pipeline(vo);

	// set viewport
	//D3DVIEWPORT9 viewport;
	//ZeroMemory(&viewport, sizeof(viewport));
	//viewport.X = 0;
	//viewport.Y = 0;
	//viewport.Width = vo->window_width;
	//viewport.Height = vo->window_height;
	//viewport.MinZ = 0.0f;
	//viewport.MaxZ = 1.0f;
	//vo->device->SetViewport(&viewport);

	return 0;
}

static int D3D9Close(d3d9_render_t* vo)
{
	d3d9_surface_release(vo);

	for (int i = 0; i < N_OSD_TEXT; i++)
	{
		if (vo->textures[i])
		{
			vo->textures[i]->Release();
			vo->textures[i] = NULL;
		}
	}

	if(vo->vertex)
	{
		vo->vertex->Release();
		vo->vertex = NULL;
	}

	if(vo->device)
	{
		vo->device->Release();
		vo->device = NULL;
	}
	return 0;
}

static int Close(void* object)
{
	d3d9_render_t* vo = (d3d9_render_t*)object;
	if(!vo)
		return -1;

	D3D9Close(vo);
	free(vo);
	return 0;
}

static void* Open(void* window, int format, int width, int height)
{
	D3DFORMAT d3dformat = D3D9Format(format);
	if(D3DFMT_UNKNOWN == d3dformat)
	{
		DxLog("D3D9 unsupported video format.\n");
		return NULL;
	}

	// check display support format
	if(!D3D9CheckFormat(g_d3d9, d3dformat))
	{
		DxLog("D3D9 CheckDeviceFormatConversion(0x%0X) failed\n", (unsigned int)format);
		return NULL;
	}

	d3d9_render_t* vo = (d3d9_render_t*)malloc(sizeof(d3d9_render_t));
	if(!vo)
	{
		return NULL;
	}

	memset(vo, 0, sizeof(d3d9_render_t));
	vo->video_width = width;
	vo->video_height = height;
	vo->video_format = format;
	vo->window = (HWND)window;
	
	RECT rc;
	GetClientRect(vo->window, &rc);
	vo->window_width = rc.right-rc.left;
	vo->window_height = rc.bottom-rc.top;

	// create surface and back-buffer
	if(D3D9Open(g_d3d9, vo) < 0)
	{
		Close(vo);
		return NULL;
	}

	return vo;
}

static int Prepare(d3d9_render_t* vo, const struct avframe_t* pic, const RECT& rc)
{
	if(rc.right-rc.left!=vo->window_width || rc.bottom-rc.top!=vo->window_height)
	{
		// window size changed
		D3D9Close(vo); // Reset have no effect, must close it
		vo->window_width = rc.right-rc.left;
		vo->window_height = rc.bottom-rc.top;
	}
	else if(pic->width != vo->video_width || pic->height != vo->video_height)
	{
		// video size changed
		d3d9_surface_release(vo);
		vo->video_width = pic->width;
		vo->video_height = pic->height;
	}

	if(!vo->device)
	{
		return D3D9Open(g_d3d9, vo);
	}
	else if(!vo->surface)
	{
		if(0 != d3d9_surface_create(vo))
			return -1;
	}

	return d3d9_device_preparse(vo);
}

static int Write(void* object, const struct avframe_t* pic,
				 int src_x, int src_y, int src_w, int src_h,
				 int drw_x, int drw_y, int drw_w, int drw_h)
{
	d3d9_render_t* vo = (d3d9_render_t*)object;
	if(!vo)
		return -1;

	HRESULT hr = S_OK;
	int r = 0;

	RECT rc;
	RECT srcRect;
	RECT dstRect;
	::GetClientRect(vo->window, &rc);
	if (0 == rc.right - rc.left || 0 == rc.bottom - rc.top)
		return -1; // window size error

	r = Prepare(vo, pic, rc);
	if (0 != r)
		return r;

	src_x = src_x / 2 * 2; src_y = src_y / 2 * 2; // align 2 
	src_w = src_w / 2 * 2; src_h = src_h / 2 * 2; // align 2
	srcRect.left = src_x;
	srcRect.top = src_y;
	srcRect.right = (src_w ? src_x + src_w : vo->video_width);
	srcRect.bottom = (src_h ? src_y + src_h : vo->video_height);
	dstRect.left = drw_x;
	dstRect.top = drw_y;
	dstRect.right = (drw_w ? drw_x + drw_w : rc.right);
	dstRect.bottom = (drw_h ? drw_y + drw_h : rc.bottom);
	memcpy(&vo->dstRect, &dstRect, sizeof(vo->dstRect)); // for read video from buffer

	struct avframe_t yv12 = *pic;
	if (PICTURE_YUV420 == pic->format || PICTURE_YUV422 == pic->format)
	{
		// switch U/V planar
		yv12.data[1] = pic->data[2];
		yv12.data[2] = pic->data[1];
	}
	r = d3d9_surface_lockwrite(vo, &yv12);
	if (0 != r)
		return r;

	// video
	//hr = vo->device->Clear(0, NULL, D3DCLEAR_TARGET, 0, 0, 0);
	hr = vo->device->StretchRect(vo->surface, &srcRect, vo->backbuffer, &dstRect, D3DTEXF_LINEAR);
	if (FAILED(hr))
	{
		DxLog("D3D9 StretchRect err: %d\n", (int)hr);
		return -1;
	}

	// Direct3D9: (ERROR) :BitBlt or StretchBlt failed in Present
	// D3D9 Helper: IDirect3DDevice9::Present failed: E_FAIL
	// http://software.intel.com/en-us/forums/topic/279262
	hr = vo->device->Present(&dstRect, &dstRect, 0, NULL);
	//hr = vo->device->Present(NULL, NULL, 0, NULL);
	if (FAILED(hr))
	{
		DxLog("D3D9 Present err: %d\n", (int)hr);
		return -1;
	}

	return 0;
}

typedef IDirect3D9 * (WINAPI *DLL_Direct3DCreate9)(UINT SDKVersion);

static IDirect3D9* D3D9Load()
{
	HMODULE hD3d9 = LoadLibraryEx("d3d9.dll", NULL, 0);
	if (!hD3d9)
	{
		DxLog("D3D9 load d3d9.dll error.\n");
		return NULL;
	}

	DLL_Direct3DCreate9 fpDirect3DCreate9;
	fpDirect3DCreate9 = (DLL_Direct3DCreate9)GetProcAddress(hD3d9, "Direct3DCreate9");
	if (!fpDirect3DCreate9)
	{
		DxLog("D3D9 load procedure Direct3DCreate9 error.\n");
		return NULL;
	}
	
	return fpDirect3DCreate9(D3D_SDK_VERSION);
}

extern "C" int d3d9_render_register()
{
	// check device capability
	g_d3d9 = D3D9Load();
	if(NULL == g_d3d9) return false;

	static video_output_t vo;
	vo.open = Open;
	vo.close = Close;
	vo.write = Write;
	vo.read = NULL;
	vo.control = NULL;
	return av_set_class(AV_VIDEO_RENDER, "d3d9", &vo);
}
