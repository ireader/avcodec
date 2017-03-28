#ifndef _avplayer_h_
#define _avplayer_h_

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
	#define DLL_EXPORT_API __declspec(dllexport)
	#define DLL_IMPORT_API __declspec(dllimport)
#else
	#if __GNUC__ >= 4
		#define DLL_EXPORT_API __attribute__((visibility ("default")))
		#define DLL_IMPORT_API
	#else
		#define DLL_EXPORT_API
		#define DLL_IMPORT_API
	#endif
#endif

#if defined(AVPLAYER_EXPORTS)
#define AVPLAYER_API DLL_EXPORT_API
#else
#define AVPLAYER_API DLL_IMPORT_API
#endif

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

struct avplayer_notify_t
{
	void (*on_status_changed)(void* param, int status);
	void (*on_buffering)(void* param, int buffering); // 0-buffering, 1-ok

	///@return audio output buffer durationMS(include pcm data)
	uint64_t (*on_audio)(void* param, const void* pcm, int discard); // audio output
	void (*on_video)(void* param, const void* frame, int discard); // draw video
};

AVPLAYER_API void* avplayer_create(const struct avplayer_notify_t* notify, void* param);
AVPLAYER_API void avplayer_destroy(void* player);
AVPLAYER_API void avplayer_play(void* player);
AVPLAYER_API void avplayer_stop(void* player);
AVPLAYER_API void avplayer_pause(void* player);

AVPLAYER_API int avplayer_input_audio(void* player, const void* pcm, uint64_t pts, uint64_t durationMS, int serial);
AVPLAYER_API int avplayer_input_video(void* player, const void* frame, uint64_t pts, int serial);

#if defined(__cplusplus)
}
#endif
#endif /* !_avplayer_h_ */
