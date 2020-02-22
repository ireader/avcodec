#ifndef _avplayer_file_h_
#define _avplayer_file_h_

#include <stdint.h>
#include "avpacket.h"

#if defined(__cplusplus)
extern "C" {
#endif

/// read one frame(audio or video)
/// @return new A/V apcket by avpacket_alloc (internal free by avpacket_release)
typedef struct avpacket_t* (*avplayer_file_read)(void* param);

void* avplayer_file_create(void* window, avplayer_file_read reader, void* param);
void avplayer_file_destroy(void* player);
int avplayer_file_process(void* player, uint64_t clock);

void avplayer_file_play(void* player);
void avplayer_file_pause(void* player);

/// Seek: read seek to timestamp, then reset player
void avplayer_file_reset(void* player);

/// avplayer_file_getpos get current position
/// @return current position
uint64_t avplayer_file_getpos(void* player);

/// set/get speed
/// @param[in] speed get/set play speed, base 128(2x=256,4x=512,0.5x=64)
/// @return current speed
int avplayer_file_setspeed(void* player, int speed);
int avplayer_file_getspeed(void* player);

#if defined(__cplusplus)
}
#endif
#endif /* !_avplayer_file_h_ */
