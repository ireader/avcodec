#ifndef _avplayer_h_
#define _avplayer_h_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum
{
	avplayer_status_close = 0,
	avplayer_status_play,
	avplayer_status_pause,
	avplayer_status_stop,
};

enum
{
	avplayer_render_buffering = 0, // buffering(all audio/video play finished)
	avplayer_render_video, // play video
	avplayer_render_audio, // play audio
};

/// audio/video output
/// @param[in] type avplayer_render_xxx
/// @return audio output buffer durationMS(include pcm data), 0-if video
typedef uint64_t (*avplayer_onrender)(void* param, int type, const void* frame, int discard);

void* avplayer_create(avplayer_onrender avrender, void* param);
void avplayer_destroy(void* player);
int avplayer_process(void* player, uint64_t clock);

void avplayer_play(void* player);
void avplayer_stop(void* player);
void avplayer_pause(void* player);

int avplayer_input_audio(void* player, const void* pcm, int64_t pts, uint64_t durationMS, int serial);
int avplayer_input_video(void* player, const void* frame, int64_t pts, int serial);

int64_t avplayer_get_audio_duration(void* player);

#if defined(__cplusplus)
}
#endif
#endif /* !_avplayer_h_ */
