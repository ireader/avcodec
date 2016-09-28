#ifndef _picture_h_
#define _picture_h_

#include <stdint.h>

enum picture_format {
	PICTURE_NONE	= -1,

	PICTURE_YUV420	= 0,	///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
	PICTURE_YUV422	= 4,	///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
	PICTURE_YUV444	= 5,	///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)

	PICTURE_NV12	= 25,	///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
	PICTURE_NV21	= 26,	///< as above, but U and V bytes are swapped

	PICTURE_BGR32	= 30,	///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
	PICTURE_BGR24	= 3,	///< packed RGB 8:8:8, 24bpp, BGRBGR...
	PICTURE_BGR16	= 47,	///< packed RGB 5:6:5, 16bpp, BGRBGR...
};

#define PICTURE_BGRA	PICTURE_BGR32
#define PICTURE_BGR565	PICTURE_BGR16

enum picture_type
{
	PICTURE_TYPE_NONE = 0,	///< Undefined
	PICTURE_TYPE_I,			///< Intra
	PICTURE_TYPE_P,			///< Predicted
	PICTURE_TYPE_B,			///< Bi-dir predicted
	PICTURE_TYPE_S,			///< S(GMC)-VOP MPEG-4
	PICTURE_TYPE_SI,		///< Switching Intra
	PICTURE_TYPE_SP,		///< Switching Predicted
	PICTURE_TYPE_BI,		///< BI type
};

typedef struct _picture_t
{
	int format;	//PICTURE_XXX
	int width;
	int height;
	int flags;

#define PICTURE_PLANAR_NUM 4
	uint8_t* data[PICTURE_PLANAR_NUM];
	int linesize[PICTURE_PLANAR_NUM];

	int64_t pts;
	int64_t dts;
} picture_t;

#endif /* !_picture_h_ */
