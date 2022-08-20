#ifndef _avdecoder_h_
#define _avdecoder_h_

#include "avframe.h"
#include "avpacket.h"
#include "audio-decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

void* avdecoder_create_h264(const void* extra, int bytes);
void* avdecoder_create_h265(const void* extra, int bytes);

void* avdecoder_create(const struct avpacket_t* pkt);
void avdecoder_destroy(void* ff);

/// @return 0-ok, other-error
int avdecoder_input(void* ff, const struct avpacket_t* pkt);

/// @param[out] frame alloc memory internal, use avframe_release to free memory
/// @return >=0-got frame, <0-error
int avdecoder_getframe(void* ff, struct avframe_t** frame);

struct audio_decoder_t* aac_decoder();
struct audio_decoder_t* mp3_decoder();

#ifdef __cplusplus
}
#endif
#endif /* !_avdecoder_h_ */
