#include "text-render.h"
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

struct text_render_t
{
	HDC hDC;
	HBITMAP hBitmap;
	void *bitmap;

	int width;
	int height;
};

void* text_render_create(int width, int height)
{
	struct text_render_t* render;
	render = malloc(sizeof(struct text_render_t) + width * height * 4);
	if (NULL == render)
		return NULL;

	memset(render, 0, sizeof(struct text_render_t));
	render->width = width;
	render->height = height;
	render->bitmap = render + 1;
	render->hDC = CreateCompatibleDC(GetDC(NULL)); // screen DC
	render->hBitmap = CreateCompatibleBitmap(render->hDC, width, height);
	if (NULL == render->hBitmap)
	{
		text_render_destroy(render);
		return NULL;
	}
	return render;
}

void text_render_destroy(void* p)
{
	struct text_render_t* render = (struct text_render_t*)p;

	if (render->hBitmap)
	{
		DeleteObject(render->hBitmap);
		render->hBitmap = NULL;
	}

	if (render->hDC)
	{
		DeleteDC(render->hDC);
		render->hDC = NULL;
	}

	free(render);
}

const void* text_render_getimage(void* p)
{
	struct text_render_t* render = (struct text_render_t*)p;
	return render->bitmap;
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/dd183402(v=vs.85).aspx
// Capturing an Image
static int text_render_bitmap(struct text_render_t* render)
{
	BITMAP bmp;
	BITMAPINFOHEADER bi;

	// Get the BITMAP from the HBITMAP
	GetObject(render->hBitmap, sizeof(BITMAP), &bmp);
	assert(bmp.bmWidth == render->width && bmp.bmHeight == render->height);
	assert(bmp.bmBitsPixel == 1);

	memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = bmp.bmWidth;
	bi.biHeight = bmp.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;

	GetDIBits(render->hDC, render->hBitmap, 0, bmp.bmHeight, render->bitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
	return 0;
}

static int text_render_draw_text(struct text_render_t* render, const wchar_t* txt, int x, int y)
{
//	SetBkColor(render->hDC, RGB(0x0, 0x0, 0xFF));
//	SetTextColor(render->hDC, RGB(0xFF, 0xFF, 0));
	TextOutW(render->hDC, x, y, txt, wcslen(txt));
	//RECT rc;
	//rc.left = x;
	//rc.top = y;
	//rc.right = render->width;
	//rc.bottom = render->height;

	//DrawTextA(render->hDC, txt, -1, &rc, DT_VCENTER | DT_LEFT);
	return 0;
}

int text_render_draw(void* p, const wchar_t* txt, int x, int y)
{
	struct text_render_t* render = (struct text_render_t*)p;
	HGDIOBJ hBitmap;

	hBitmap = SelectObject(render->hDC, render->hBitmap);

	text_render_draw_text(render, txt, x, y);
	text_render_bitmap(render);

	SelectObject(render->hDC, hBitmap);
	return 0;
}
