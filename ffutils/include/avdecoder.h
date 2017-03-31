#ifndef _avdecoder_h_
#define _avdecoder_h_

#include "avframe.h"
#include "avpacket.h"

#ifdef __cplusplus
extern "C" {
#endif

void* avdecoder_create_h264();
void* avdecoder_create_aac();
void avdecoder_destroy(void* ff);

/// @return 0-ok, other-error
int avdecoder_input(void* ff, const struct avpacket_t* pkt);

/// @param[in] frame don't need free
/// @return >=0-got frame, <0-error
void* avdecoder_getframe(void* ff);

void avdecoder_freeframe(void* ff, void* frame);

void avdecoder_frame_to(const void* frame, struct avframe_t* dst);

#ifdef __cplusplus
}
#endif
#endif /* !_avdecoder_h_ */
