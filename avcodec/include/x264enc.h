#ifndef _x264enc_h_
#define _x264enc_h_

#include "picture.h"
#include "avpacket.h"
#include "h264-parameter.h"

#ifdef __cplusplus
extern "C" {
#endif

	void* x264enc_create(h264_parameter_t* param);
	void x264enc_destroy(void* h264);

	/// pic->flags & AVPACKET_FLAG_KEY => force IDR
	int x264enc_input(void* h264, const picture_t* pic);

	int x264enc_getpacket(void* h264, avpacket_t* pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_x264enc_h_ */
