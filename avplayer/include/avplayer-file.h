#ifndef _file_player_h_
#define _file_player_h_

#include <stdint.h>
#include "avpacket.h"

#if defined(__cplusplus)
extern "C" {
#endif

/// read one frame(audio or video)
/// @param[out] type 0-audio, 1-video
/// @return >0-ok, 0-eof, <0-error
typedef int (*avplayer_file_read)(void* param, struct avpacket_t* pkt, int* type);

void* avplayer_file_create(void* window, avplayer_file_read reader, void* param);
void avplayer_file_destroy(void* player);
void avplayer_file_play(void* player);
void avplayer_file_pause(void* player);

/// seek
void avplayer_file_reset(void* player);

#if defined(__cplusplus)
}
#endif
#endif /* !_file_player_h_ */
