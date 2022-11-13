#include "h265-vui.h"
#include "h265-hrd.h"
#include "h265-parser.h"
#include <string.h>
#include <assert.h>

#define H265_VUI_Extended_SAR 255

int h265_hrd(bitstream_t* stream, struct h265_hrd_t* hrd, int commonInfPresentFlag, int maxNumSubLayersMinus1);

int h265_vui(bitstream_t* stream, struct h265_vui_t* vui, int sps_max_sub_layers_minus1)
{
	vui->aspect_ratio_info_present_flag = bitstream_read_bit(stream);
	if (vui->aspect_ratio_info_present_flag)
	{
		int aspect_ratio_idc = bitstream_read_bits(stream, 8);
		if (aspect_ratio_idc == H265_VUI_Extended_SAR)
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
		vui->colour_description_present_flag = (unsigned int)bitstream_read_bit(stream);
		if (vui->colour_description_present_flag)
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

	vui->neutral_chroma_indication_flag = bitstream_read_bit(stream);
	vui->field_seq_flag = bitstream_read_bit(stream);
	vui->frame_field_info_present_flag = bitstream_read_bit(stream);

	vui->default_display_window_flag = bitstream_read_bit(stream);
	if (vui->default_display_window_flag)
	{
		/*int def_disp_win_left_offset =*/ bitstream_read_ue(stream);
		/*int def_disp_win_right_offset =*/ bitstream_read_ue(stream);
		/*int def_disp_win_top_offset =*/ bitstream_read_ue(stream);
		/*int def_disp_win_bottom_offset =*/ bitstream_read_ue(stream);
	}

	vui->vui_timing_info_present_flag = bitstream_read_bit(stream);
	if (vui->vui_timing_info_present_flag)
	{
		/*int vui_num_units_in_tick =*/ bitstream_read_bits(stream, 32);
		/*int vui_time_scale =*/ bitstream_read_bits(stream, 32);
		if (/*int vui_poc_proportional_to_timing_flag =*/ bitstream_read_bit(stream))
		{
			/*int vui_num_ticks_poc_diff_one_minus1 = */ bitstream_read_ue(stream);
		}

		if (/*int vui_hrd_parameters_present_flag =*/ bitstream_read_bit(stream))
		{
			h265_hrd(stream, &vui->hrd, 1, sps_max_sub_layers_minus1);
		}
	}

	vui->bitstream_restriction_flag = bitstream_read_bit(stream);
	if (vui->bitstream_restriction_flag)
	{
		/*int tiles_fixed_structure_flag =*/ bitstream_read_bit(stream);
		/*int motion_vectors_over_pic_boundaries_flag =*/ bitstream_read_bit(stream);
		/*int restricted_ref_pic_lists_flag =*/ bitstream_read_bit(stream);
		/*int min_spatial_segmentation_idc =*/ bitstream_read_ue(stream);
		/*int max_bytes_per_pic_denom =*/ bitstream_read_ue(stream);
		/*int max_bits_per_mb_denom =*/ bitstream_read_ue(stream);
		/*int log2_max_mv_length_horizontal =*/ bitstream_read_ue(stream);
		/*int log2_max_mv_length_vertical =*/ bitstream_read_ue(stream);
	}

	return 0;
}
