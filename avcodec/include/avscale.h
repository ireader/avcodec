#ifndef _avscale_h_
#define _avscale_h_

#include "picture.h"

#ifdef __cplusplus
extern "C" {
#endif

/// scale picture
/// @param src input picture
/// @param dst output picture, dst->format/dst->width/dst->height/dst->linesize MUST set by user
/// @return 0-ok, other-error
int avscale(picture_t* dst, const picture_t* src);

#ifdef __cplusplus
}
#endif
#endif /* !_avscale_h_ */
