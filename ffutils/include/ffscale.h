#ifndef _ffscale_h_
#define _ffscale_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/frame.h"

/// scale picture
/// @param src input picture
/// @param dst output picture, dst->format/dst->width/dst->height/dst->linesize MUST set by user
/// @return 0-ok, other-error
int ffscale(AVFrame* dst, const AVFrame* src);

#ifdef __cplusplus
}
#endif
#endif /* !_ffscale_h_ */
