#include "h264-internal.h"

void h264_hrd_parameters(bitstream_t* stream)
{
	int bit_rate_value_minus1[32];
	int cpb_size_value_minus1[32];
	int cbr_flag[32];

	int cpb_cnt_minus1 = bitstream_read_ue(stream);
	int bit_rate_scale = bitstream_read_bits(stream, 4);
	int cpb_size_scale = bitstream_read_bits(stream, 4);
	for (int SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; ++SchedSelIdx)
	{
		bit_rate_value_minus1[SchedSelIdx] = bitstream_read_ue(stream);
		cpb_size_value_minus1[SchedSelIdx] = bitstream_read_ue(stream);
		cbr_flag[SchedSelIdx] = bitstream_read_bit(stream);
	}

	int initial_cpb_removal_delay_length_minus1 = bitstream_read_bits(stream, 5);
	int cpb_removal_delay_length_minus1 = bitstream_read_bits(stream, 5);
	int dpb_output_delay_length_minus1 = bitstream_read_bits(stream, 5);
	int time_offset_length = bitstream_read_bits(stream, 5);
}
