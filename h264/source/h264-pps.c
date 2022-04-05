// Rec. ITU-T H.264 (02/2016)
// 7.3.2.2 Picture parameter set RBSP syntax (p69)

#include "h264-pps.h"
#include "h264-internal.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int h264_pps(bitstream_t* stream, struct h264_context_t* h264, struct h264_pps_t* pps)
{
	memset(pps, 0, sizeof(struct h264_pps_t));
	pps->pic_parameter_set_id = bitstream_read_ue(stream);
	pps->seq_parameter_set_id = bitstream_read_ue(stream);
	pps->entropy_coding_mode_flag = (uint8_t)bitstream_read_bit(stream);
	pps->bottom_field_pic_order_in_frame_present_flag = (uint8_t)bitstream_read_bit(stream);
	pps->num_slice_groups_minus1 = bitstream_read_ue(stream);
	if (pps->num_slice_groups_minus1 > 0)
	{
		pps->slice_group_map_type = bitstream_read_ue(stream);
		if (0 == pps->slice_group_map_type)
		{
			uint32_t iGroup;
			for (iGroup = 0; iGroup < pps->num_slice_groups_minus1 && iGroup < sizeof_array(pps->group.run_length_minus1); ++iGroup)
			{
				pps->group.run_length_minus1[iGroup] = bitstream_read_ue(stream);
			}
		}
		else if (2 == pps->slice_group_map_type)
		{
			uint32_t iGroup;
			for (iGroup = 0; iGroup < pps->num_slice_groups_minus1 && iGroup < sizeof_array(pps->group.dispersed.top_left); ++iGroup)
			{
				pps->group.dispersed.top_left[iGroup] = bitstream_read_ue(stream);
				pps->group.dispersed.bottom_right[iGroup] = bitstream_read_ue(stream);
			}
		}
		else if (3 == pps->slice_group_map_type || 4 == pps->slice_group_map_type || 5 == pps->slice_group_map_type)
		{
			pps->group.direction.slice_group_change_direction_flag = (uint8_t)bitstream_read_bit(stream);
			pps->group.direction.slice_group_change_rate_minus1 = bitstream_read_ue(stream);
		}
		else if (6 == pps->slice_group_map_type)
		{
			uint32_t i;
			pps->group.groups.pic_size_in_map_units_minus1 = bitstream_read_ue(stream);
			for (i = 0; i < pps->group.groups.pic_size_in_map_units_minus1; i++)
			{
				/*pps->group.groups.slice_group_id[i] =*/ bitstream_read_ue(stream);
			}
		}
	}

	pps->num_ref_idx_l0_default_active_minus1 = bitstream_read_ue(stream);
	pps->num_ref_idx_l1_default_active_minus1 = bitstream_read_ue(stream);
	pps->weighted_pred_flag = (uint8_t)bitstream_read_bit(stream);
	pps->weighted_bipred_idc = bitstream_read_bits(stream, 2);
	pps->pic_init_qp_minus26 = bitstream_read_se(stream);
	pps->pic_init_qs_minus26 = bitstream_read_se(stream);
	pps->chroma_qp_index_offset = bitstream_read_se(stream);
	pps->deblocking_filter_control_present_flag = (uint8_t)bitstream_read_bit(stream);
	pps->constrained_intra_pred_flag = (uint8_t)bitstream_read_bit(stream);
	pps->redundant_pic_cnt_present_flag = (uint8_t)bitstream_read_bit(stream);

	if (h264_more_rbsp_data(stream))
	{
		pps->transform_8x8_mode_flag = (uint8_t)bitstream_read_bit(stream);
		pps->pic_scaling_matrix_present_flag = (uint8_t)bitstream_read_bit(stream);
		if (pps->pic_scaling_matrix_present_flag)
		{
			int i;
			struct h264_sps_t *sps;
			if (pps->seq_parameter_set_id >= sizeof(h264->sps) / sizeof(h264->sps[0]))
				return -1; // invalid seq_parameter_set_id
			sps = h264->sps + pps->seq_parameter_set_id;
			for (i = 0; i < 6 + ((sps->chroma_format_idc != 3) ? 2 : 6) * pps->transform_8x8_mode_flag && i < sizeof_array(pps->pic_scaling_list_present_flag); i++)
			{
				pps->pic_scaling_list_present_flag[i] = (uint8_t)bitstream_read_bit(stream);
				if (pps->pic_scaling_list_present_flag[i])
				{
					if (i < 6)
					{
						h264_scaling_list_4x4(stream, pps->ScalingList4x4[i], &pps->UseDefaultScalingMatrix4x4Flag[i]);
					}
					else
					{
						h264_scaling_list_8x8(stream, pps->ScalingList8x8[i-6], &pps->UseDefaultScalingMatrix8x8Flag[i-6]);
					}
				}
			}
		}
		pps->second_chroma_qp_index_offset = bitstream_read_se(stream);
	}

	return h264_rbsp_trailing_bits(stream);
}

#if defined(DEBUG) || defined(_DEBUG)
void h264_pps_print(const struct h264_pps_t* pps)
{
	printf("H.264 Picture parameter set:\n");
	printf(" pic_parameter_set_id: %u\n", pps->pic_parameter_set_id);
	printf(" seq_parameter_set_id: %u\n", pps->seq_parameter_set_id);
	printf(" entropy_coding_mode_flag: %hhu\n", pps->entropy_coding_mode_flag);
	printf(" bottom_field_pic_order_in_frame_present_flag: %hhu\n", pps->bottom_field_pic_order_in_frame_present_flag);
	printf(" num_slice_groups_minus1: %u\n", pps->num_slice_groups_minus1);
	if (pps->num_slice_groups_minus1 > 0)
	{
		printf("   slice_group_map_type: %u\n", pps->slice_group_map_type);
		if (0 == pps->slice_group_map_type)
		{
			uint32_t iGroup;
			for (iGroup = 0; iGroup < pps->num_slice_groups_minus1; ++iGroup)
			{
				printf("   run_length_minus1[%u]: %u\n", iGroup, pps->group.run_length_minus1[iGroup]);
			}
		}
		else if (2 == pps->slice_group_map_type)
		{
			uint32_t iGroup;
			for (iGroup = 0; iGroup < pps->num_slice_groups_minus1; ++iGroup)
			{
				printf("   top_left[%u]: %u\n", iGroup, pps->group.dispersed.top_left[iGroup]);
				printf("   bottom_right[%u]: %u\n", iGroup, pps->group.dispersed.bottom_right[iGroup]);
			}
		}
		else if (3 == pps->slice_group_map_type || 4 == pps->slice_group_map_type || 5 == pps->slice_group_map_type)
		{
			printf("   slice_group_change_direction_flag: %u\n", pps->group.direction.slice_group_change_direction_flag);
			printf("   slice_group_change_rate_minus1: %u\n", pps->group.direction.slice_group_change_rate_minus1);
		}
		else if (6 == pps->slice_group_map_type)
		{
		}
	}
	printf(" num_ref_idx_l0_default_active_minus1: %u\n", pps->num_ref_idx_l0_default_active_minus1);
	printf(" num_ref_idx_l1_default_active_minus1: %u\n", pps->num_ref_idx_l1_default_active_minus1);
	printf(" weighted_pred_flag: %u\n", pps->weighted_pred_flag);
	printf(" weighted_bipred_idc: %u\n", pps->weighted_bipred_idc);
	printf(" pic_init_qp_minus26: %d\n", pps->pic_init_qp_minus26);
	printf(" pic_init_qs_minus26: %d\n", pps->pic_init_qs_minus26);
	printf(" chroma_qp_index_offset: %d\n", pps->chroma_qp_index_offset);
	printf(" deblocking_filter_control_present_flag: %u\n", pps->deblocking_filter_control_present_flag);
	printf(" constrained_intra_pred_flag: %u\n", pps->constrained_intra_pred_flag);
	printf(" redundant_pic_cnt_present_flag: %u\n", pps->redundant_pic_cnt_present_flag);
}

static void h264_pps_parse_test(const uint8_t* nalu, uint32_t bytes, const struct h264_pps_t* check)
{
	bitstream_t stream;
	struct h264_nal_t nal;
	struct h264_pps_t pps;
	memset(&pps, 0, sizeof(pps));
	bitstream_init(&stream, nalu, bytes);

	assert(0 == h264_nal(&stream, &nal));
	assert(H264_NAL_PPS == nal.nal_unit_type && nal.nal_ref_idc > 0);
	assert(0 == h264_pps(&stream, NULL, &pps));
	assert(0 == memcmp(check, &pps, sizeof(struct h264_pps_t)));
}

void h264_pps_test()
{
	const uint8_t rawh264[] = { 0x68, 0xde, 0x3c, 0x80 };
	struct h264_pps_t pps;
	memset(&pps, 0, sizeof(pps));
	pps.pic_parameter_set_id = 0;
	pps.seq_parameter_set_id = 0;
	pps.entropy_coding_mode_flag = 0;
	pps.bottom_field_pic_order_in_frame_present_flag = 1;
	pps.num_slice_groups_minus1 = 0;
	pps.deblocking_filter_control_present_flag = 1;
	h264_pps_parse_test(rawh264, sizeof(rawh264), &pps);
}
#endif
