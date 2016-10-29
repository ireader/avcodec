#include "text-render.h"
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

struct text_render_t
{
	HDC hDC;
	HFONT hFont;
	void *bitmap;

	int width;
	int height;
};

void* text_render_create(const struct text_parameter_t* param)
{
	struct text_render_t* render;
	render = malloc(sizeof(struct text_render_t));
	if (NULL == render)
		return NULL;

	memset(render, 0, sizeof(struct text_render_t));
	render->hDC = CreateCompatibleDC(GetDC(NULL)); // screen DC
	if (NULL == render->hDC)
	{
		text_render_destroy(render);
		return NULL;
	}

	if (0 != text_render_config(render, param))
	{
		text_render_destroy(render);
		return NULL;
	}

	return render;
}

void text_render_destroy(void* p)
{
	struct text_render_t* render = (struct text_render_t*)p;

	if (render->hFont)
	{
		DeleteObject(render->hFont);
		render->hFont = NULL;
	}

	if (render->hDC)
	{
		DeleteDC(render->hDC);
		render->hDC = NULL;
	}

	if (render->bitmap)
	{
		free(render->bitmap);
		render->bitmap = NULL;
	}

	free(render);
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/dd183402(v=vs.85).aspx
// Capturing an Image
static int text_render_bitmap(struct text_render_t* render, HBITMAP hBitmap)
{
	void* p;
	BITMAP bmp;
	BITMAPINFOHEADER bi;

	// Get the BITMAP from the HBITMAP
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	assert(bmp.bmWidth == render->width && bmp.bmHeight == render->height);
	assert(bmp.bmBitsPixel == 1);

	memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = bmp.bmWidth;
	bi.biHeight = bmp.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;

	p = realloc(render->bitmap, bmp.bmHeight * bmp.bmWidth * 4);
	if (!p)
		return ENOMEM;

	render->bitmap = p;
	GetDIBits(render->hDC, hBitmap, 0, bmp.bmHeight, render->bitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	return 0;
}

static int text_render_draw_text(struct text_render_t* render, const wchar_t* txt)
{
//	SetBkColor(render->hDC, RGB(0x0, 0x0, 0xFF));
//	SetTextColor(render->hDC, RGB(0xFF, 0xFF, 0));
	TextOutW(render->hDC, 0, 0, txt, wcslen(txt));
	//RECT rc;
	//rc.left = x;
	//rc.top = y;
	//rc.right = render->width;
	//rc.bottom = render->height;

	//DrawTextA(render->hDC, txt, -1, &rc, DT_VCENTER | DT_LEFT);
	return 0;
}

static int text_render_getsize(struct text_render_t* render, const wchar_t* txt)
{
	RECT rc;
	memset(&rc, 0, sizeof(rc));

	// 1. GetTextExtent
	// 2. GetTextExtentPoint32
	// 3. DrawText with DT_CALCRECT
	if (0 == DrawTextW(render->hDC, txt, -1, &rc, DT_CALCRECT | DT_LEFT | DT_VCENTER))
		return -1;

	render->width = rc.right - rc.left;
	render->height = rc.bottom - rc.top;
	return 0;
}

const void* text_render_draw(void* p, const wchar_t* txt, int *w, int *h, int* pitch)
{
	HBITMAP hBitmap;
	struct text_render_t* render = (struct text_render_t*)p;

	if (render->hFont)
		SelectObject(render->hDC, render->hFont);

	if (0 != text_render_getsize(render, txt))
		return NULL;

	hBitmap = CreateCompatibleBitmap(render->hDC, render->width, render->height);
	SelectObject(render->hDC, hBitmap);

	text_render_draw_text(render, txt);
	text_render_bitmap(render, hBitmap);

	DeleteObject(hBitmap);

	*w = render->width;
	*h = render->height;
	*pitch = render->width;
	return render->bitmap;
}

int text_render_config(void* p, const struct text_parameter_t* param)
{
	HFONT hFont;
	struct text_render_t* render = (struct text_render_t*)p;
	
	hFont = CreateFontA(param->size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, param->font);
	if (NULL == hFont)
		return -1;

	if (render->hFont)
		DeleteObject(render->hFont);
	render->hFont = hFont;
	return 0;
}
