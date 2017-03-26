#ifndef _avdecoder_h_
#define _avdecoder_h_

#include "avframe.h"
#include "avpacket.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	AV_CODEC_H264	= 28,
	AV_CODEC_AAC	= 86018,
};

void* avdecoder_create(int codec);
void avdecoder_destroy(void* ff);

/// @return 0-ok, other-error
int avdecoder_input(void* ff, const struct avpacket_t* pkt);

/// @param[in] frame don't need free
/// @return >=0-got frame, <0-error
int avdecoder_getframe(void* ff, struct avframe_t* frame);

#ifdef __cplusplus
}
#endif
#endif /* !_avdecoder_h_ */
