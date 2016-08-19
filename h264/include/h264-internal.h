#ifndef _h264_internal_h_
#define _h264_internal_h_

#include "bitstream.h"

void h264_rbsp_trailing_bits(bitstream_t* stream);
int h264_more_rbsp_data(bitstream_t* stream);

#endif /* !_h264_internal_h_ */
