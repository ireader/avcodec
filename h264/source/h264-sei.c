#include "h264-internal.h"
#include "h264-sei.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void h264_sei_buffering_period(bitstream_t* stream, struct h264_context_t* h264, struct h264_sei_buffering_period_t* bp)
{
	uint32_t i;
	struct h264_sps_t* sps;
	uint8_t seq_parameter_set_id;
	seq_parameter_set_id = (uint8_t)bitstream_read_ue(stream);
	sps = h264->sps + seq_parameter_set_id;
	if (sps->seq_parameter_set_id != seq_parameter_set_id)
		return;

	if (sps->vui.nal_hrd_parameters_present_flag)
	{
		for (i = 0; i <= sps->vui.nal_hrd.cpb_cnt_minus1 && i < sizeof_array(bp->initial_cpb_removal_delay); i++)
		{
			bp->initial_cpb_removal_delay[i] = bitstream_read_bits(stream, sps->vui.nal_hrd.initial_cpb_removal_delay_length_minus1 + 1);
			bp->initial_cpb_removal_delay_offset[i] = bitstream_read_bits(stream, sps->vui.nal_hrd.initial_cpb_removal_delay_length_minus1 + 1);
		}
	}

	if (sps->vui.vcl_hrd_parameters_present_flag)
	{
		for (i = 0; i <= sps->vui.vcl_hrd.cpb_cnt_minus1 && i < sizeof_array(bp->initial_cpb_removal_delay); i++)
		{
			bp->initial_cpb_removal_delay[i] = bitstream_read_bits(stream, sps->vui.vcl_hrd.initial_cpb_removal_delay_length_minus1 + 1);
			bp->initial_cpb_removal_delay_offset[i] = bitstream_read_bits(stream, sps->vui.vcl_hrd.initial_cpb_removal_delay_length_minus1 + 1);
		}
	}
}

static void h264_sei_pic_timing(bitstream_t* stream, struct h264_context_t* h264, struct h264_sei_pic_timing_t* pt)
{
	int i, pic_struct;
	struct h264_sps_t* sps;
	struct h264_sei_pic_timing_code_t* tc;
	sps = h264->_sps ? h264->_sps : &h264->sps[0];

	if (sps->vui.nal_hrd_parameters_present_flag)
	{
		pt->cpb_removal_delay = bitstream_read_bits(stream, sps->vui.nal_hrd.cpb_removal_delay_length_minus1 + 1);
		pt->dpb_output_delay = bitstream_read_bits(stream, sps->vui.nal_hrd.dpb_output_delay_length_minus1 + 1);
	}
	else if (sps->vui.vcl_hrd_parameters_present_flag)
	{
		pt->cpb_removal_delay = bitstream_read_bits(stream, sps->vui.vcl_hrd.initial_cpb_removal_delay_length_minus1 + 1);
		pt->dpb_output_delay = bitstream_read_bits(stream, sps->vui.vcl_hrd.dpb_output_delay_length_minus1 + 1);
	}

	if (sps->vui.pic_struct_present_flag)
	{
		static const uint8_t sei_num_clock_ts_table[9] = {
			1, 1, 1, 2, 2, 3, 3, 2, 3
		};

		pt->timecode_count = 0;
		pic_struct = bitstream_read_bits(stream, 4);
		for (i = 0; i < sei_num_clock_ts_table[pic_struct % 9]; i++)
		{
			// clock_timestamp_flag
			if (0 == bitstream_read_bit(stream)) 
				continue;

			tc = &pt->timecode[pt->timecode_count++];
			tc->ct_type = bitstream_read_bits(stream, 2);
			tc->nuit_field_based_flag = bitstream_read_bit(stream);
			tc->counting_type = bitstream_read_bits(stream, 5);
			tc->full_timestamp_flag = bitstream_read_bits(stream, 1);
			tc->discontinuity_flag = bitstream_read_bits(stream, 1);
			tc->cnt_dropped_flag = bitstream_read_bits(stream, 1);
			tc->n_frames = bitstream_read_bits(stream, 8);

			if (tc->full_timestamp_flag)
			{
				tc->seconds_value = bitstream_read_bits(stream, 6); // 0-59
				tc->minutes_value = bitstream_read_bits(stream, 6); // 0-59
				tc->hours_value = bitstream_read_bits(stream, 5); // 0-23
			}
			else
			{
				if (bitstream_read_bit(stream)) // seconds_flag
				{
					tc->seconds_value = bitstream_read_bits(stream, 6); // 0-59
					if (bitstream_read_bit(stream)) // minutes_flag
					{
						tc->minutes_value = bitstream_read_bits(stream, 6); // 0-59
						if (bitstream_read_bit(stream)) // hours_flag
							tc->hours_value = bitstream_read_bits(stream, 5); // 0-23
					}
				}
			}

			if (sps->vui.nal_hrd_parameters_present_flag || sps->vui.vcl_hrd_parameters_present_flag)
			{
				tc->time_offset = bitstream_read_bits(stream, sps->vui.nal_hrd_parameters_present_flag ? sps->vui.nal_hrd.time_offset_length : sps->vui.vcl_hrd.time_offset_length);
			}
		}
	}
}

static void h264_sei_recovery_point(bitstream_t* stream, struct h264_sei_recovery_point_t* recovery)
{
	recovery->recovery_frame_cnt = bitstream_read_ue(stream);
	recovery->exact_match_flag = bitstream_read_bit(stream);
	recovery->broken_link_flag = bitstream_read_bit(stream);
	recovery->changing_slice_group_idc = bitstream_read_bits(stream, 2);
}

int h264_sei(bitstream_t* stream, struct h264_context_t* h264)
{
	uint8_t v;
	uint32_t n, t;
	size_t bits;
	struct h264_sei_t* sei;
	sei = &h264->sei;

	do
	{
		t = 0;
		do
		{
			v = (uint8_t)bitstream_read_bits(stream, 8);
			t += v;
		} while (v == 0xFF);

		n = 0;
		do
		{
			v = (uint8_t)bitstream_read_bits(stream, 8);
			n += v;
		} while (v == 0xFF);

		bitstream_get_offset(stream, &bits);
		switch (t)
		{
		case H264_SEI_BUFFERING_PERIOD:
			h264_sei_buffering_period(stream, h264, &sei->buffering_period);
			break;

		case H264_SEI_PIC_TIMING:
			h264_sei_pic_timing(stream, h264, &sei->pic_timing);
			break;

		case H264_SEI_RECOVERY_POINT:
			h264_sei_recovery_point(stream, &sei->recovery_point);
			break;

		default:
			break;
		}

		// restore
		bitstream_set_offset(stream, bits);
		while(n > 0)
		{
			bitstream_read_bits(stream, 8);
			n--;
		}
	} while (0 == bitstream_error(stream) && h264_more_rbsp_data(stream));

	h264_rbsp_trailing_bits(stream);
	return 0;
}
