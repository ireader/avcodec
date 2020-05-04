#ifndef _avbsf_h_
#define _avbsf_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @param[in] flags 0x01-keyframe, other-undefined
/// @return 0-ok, other-error
typedef int (*avbsf_onpacket)(void* param, int64_t pts, int64_t dts, const uint8_t* data, int bytes, int flags);

struct avbsf_t
{
	void* (*create)(const uint8_t* extra, int bytes, avbsf_onpacket onpacket, void* param);
	int (*destroy)(void** pbsf);

	int (*input)(void* bsf, int64_t pts, int64_t dts, const uint8_t* nalu, int bytes);
};

struct avbsf_t* avbsf_aac(void);
struct avbsf_t* avbsf_h264(void);
struct avbsf_t* avbsf_h265(void);

#ifdef __cplusplus
}
#endif
#endif /* !_avbsf_h_ */
