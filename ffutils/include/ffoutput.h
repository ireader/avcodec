#ifndef _ffoutput_h_
#define _ffoutput_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

void* ffoutput_create(const char* url, const char* format);
void ffoutput_destroy(void* ff);

int ffoutput_add_video_stream(void* ff, enum AVCodecID codecid);
int ffoutput_add_audio_stream(void* ff, enum AVCodecID codecid, int channel, int frequency, int bits_per_sample);

/// @param[out] pkt audio/video packet, MUST be freed with av_packet_unref
/// @return 0-ok, <0-error or end of file
int ffoutput_write(void* ff, AVPacket* pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_ffoutput_h_ */
