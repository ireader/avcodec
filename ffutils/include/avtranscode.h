#ifndef _avtranscode_h_
#define _avtranscode_h_

#include "avframe.h"
#include "avpacket.h"

#ifdef __cplusplus
extern "C" {
#endif

struct avtranscode_t;
struct avtranscode_t* avtranscode_create_h264(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate);
//struct avtranscode_t* avtranscode_create_h265(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate);
struct avtranscode_t* avtranscode_create_aac(int sample_rate, int channel, int bitrate);
struct avtranscode_t* avtranscode_create_opus(int sample_rate, int channel, int bitrate);

void avtranscode_destroy(struct avtranscode_t* avt);

/// @return 0-ok, other-error
int avtranscode_input(struct avtranscode_t* avt, const struct avpacket_t* pkt);

int avtranscode_getpacket(struct avtranscode_t* avt, struct avpacket_t** pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_avtranscode_h_ */
