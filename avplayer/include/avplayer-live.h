#ifndef _avplayer_live_h_
#define _avplayer_live_h_

#include <stdint.h>
#include "avpacket.h"

#if defined(__cplusplus)
extern "C" {
#endif

void* avplayer_live_create(void* window);
void avplayer_live_destroy(void* player);
int avplayer_live_process(void* player, uint64_t clock);
int avplayer_live_input(void* player, struct avpacket_t* pkt);

#if defined(__cplusplus)
}
#endif
#endif /* !_avplayer_live_h_ */
