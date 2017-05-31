#include "avpacket.h"
#include "sys/atomic.h"
#include <stdlib.h>
#include <assert.h>

struct avpacket_t* avpacket_alloc(size_t bytes)
{
	struct avpacket_t* pkt;
	pkt = calloc(1, sizeof(*pkt) + bytes);
	if (pkt)
	{
		pkt->data = (uint8_t*)(pkt + 1);
		pkt->ref = 1;
	}
	return pkt;
}

int32_t avpacket_addref(struct avpacket_t* pkt)
{
	assert((struct avpacket_t*)pkt->data - 1 == pkt);
	return atomic_increment32(&pkt->ref);
}

int32_t avpacket_release(struct avpacket_t* pkt)
{
	int32_t ref;
	assert((struct avpacket_t*)pkt->data - 1 == pkt);
	ref = atomic_decrement32(&pkt->ref);
	assert(ref >= 0);
	if (0 == ref)
	{
		free(pkt);
	}
	return ref;
}