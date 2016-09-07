#ifndef _ffencoder_h_
#define _ffencoder_h_

#include "picture.h"
#include "avpacket.h"
#include "h264-parameter.h"

#ifdef __cplusplus
extern "C" {
#endif

	void ffencoder_init(void);
	void ffencoder_clean(void);

	void* ffencoder_create(h264_parameter_t* param);
	void ffencoder_destroy(void* ff);

	/// @return 0-ok, other-error
	int ffencoder_input(void* ff, const picture_t* pic);

	/// @return >=0-got packet, <0-error
	int ffencoder_getpacket(void* ff, avpacket_t* pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_ffencoder_h_ */
