#ifndef _avencoder_h_
#define _avencoder_h_

#include "avframe.h"
#include "avpacket.h"
#include "audio-encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

void* avencoder_create_h264(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate);
//void* avencoder_create_h265(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate);

void* avencoder_create(const struct avpacket_t* pkt);
void avencoder_destroy(void* ff);

/// @return 0-ok, other-error
int avencoder_input(void* ff, const struct avframe_t* frame);

/// @param[out] pkt alloc memory internal, use avpacket_release to free memory
/// @return >=0-got frame, <0-error
int avencoder_getframe(void* ff, struct avpacket_t** pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_avdecoder_h_ */
