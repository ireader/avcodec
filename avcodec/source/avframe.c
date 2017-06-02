#include "avframe.h"
#include "sys/atomic.h"
#include <stdlib.h>
#include <assert.h>

struct avframe_t* avframe_alloc(int size)
{
	struct avframe_t* frame;
	frame = calloc(1, sizeof(*frame) + size);
	if (frame)
	{
		frame->data[0] = (uint8_t*)(frame + 1);
		frame->ref = 1;
	}
	return frame;
}

int32_t avframe_addref(struct avframe_t* frame)
{
	struct avframe_t* p;
	//assert((struct avframe_t*)frame->data[0] - 1 == frame);
	p = (struct avframe_t*)frame->data - 1;
	return atomic_increment32(&p->ref);
}

int32_t avframe_release(struct avframe_t* frame)
{
	int32_t ref;
	struct avframe_t* p;
	//assert((struct avframe_t*)frame->data[0] - 1 == frame);
	p = (struct avframe_t*)frame->data - 1;
	ref = atomic_decrement32(&p->ref);
	assert(ref >= 0);
	if (0 == ref)
	{
		free(p);
	}
	return ref;
}
