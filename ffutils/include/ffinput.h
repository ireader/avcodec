#ifndef _ffinput_h_
#define _ffinput_h_

#include "avpacket.h"

#ifdef __cplusplus
extern "C" {
#endif

void* ffinput_create(const char* url);

void ffinput_destroy(void* ff);

/// @param[out] pkt audio/video packet, MUST be freed with avpacket_release
/// @return 0-EOF, >0-ok, <0-error
int ffinput_read(void* ff, struct avpacket_t** pkt);

#ifdef __cplusplus
}
#endif
#endif /* !_ffinput_h_ */
