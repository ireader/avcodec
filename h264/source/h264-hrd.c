#include "h264-hrd.h"
#include "h264-internal.h"

int h264_hrd(bitstream_t* stream, struct h264_hrd_t* hrd)
{
	uint8_t SchedSelIdx;
	
	hrd->cpb_cnt_minus1 = bitstream_read_ue(stream);
	hrd->bit_rate_scale = bitstream_read_bits(stream, 4);
	hrd->cpb_size_scale = bitstream_read_bits(stream, 4);
	
	for (SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1 && SchedSelIdx < sizeof_array(hrd->bit_rate_value_minus1); ++SchedSelIdx)
	{
		hrd->bit_rate_value_minus1[SchedSelIdx] = bitstream_read_ue(stream);
		hrd->cpb_size_value_minus1[SchedSelIdx] = bitstream_read_ue(stream);
		hrd->cbr_flag[SchedSelIdx] = bitstream_read_bit(stream);
	}

	hrd->initial_cpb_removal_delay_length_minus1 = bitstream_read_bits(stream, 5);
	hrd->cpb_removal_delay_length_minus1 = bitstream_read_bits(stream, 5);
	hrd->dpb_output_delay_length_minus1 = bitstream_read_bits(stream, 5);
	hrd->time_offset_length = bitstream_read_bits(stream, 5);
	return 0;
}
