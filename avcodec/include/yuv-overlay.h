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

	int alpha; // [0, 255] default to 0(src overwrite dst)
	int mask;
	int op;
} overlay_t;

int yuv_overlay(picture_t* dst, const picture_t* src, overlay_t* overlay);

#ifdef __cplusplus
}
#endif
#endif /* !_yuv_overlay_h_ */
