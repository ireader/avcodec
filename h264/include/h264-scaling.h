#ifndef _h264_scaling_h_
#define _h264_scaling_h_

#include "bitstream.h"
#include <stdint.h>

void h264_scaling_list_4x4(bitstream_t* stream, int32_t ScalingList4x4[16], uint8_t *useDefaultScalingMatrixFlag);
void h264_scaling_list_8x8(bitstream_t* stream, int32_t ScalingList8x8[64], uint8_t *useDefaultScalingMatrixFlag);

#endif /* !_h264_scaling_h_ */
