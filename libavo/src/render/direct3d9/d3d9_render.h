#pragma once

#include "avframe.h"
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <d3d9.h>

#define N_OSD_TEXT 3

typedef struct _dx9_vertex_t
{
	float x, y, z;
	float u, v;
} d3d9_vertex_t;

typedef struct _d3d9_render_t
{
	int video_width;
	int video_height;
	int video_format;

	HWND window;
	int window_width;
	int window_height;

	UINT adapter;
	RECT dstRect;

	D3DCAPS9 caps;
	IDirect3DDevice9* device;
	IDirect3DSurface9* surface;
	IDirect3DSurface9* backbuffer;

	IDirect3DTexture9* textures[N_OSD_TEXT];
	IDirect3DVertexBuffer9* vertex;
} d3d9_render_t;

inline void DxLog(const char* format, ...)
{
	static TCHAR msg[2 * 1024] = { 0 };

	va_list vl;
	va_start(vl, format);
	vsnprintf(msg, sizeof(msg) - 1, format, vl);
	va_end(vl);

	OutputDebugString(msg);
}
