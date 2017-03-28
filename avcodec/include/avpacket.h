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
};

#endif /* !_avpacket_h_ */
