#ifndef _avplayer_live_h_
#define _avplayer_live_h_

#include <stdint.h>
#include "avpacket.h"

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
	#define AVPLAYER_LIVE_API DLL_EXPORT_API
#else
	#define AVPLAYER_LIVE_API DLL_IMPORT_API
#endif

#if defined(__cplusplus)
extern "C" {
#endif

AVPLAYER_LIVE_API void* avplayer_live_create(void* window);
AVPLAYER_LIVE_API void avplayer_live_destroy(void* player);
AVPLAYER_LIVE_API int avplayer_live_input(void* player, struct avpacket_t* pkt, int video);

#if defined(__cplusplus)
}
#endif
#endif /* !_avplayer_live_h_ */
