#include "yuv-overlay.h"
#include "colorspace.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// /ffmpeg/libavfilter/vf_overlay.c:blend_image
static void yuv_blend(picture_t* yv12, const picture_t* pic, overlay_t* overlay)
{
	int samples[3];
	int planar, row, col, rowmax, colmax;
	int x, y, srcw, srch, dstw, dsth;
	const uint8_t* sp;
	uint8_t* dp;

	// YUV420
	samples[0] = 0;
	samples[1] = samples[2] = 1;

	for (planar = 0; planar < 3; planar++)
	{
		x = overlay->x >> samples[planar];
		y = overlay->y >> samples[planar];
		srcw = pic->width >> samples[planar];
		srch = pic->height >> samples[planar];
		dstw = yv12->width >> samples[planar];
		dsth = yv12->height >> samples[planar];

		row = MAX(-y, 0);
		sp = pic->data[planar] + row * pic->linesize[planar];
		dp = yv12->data[planar] + (y + row) * yv12->linesize[planar];

		for (rowmax = MIN(-y + dsth, srch); row < rowmax; row++)
		{
			col = MAX(-x, 0);
			for (colmax = MIN(-x + dstw, srcw); col < colmax; col++)
			{
				dp[col + x] = (uint8_t)((dp[col + x] * overlay->alpha + sp[col] * (255 - overlay->alpha)) / 255);
			}

			sp += pic->linesize[planar];
			dp += yv12->linesize[planar];
		}
	}
}

static int yv12_overlay_bgr24(picture_t* yv12, const picture_t* pic, overlay_t* overlay)
{
	picture_t src;
	memcpy(&src, &pic, sizeof(picture_t));
	src.data[0] = (uint8_t*)malloc(pic->width * pic->height * 3 / 2);
	if (NULL == src.data[0])
		return ENOMEM;
	src.data[1] = src.data[0] + pic->width * pic->height;
	src.data[2] = src.data[1] + pic->width * pic->height / 4;
	src.linesize[0] = pic->width;
	src.linesize[1] = src.linesize[2] = pic->width / 2;
	src.format = PICTURE_YUV420;

	rgb24_yv12(pic->data[0], pic->width, pic->height, pic->linesize[0], src.data[0], src.data[1], src.data[2]);

	yuv_blend(yv12, &src, overlay);

	free(src.data[0]);
	return 0;
}

static int yv12_overlay_bgr32(picture_t* yv12, const picture_t* pic, overlay_t* overlay)
{
	picture_t src;
	memcpy(&src, &pic, sizeof(picture_t));
	src.data[0] = (uint8_t*)malloc(pic->width * pic->height * 3 / 2);
	if (NULL == src.data[0])
		return ENOMEM;
	src.data[1] = src.data[0] + pic->width * pic->height;
	src.data[2] = src.data[1] + pic->width * pic->height / 4;
	src.linesize[0] = pic->width;
	src.linesize[1] = src.linesize[2] = pic->width / 2;
	src.format = PICTURE_YUV420;

	rgb32_yv12(pic->data[0], pic->width, pic->height, pic->linesize[0], src.data[0], src.data[1], src.data[2]);

	yuv_blend(yv12, &src, overlay);

	free(src.data[0]);
	return 0;
}

static int yuv_overlay2(picture_t* yv12, const picture_t* pic, overlay_t* overlay)
{
	switch (pic->format)
	{
	case PICTURE_YUV420:
		yuv_blend(yv12, pic, overlay);
		return 0;

	case PICTURE_BGR24:
		return yv12_overlay_bgr24(yv12, pic, overlay);

	case PICTURE_BGR32:
		return yv12_overlay_bgr32(yv12, pic, overlay);

	default:
		return -1;
	}
}

int yuv_overlay(picture_t* dst, const picture_t* src, overlay_t* overlay)
{
	overlay->alpha = overlay->alpha % 255;
	switch (dst->format)
	{
	case PICTURE_YUV420:
	case PICTURE_YUV422:
	case PICTURE_YUV444:
		return yuv_overlay2(dst, src, overlay);

	default:
		printf("%s: unknown picture format %d\n", __FUNCTION__, dst->format);
		return -1;
	}
}
