#ifndef _openh264enc_h_
#define _openh264enc_h_

#include "picture.h"
#include "avpacket.h"
#include "h264-parameter.h"

#ifdef __cplusplus
extern "C" {
#endif

	void* openh264enc_create(h264_parameter_t* param);
	void openh264enc_destroy(void* h264);

	int openh264enc_input(void* h264, const picture_t* pic);

	int openh264enc_getpacket(void* h264, avpacket_t* pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_openh264enc_h_  */
