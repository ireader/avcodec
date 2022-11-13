#include "h264-vui.h"
#include "h264-hrd.h"
#include "h264-internal.h"
#include <string.h>

#define H264_VUI_Extended_SAR 255

int h264_vui(bitstream_t* stream, struct h264_vui_t* vui)
{
	vui->aspect_ratio_info_present_flag = bitstream_read_bit(stream);
	if (vui->aspect_ratio_info_present_flag)
	{
		int aspect_ratio_idc = bitstream_read_bits(stream, 8);
		if (aspect_ratio_idc == H264_VUI_Extended_SAR)
		{
			/*int sar_width =*/ bitstream_read_bits(stream, 16);
			/*int sar_height =*/ bitstream_read_bits(stream, 16);
		}
	}

	vui->overscan_info_present_flag = bitstream_read_bit(stream);
	if (vui->overscan_info_present_flag)
	{
		/*int overscan_appropriate_flag =*/ bitstream_read_bit(stream);
	}

	vui->video_signal_type_present_flag = bitstream_read_bit(stream);
	if (vui->video_signal_type_present_flag)
	{
		vui->video_format = (unsigned int)bitstream_read_bits(stream, 3);
		vui->video_full_range_flag = (unsigned int)bitstream_read_bit(stream);
		int colour_description_present_flag = (unsigned int)bitstream_read_bit(stream);
		if (colour_description_present_flag)
		{
			vui->colour_primaries = (unsigned int)bitstream_read_bits(stream, 8);
			vui->transfer_characteristics = (unsigned int)bitstream_read_bits(stream, 8);
			vui->matrix_coefficients = (unsigned int)bitstream_read_bits(stream, 8);
		}
	}

	vui->chroma_loc_info_present_flag = bitstream_read_bit(stream);
	if (vui->chroma_loc_info_present_flag)
	{
		/*int chroma_sample_loc_type_top_field =*/ bitstream_read_ue(stream);
		/*int chroma_sample_loc_type_bottom_field =*/ bitstream_read_ue(stream);
	}

	vui->timing_info_present_flag = bitstream_read_bit(stream);
	if (vui->timing_info_present_flag)
	{
		/*int num_units_in_tick =*/ bitstream_read_bits(stream, 32);
		/*int time_scale =*/ bitstream_read_bits(stream, 32);
		/*int fixed_frame_rate_flag =*/ bitstream_read_bit(stream);
	}

	vui->nal_hrd_parameters_present_flag = bitstream_read_bit(stream);
	if (vui->nal_hrd_parameters_present_flag)
	{
		memset(&vui->nal_hrd, 0, sizeof(vui->nal_hrd));
		h264_hrd(stream, &vui->nal_hrd);
	}

	vui->vcl_hrd_parameters_present_flag = bitstream_read_bit(stream);
	if (vui->vcl_hrd_parameters_present_flag)
	{
		memset(&vui->vcl_hrd, 0, sizeof(vui->vcl_hrd));
		h264_hrd(stream, &vui->vcl_hrd);
	}

	if (vui->nal_hrd_parameters_present_flag || vui->vcl_hrd_parameters_present_flag)
	{
		/*int low_delay_hrd_flag =*/ bitstream_read_bit(stream);
	}

	vui->pic_struct_present_flag = bitstream_read_bit(stream);
	vui->bitstream_restriction_flag = bitstream_read_bit(stream);
	if (vui->bitstream_restriction_flag)
	{
		/*int motion_vectors_over_pic_boundaries_flag =*/ bitstream_read_bit(stream);
		/*int max_bytes_per_pic_denom =*/ bitstream_read_ue(stream);
		/*int max_bits_per_mb_denom =*/ bitstream_read_ue(stream);
		/*int log2_max_mv_length_horizontal =*/ bitstream_read_ue(stream);
		/*int log2_max_mv_length_vertical =*/ bitstream_read_ue(stream);
		/*int max_num_reorder_frames =*/ bitstream_read_ue(stream);
		/*int max_dec_frame_buffering =*/ bitstream_read_ue(stream);
	}
	return 0;
}
