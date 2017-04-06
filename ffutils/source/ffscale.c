#include "ffscale.h"
#include "libswscale/swscale.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/// scale picture
/// @param src input picture
/// @param dst output picture, dst->format/dst->width/dst->height/dst->linesize MUST set by user
/// @return 0-ok, other-error
int ffscale(AVFrame* dst, const AVFrame* src)
{
	struct SwsContext* sws;
	if (0 == sws_isSupportedInput(src->format) || 0 == sws_isSupportedOutput(dst->format))
	{
		printf("input(%d) %s, output(%d) %s\n", src->format, sws_isSupportedInput(src->format) ? "ok" : "don't support", dst->format, sws_isSupportedOutput(dst->format) ? "ok" : "don't support");
		return -1;
	}

	sws = sws_getContext(src->width, src->height, src->format,
		dst->width, dst->height, dst->format,
		SWS_POINT, NULL, NULL, NULL);

	sws_scale(sws, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);

	sws_freeContext(sws);
	return 0;
}
