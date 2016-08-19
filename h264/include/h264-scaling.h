#ifndef _h264_scaling_h_
#define _h264_scaling_h_

#include "bitstream.h"

void h264_scaling_list(bitstream_t* stream, int *scalingList, int sizeOfScalingList, int *useDefaultScalingMatrixFlag);

#endif /* !_h264_scaling_h_ */

