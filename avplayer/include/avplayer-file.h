#ifndef _file_player_h_
#define _file_player_h_

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
	#define AVPLAYER_FILE_API DLL_EXPORT_API
#else
	#define AVPLAYER_FILE_API DLL_IMPORT_API
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef int (*avplayer_file_read)(void* param, struct avpacket_t* pkt, int* type);

AVPLAYER_FILE_API void* avplayer_file_create(void* window, avplayer_file_read reader, void* param);
AVPLAYER_FILE_API void avplayer_file_destroy(void* player);
AVPLAYER_FILE_API void avplayer_file_play(void* player);
AVPLAYER_FILE_API void avplayer_file_pause(void* player);

#if defined(__cplusplus)
}
#endif
#endif /* !_file_player_h_ */
