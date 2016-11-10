#ifndef _yuv_overlay_h_
#define _yuv_overlay_h_

#include "picture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _overlay_t
{
	int x;
	int y;
	int w;
	int h;

	uint8_t alpha; // opaque: [0, 255] default to 0(src overwrite dst)
	int mask;
	int op;

	int src_alpha; // 1-src has alpha channel
} overlay_t;

int yuv_overlay(picture_t* dst, const picture_t* src, const overlay_t* overlay);

#ifdef __cplusplus
}
#endif
#endif /* !_yuv_overlay_h_ */
