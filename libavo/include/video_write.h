#ifndef _video_write_h_
#define _video_write_h_

#include "avframe.h"

#ifdef __cplusplus
extern "C"{
#endif

/// Copy video data from pic
/// @param[in] pic decoded video picture
/// @paran[out] data video render buffer
/// @param[in] pitches video render buffer linesize
/// @return 0-ok, other-error
int video_write(const struct avframe_t* pic, void* data, int pitches);

#ifdef __cplusplus
}
#endif

#endif /* !_video_write_h_ */
