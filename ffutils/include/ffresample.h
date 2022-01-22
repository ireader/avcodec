#ifndef _ffresample_h_
#define _ffresample_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"

void* ffresample_create(enum AVSampleFormat in_format, int in_sample_rate, int in_channels, enum AVSampleFormat out_format, int out_sample_rate, int out_channels);
void ffresample_destroy(void* ff);

/// @return 0-ok, other-error
int ffresample_convert(void* ff, const uint8_t** in_samples, int in_count, uint8_t** out_samples, int out_count);

#ifdef __cplusplus
}
#endif
#endif /* !_ffresample_h_ */
