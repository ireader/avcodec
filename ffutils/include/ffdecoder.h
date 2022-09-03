#ifndef _ffdecoder_h_
#define _ffdecoder_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"

void* ffdecoder_create(const AVCodecParameters* codecpar, AVDictionary** opts);
void ffdecoder_destroy(void* ff);

/// @return 0-ok, other-error
int ffdecoder_input(void* ff, const AVPacket* pkt);

/// @param[in] frame must be memset to 0 or from av_frame_alloc()
/// @return >=0-got frame, <0-error
int ffdecoder_getframe(void* ff, AVFrame* frame);

/// @return should call avcodec_parameters_free
AVCodecParameters* ffdecoder_getcodecpar(void* ff);

#ifdef __cplusplus
}
#endif
#endif /* !_ffdecoder_h_ */
