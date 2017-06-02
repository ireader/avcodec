#include "avpacket.h"
#include "sys/atomic.h"
#include <stdlib.h>
#include <assert.h>

struct avpacket_t* avpacket_alloc(int size)
{
	struct avpacket_t* pkt;
	pkt = calloc(1, sizeof(*pkt) + size);
	if (pkt)
	{
		pkt->data = (uint8_t*)(pkt + 1);
		pkt->size = size;
		pkt->ref = 1;
	}
	return pkt;
}

int32_t avpacket_addref(struct avpacket_t* pkt)
{
	struct avpacket_t* p;
	//assert((struct avpacket_t*)pkt->data - 1 == pkt);
	p = (struct avpacket_t*)pkt->data - 1;
	return atomic_increment32(&p->ref);
}

int32_t avpacket_release(struct avpacket_t* pkt)
{
	int32_t ref;
	struct avpacket_t* p;
	//assert((struct avpacket_t*)pkt->data - 1 == pkt);
	p = (struct avpacket_t*)pkt->data - 1;
	ref = atomic_decrement32(&p->ref);
	assert(ref >= 0);
	if (0 == ref)
	{
		free(p);
	}
	return ref;
}
