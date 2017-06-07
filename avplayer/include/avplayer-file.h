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
void avplayer_file_play(void* player);
void avplayer_file_pause(void* player);

/// seek
void avplayer_file_reset(void* player);

#if defined(__cplusplus)
}
#endif
#endif /* !_avplayer_file_h_ */
