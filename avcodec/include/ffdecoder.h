#ifndef _ffdecoder_h_
#define _ffdecoder_h_

#include "picture.h"
#include "avpacket.h"

#ifdef __cplusplus
extern "C" {
#endif

void ffdecoder_init(void);
void ffdecoder_clean(void);

void* ffdecoder_create();
void ffdecoder_destroy(void* ff);

/// @return 0-ok, other-error
int ffdecoder_input(void* ff, const avpacket_t* pkt);

/// @return >=0-got frame, <0-error
int ffdecoder_getpicture(void* ff, picture_t* pic);

#ifdef __cplusplus
}
#endif
#endif /* !_ffdecoder_h_ */
