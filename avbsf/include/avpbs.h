#ifndef _avpbs_h_
#define _avpbs_h_

#include <stdint.h>
#include "avpacket.h"
#include "avcodecid.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*avpbs_onpacket)(void* param, struct avpacket_t* pkt);

// avpacket bitstream helper
struct avpbs_t
{
	void* (*create)(int stream, AVPACKET_CODEC_ID codec, const uint8_t* extra, int bytes, avpbs_onpacket onpacket, void* param);
	int (*destroy)(void** ppbs);

	int (*input)(void* pbs, int64_t pts, int64_t dts, const uint8_t* nalu, int bytes, int flags);
};

struct avpbs_t* avpbs_find(AVPACKET_CODEC_ID codec);

#ifdef __cplusplus
}
#endif
#endif /* !_avpbs_h_ */
