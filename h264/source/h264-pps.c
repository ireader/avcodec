#include "h264-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void pic_parameter_set_rbsp(bitstream_t* stream)
{
	int pic_parameter_set_id = bitstream_read_ue(stream);
	int seq_parameter_set_id = bitstream_read_ue(stream);
	int entropy_coding_mode_flag = bitstream_read_bit(stream);
	int bottom_field_pic_order_in_frame_present_flag = bitstream_read_bit(stream);
	int num_slice_groups_minus1 = bitstream_read_ue(stream);
	if (num_slice_groups_minus1 > 0)
	{
		int slice_group_map_type = bitstream_read_ue(stream);
		if (0 == slice_group_map_type)
		{
			int run_length_minus1[64];
			for (int iGroup = 0; iGroup < num_slice_groups_minus1; ++iGroup)
			{
				run_length_minus1[iGroup] = bitstream_read_ue(stream);
			}
		}
		else if (2 == slice_group_map_type)
		{
			int top_left[64];
			int bottom_right[64];
			for (int iGroup = 0; iGroup < num_slice_groups_minus1; ++iGroup)
			{
				top_left[iGroup] = bitstream_read_ue(stream);
				bottom_right[iGroup] = bitstream_read_ue(stream);
			}
		}
		else if (3 == slice_group_map_type || 4 == slice_group_map_type || 5 == slice_group_map_type)
		{
			int slice_group_change_direction_flag = bitstream_read_bit(stream);
			int slice_group_change_rate_minus1 = bitstream_read_ue(stream);
		}
		else if (6 == slice_group_map_type)
		{
			int slice_group_id[64];
			int pic_size_in_map_units_minus1 = bitstream_read_ue(stream);
			for (int i = 0; i < pic_size_in_map_units_minus1; i++)
			{
				slice_group_id[i] = bitstream_read_ue(stream);
			}
		}
	}

	int num_ref_idx_l0_default_active_minus1 = bitstream_read_ue(stream);
	int num_ref_idx_l1_default_active_minus1 = bitstream_read_ue(stream);
	int weighted_pred_flag = bitstream_read_bit(stream);
	int weighted_bipred_idc = bitstream_read_bits(stream, 2);
	int pic_init_qp_minus26 = bitstream_read_se(stream);
	int pic_init_qs_minus26 = bitstream_read_se(stream);
	int chroma_qp_index_offset = bitstream_read_se(stream);
	int deblocking_filter_control_present_flag = bitstream_read_bit(stream);
	int constrained_intra_pred_flag = bitstream_read_bit(stream);
	int redundant_pic_cnt_present_flag = bitstream_read_bit(stream);

	if (h264_more_rbsp_data(stream))
	{
		int transform_8x8_mode_flag = bitstream_read_bit(stream);
		int pic_scaling_matrix_present_flag = bitstream_read_bit(stream);
		if (pic_scaling_matrix_present_flag)
		{
			int ScalingList4x4[16][64];
			int ScalingList8x8[64][64];
			int UseDefaultScalingMatrix4x4Flag[16];
			int UseDefaultScalingMatrix8x8Flag[16];

			int pic_scaling_list_present_flag[64];
			int chroma_format_idc = 1; // 4:2:0
			for (int i = 0; i < 6 + ((chroma_format_idc != 3) ? 2 : 6) * transform_8x8_mode_flag; i++)
			{
				pic_scaling_list_present_flag[i] = bitstream_read_bit(stream);
				if (pic_scaling_list_present_flag[i])
				{
					if (i < 6)
					{
						h264_scaling_list(stream, ScalingList4x4[i], 16, &UseDefaultScalingMatrix4x4Flag[i]);
					}
					else
					{
						h264_scaling_list(stream, ScalingList8x8[i - 6], 64, &UseDefaultScalingMatrix8x8Flag[i - 6]);
					}
				}
			}
		}
		int second_chroma_qp_index_offset = bitstream_read_se(stream);
	}

	h264_rbsp_trailing_bits(stream);
}
