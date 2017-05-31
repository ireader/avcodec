#include "avframe.h"
#include "sys/atomic.h"
#include <stdlib.h>
#include <assert.h>

struct avframe_t* avframe_alloc(size_t bytes)
{
	struct avframe_t* frame;
	frame = calloc(1, sizeof(*frame) + bytes);
	if (frame)
	{
		frame->data[0] = (uint8_t*)(frame + 1);
		frame->ref = 1;
	}
	return frame;
}

int32_t avframe_addref(struct avframe_t* frame)
{
	assert((struct avframe_t*)frame->data[0] - 1 == frame);
	return atomic_increment32(&frame->ref);
}

int32_t avframe_release(struct avframe_t* frame)
{
	int32_t ref;
	assert((struct avframe_t*)frame->data[0] - 1 == frame);
	ref = atomic_decrement32(&frame->ref);
	assert(ref >= 0);
	if (0 == ref)
	{
		free(frame);
	}
	return ref;
}
