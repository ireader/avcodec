#ifndef _avpacket_h_
#define _avpacket_h_

#include <stdint.h>
#include <stddef.h>

enum avpacket_flag
{
	AVPACKET_FLAG_KEY = 0x01,
};

struct avpacket_t
{
	uint8_t* data;
	size_t bytes;

	int64_t pts;
	int64_t dts;

	int pic_type;	// picture type(PICTURE_TYPE_XXX)
	int flags;		// AVPACKET_FLAG_XXX

	int32_t ref;
};

#ifdef __cplusplus
extern "C" {
#endif

///@param[in] bytes alloc packet data size, don't include sizeof(struct avpacket_t)
///@return alloc new avpacket_t, use avpacket_release to free memory
struct avpacket_t* avpacket_alloc(size_t bytes);
int32_t avpacket_addref(struct avpacket_t* pkt);
int32_t avpacket_release(struct avpacket_t* pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_avpacket_h_ */
