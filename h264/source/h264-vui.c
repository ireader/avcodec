#include "h264-vui.h"
#include "h264-hrd.h"

#define H264_VUI_Extended_SAR 255

void h264_vui_parameters(bitstream_t* stream)
{
	int aspect_ratio_info_present_flag = bitstream_read_bit(stream);
	if (aspect_ratio_info_present_flag)
	{
		int aspect_ratio_idc = bitstream_read_bits(stream, 8);
		if (aspect_ratio_idc == H264_VUI_Extended_SAR)
		{
			int sar_width = bitstream_read_bits(stream, 16);
			int sar_height = bitstream_read_bits(stream, 16);
		}
	}

	int overscan_info_present_flag = bitstream_read_bit(stream);
	if (overscan_info_present_flag)
	{
		int overscan_appropriate_flag = bitstream_read_bit(stream);
	}

	int video_signal_type_present_flag = bitstream_read_bit(stream);
	if (video_signal_type_present_flag)
	{
		int video_format = bitstream_read_bits(stream, 3);
		int video_full_range_flag = bitstream_read_bit(stream);
		int colour_description_present_flag = bitstream_read_bit(stream);
		if (colour_description_present_flag)
		{
			int colour_primaries = bitstream_read_bits(stream, 8);
			int transfer_characteristics = bitstream_read_bits(stream, 8);
			int matrix_coefficients = bitstream_read_bits(stream, 8);
		}
	}

	int chroma_loc_info_present_flag = bitstream_read_bit(stream);
	if (chroma_loc_info_present_flag)
	{
		int chroma_sample_loc_type_top_field = bitstream_read_ue(stream);
		int chroma_sample_loc_type_bottom_field = bitstream_read_ue(stream);
	}

	int timing_info_present_flag = bitstream_read_bit(stream);
	if (timing_info_present_flag)
	{
		int num_units_in_tick = bitstream_read_bits(stream, 32);
		int time_scale = bitstream_read_bits(stream, 32);
		int fixed_frame_rate_flag = bitstream_read_bit(stream);
	}

	int nal_hrd_parameters_present_flag = bitstream_read_bit(stream);
	if (nal_hrd_parameters_present_flag)
	{
		h264_hrd_parameters(stream);
	}

	int vcl_hrd_parameters_present_flag = bitstream_read_bit(stream);
	if (vcl_hrd_parameters_present_flag)
	{
		h264_hrd_parameters(stream);
	}

	if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
	{
		int low_delay_hrd_flag = bitstream_read_bit(stream);
	}

	int pic_struct_present_flag = bitstream_read_bit(stream);
	int bitstream_restriction_flag = bitstream_read_bit(stream);
	if (bitstream_restriction_flag)
	{
		int motion_vectors_over_pic_boundaries_flag = bitstream_read_bit(stream);
		int max_bytes_per_pic_denom = bitstream_read_ue(stream);
		int max_bits_per_mb_denom = bitstream_read_ue(stream);
		int log2_max_mv_length_horizontal = bitstream_read_ue(stream);
		int log2_max_mv_length_vertical = bitstream_read_ue(stream);
		int max_num_reorder_frames = bitstream_read_ue(stream);
		int max_dec_frame_buffering = bitstream_read_ue(stream);
	}
}
