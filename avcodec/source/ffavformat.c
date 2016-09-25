#include "ffavformat.h"
#include "libavutil/pixfmt.h"
#include "picture.h"

enum AVPixelFormat ffavforamt_from(int format)
{
	switch (format)
	{
	case PICTURE_NV12:		return AV_PIX_FMT_NV12;
	case PICTURE_NV21:		return AV_PIX_FMT_NV21;
	case PICTURE_YUV420:	return AV_PIX_FMT_YUV420P;
	case PICTURE_YUV422:	return AV_PIX_FMT_YUV422P;
	case PICTURE_YUV444:	return AV_PIX_FMT_YUV444P;

	case PICTURE_BGR32:		return AV_PIX_FMT_BGRA;
	case PICTURE_BGR24:		return AV_PIX_FMT_BGR24;
	case PICTURE_BGR16:		return AV_PIX_FMT_BGR565BE;

	default:				return AV_PIX_FMT_NONE;
	}
}

int ffavforamt_to(enum AVPixelFormat avpixelformat)
{
	switch (avpixelformat)
	{
	case AV_PIX_FMT_NV12:		return PICTURE_NV12;
	case AV_PIX_FMT_NV21:		return PICTURE_NV21;
	case AV_PIX_FMT_YUV420P:	return PICTURE_YUV420;
	case AV_PIX_FMT_YUV422P:	return PICTURE_YUV422;
	case AV_PIX_FMT_YUV444P:	return PICTURE_YUV444;

	case AV_PIX_FMT_BGRA:		return PICTURE_BGR32;
	case AV_PIX_FMT_BGR24:		return PICTURE_BGR24;
	case AV_PIX_FMT_BGR565BE:	return PICTURE_BGR16;

	default:					return AV_PIX_FMT_NONE;
	}
}
