#ifndef _h264_hrd_h_
#define _h264_hrd_h_

#include <stdint.h>

struct h264_hrd_t
{
	uint8_t cpb_cnt_minus1; // [0, 32]
	uint32_t bit_rate_value_minus1[32];
	uint32_t cpb_size_value_minus1[32];
	uint8_t cbr_flag[32];

	unsigned int bit_rate_scale : 4;
	unsigned int cpb_size_scale : 4;
	unsigned int initial_cpb_removal_delay_length_minus1 : 5;
	unsigned int cpb_removal_delay_length_minus1 : 5;
	unsigned int dpb_output_delay_length_minus1 : 5;
	unsigned int time_offset_length : 5;
};

#endif /* !_h264_hrd_h_ */
