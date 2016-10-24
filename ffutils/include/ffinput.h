#ifndef _ffinput_h_
#define _ffinput_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

void* ffinput_create(const char* url);

void ffinput_destroy(void* ff);

/// @param[out] pkt audio/video packet, MUST be freed with av_packet_unref
/// @return 0-ok, <0-error or end of file
int ffinput_read(void* ff, AVPacket* pkt);

/// @param[out]
/// @return >=0-stream count, <0-error or open stream failed
int ffinput_getstream(void* p, AVStream*** streams);

#ifdef __cplusplus
}
#endif
#endif /* !_ffinput_h_ */
