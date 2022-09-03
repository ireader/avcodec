#ifndef _ffencoder_h_
#define _ffencoder_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"

/// @param[inout] codecpar in-encode parameter, out-with extradata
void* ffencoder_create(const AVCodecParameters* codecpar, AVDictionary** opts);
void ffencoder_destroy(void* ff);

/// @return 0-ok, other-error
int ffencoder_input(void* ff, const AVFrame* frame);

/// @param[out] pkt must be free with av_packet_unref
/// @return >=0-got frame, <0-error
int ffencoder_getpacket(void* ff, AVPacket* pkt);

/// @return should call avcodec_parameters_free
AVCodecParameters* ffencoder_getcodecpar(void* ff);

#ifdef __cplusplus
}
#endif
#endif /* !_ffencoder_h_ */
