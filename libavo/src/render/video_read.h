#ifndef _video_read_h_
#define _video_read_h_

#include "avframe.h"

#ifdef __cplusplus
extern "C"{
#endif

int video_read(const void* data, int *pitches, int format, struct avframe_t* pic);

#ifdef __cplusplus
}
#endif

#endif /* _video_read_h_ */
