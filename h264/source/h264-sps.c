// Rec. ITU-T H.264 (02/2016)
// 7.3.2.1.1 Sequence parameter set data syntax (p66)

#include "h264-sps.h"
#include "h264-nalu.h"
#include "h264-vui.h"
#include "h264-scaling.h"
#include "h264-internal.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void h264_sps(bitstream_t* stream, struct h264_sps_t* sps)
{
	int i;
	sps->chroma_format_idc = 1;
	sps->profile_idc = (uint8_t)bitstream_read_bits(stream, 8);
	sps->constraint_set_flag = (uint8_t)bitstream_read_bits(stream, 8);
	sps->level_idc = (uint8_t)bitstream_read_bits(stream, 8);
	sps->seq_parameter_set_id = (uint8_t)bitstream_read_ue(stream);
	if( sps->profile_idc == 100 || sps->profile_idc == 110 ||
		sps->profile_idc == 122 || sps->profile_idc == 244 || sps->profile_idc == 44  ||
		sps->profile_idc == 83  || sps->profile_idc == 86  || sps->profile_idc == 118 ||
		sps->profile_idc == 128 || sps->profile_idc == 138 || sps->profile_idc == 139 ||
		sps->profile_idc == 134)
	{
		sps->chroma_format_idc = (uint8_t)bitstream_read_ue(stream);
		if(3 == sps->chroma_format_idc)
			sps->chroma.separate_colour_plane_flag = (bool_t)bitstream_read_bit(stream);
		sps->chroma.bit_depth_luma_minus8 = (uint8_t)bitstream_read_ue(stream);
		sps->chroma.bit_depth_chroma_minus8 = (uint8_t)bitstream_read_ue(stream);
		sps->chroma.qpprime_y_zero_transform_bypass_flag = (bool_t)bitstream_read_bit(stream);
		sps->chroma.seq_scaling_matrix_present_flag = (bool_t)bitstream_read_bit(stream);
		if(sps->chroma.seq_scaling_matrix_present_flag)
		{
			for(i=0; i<((sps->chroma_format_idc!=3)?8:12); i++)
			{
				sps->chroma.pic_scaling_list_present_flag[ i ] = bitstream_read_bit(stream);
				if(sps->chroma.pic_scaling_list_present_flag[ i ])
				{
					if(i < 6)
					{
						h264_scaling_list_4x4(stream, sps->chroma.ScalingList4x4[i], &sps->chroma.UseDefaultScalingMatrix4x4Flag[i]);
					}
					else
					{
						h264_scaling_list_8x8(stream, sps->chroma.ScalingList8x8[i-6], &sps->chroma.UseDefaultScalingMatrix8x8Flag[i-6]);
					}
				}
			}
		}
	}

	sps->log2_max_frame_num_minus4 = (uint8_t)bitstream_read_ue(stream);
	sps->pic_order_cnt_type = (uint8_t)bitstream_read_ue(stream);
	if(0 == sps->pic_order_cnt_type)
	{
		sps->log2_max_pic_order_cnt_lsb_minus4 = (uint8_t)bitstream_read_ue(stream);
	}
	else if(1 == sps->pic_order_cnt_type)
	{
		sps->delta_pic_order_always_zero_flag = (bool_t)bitstream_read_bit(stream);
		sps->offset_for_non_ref_pic = (int32_t)bitstream_read_se(stream);
		sps->offset_for_top_to_bottom_field = (int32_t)bitstream_read_se(stream);
		sps->num_ref_frames_in_pic_order_cnt_cycle = (uint8_t)bitstream_read_ue(stream);
		sps->offset_for_ref_frame = (int*)malloc(sps->num_ref_frames_in_pic_order_cnt_cycle * sizeof(int));
		for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
			sps->offset_for_ref_frame[i] = (int32_t)bitstream_read_se(stream);
	}

	sps->max_num_ref_frames = (uint32_t)bitstream_read_ue(stream);
	sps->gaps_in_frame_num_value_allowed_flag = (bool_t)bitstream_read_bit(stream);
	sps->pic_width_in_mbs_minus1 = (uint32_t)bitstream_read_ue(stream);
	sps->pic_height_in_map_units_minus1 = (uint32_t)bitstream_read_ue(stream);
	sps->frame_mbs_only_flag = (bool_t)bitstream_read_bit(stream);
	if(!sps->frame_mbs_only_flag)
		sps->mb_adaptive_frame_field_flag = (bool_t)bitstream_read_bit(stream);
	sps->direct_8x8_inference_flag = (bool_t)bitstream_read_bit(stream);
	sps->frame_cropping_flag = (bool_t)bitstream_read_bit(stream);
	if(sps->frame_cropping_flag)
	{
		sps->frame_cropping.frame_crop_left_offset	= (int32_t)bitstream_read_ue(stream);
		sps->frame_cropping.frame_crop_right_offset	= (int32_t)bitstream_read_ue(stream);
		sps->frame_cropping.frame_crop_top_offset	= (int32_t)bitstream_read_ue(stream);
		sps->frame_cropping.frame_crop_bottom_offset= (int32_t)bitstream_read_ue(stream);
	}
	sps->vui_parameters_present_flag = (bool_t)bitstream_read_bit(stream);
	if(sps->vui_parameters_present_flag)
	{
		h264_vui_parameters(stream);
	}

	h264_rbsp_trailing_bits(stream);
}

#if defined(DEBUG) || defined(_DEBUG)
void h264_sps_print(const struct h264_sps_t* sps)
{
	printf("H.264 Sequence parameter set:\n");
	printf(" profile_idc: %hhu\n", sps->profile_idc);
	printf(" constraint_set_flag: %02hhx\n", sps->constraint_set_flag);
	printf(" level_idc: %hhu\n", sps->level_idc);
	printf(" seq_parameter_set_id: %hhu\n", sps->seq_parameter_set_id);
	printf(" chroma_format_idc: %hhu\n", sps->chroma_format_idc);
	if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
		sps->profile_idc == 122 || sps->profile_idc == 244 || sps->profile_idc == 44 ||
		sps->profile_idc == 83 || sps->profile_idc == 86 || sps->profile_idc == 118 ||
		sps->profile_idc == 128 || sps->profile_idc == 138 || sps->profile_idc == 139 ||
		sps->profile_idc == 134)
	{
		if (3 == sps->chroma_format_idc)
			printf("   separate_colour_plane_flag: %s\n", sps->chroma.separate_colour_plane_flag ? "true" : "false");
		printf("   bit_depth_luma_minus8: %hhu\n", sps->chroma.bit_depth_luma_minus8);
		printf("   bit_depth_chroma_minus8: %hhu\n", sps->chroma.bit_depth_chroma_minus8);
		printf("   qpprime_y_zero_transform_bypass_flag: %s\n", sps->chroma.qpprime_y_zero_transform_bypass_flag ? "true" : "false");
		printf("   seq_scaling_matrix_present_flag: %s\n", sps->chroma.seq_scaling_matrix_present_flag ? "true" : "false");
	}
	printf(" log2_max_frame_num_minus4: %hhu\n", sps->log2_max_frame_num_minus4);
	printf(" pic_order_cnt_type: %hhu\n", sps->pic_order_cnt_type);
	if (0 == sps->pic_order_cnt_type)
	{
		printf(" log2_max_pic_order_cnt_lsb_minus4: %hhu\n", sps->log2_max_pic_order_cnt_lsb_minus4);
	}
	else if (1 == sps->pic_order_cnt_type)
	{
		printf(" delta_pic_order_always_zero_flag: %s\n", sps->delta_pic_order_always_zero_flag ? "true" : "false");
		printf(" offset_for_non_ref_pic: %d\n", sps->offset_for_non_ref_pic);
		printf(" offset_for_top_to_bottom_field: %d\n", sps->offset_for_top_to_bottom_field);
		printf(" num_ref_frames_in_pic_order_cnt_cycle: %hhu\n", sps->num_ref_frames_in_pic_order_cnt_cycle);
	}
	printf(" max_num_ref_frames: %u\n", sps->max_num_ref_frames);
	printf(" gaps_in_frame_num_value_allowed_flag: %s\n", sps->gaps_in_frame_num_value_allowed_flag ? "true" : "false");
	printf(" pic_width_in_mbs_minus1: %u\n", sps->pic_width_in_mbs_minus1);
	printf(" pic_height_in_map_units_minus1: %u\n", sps->pic_height_in_map_units_minus1);
	printf(" frame_mbs_only_flag: %s\n", sps->frame_mbs_only_flag ? "true" : "false");
	if (!sps->frame_mbs_only_flag)
		printf(" mb_adaptive_frame_field_flag: %s\n", sps->mb_adaptive_frame_field_flag ? "true" : "false");
	printf(" direct_8x8_inference_flag: %s\n", sps->direct_8x8_inference_flag ? "true" : "false");
	printf(" frame_cropping_flag: %s\n", sps->frame_cropping_flag ? "true" : "false");
	if (sps->frame_cropping_flag)
	{
		printf("   frame_crop_left_offset: %d\n", sps->frame_cropping.frame_crop_left_offset);
		printf("   frame_crop_right_offset: %d\n", sps->frame_cropping.frame_crop_right_offset);
		printf("   frame_crop_top_offset: %d\n", sps->frame_cropping.frame_crop_top_offset);
		printf("   frame_crop_bottom_offset: %d\n", sps->frame_cropping.frame_crop_bottom_offset);
	}
	printf(" vui_parameters_present_flag: %s\n", sps->vui_parameters_present_flag ? "true" : "false");
}
#endif

int h264_sps_parse(const void* data, uint32_t bytes, struct h264_sps_t* sps)
{
	bitstream_t* stream;
	stream = bitstream_create((const unsigned char*)data, bytes);
	if (!stream)
		return ENOMEM;

	int forbidden_zero_bit = bitstream_read_bit(stream);
	int nal_ref_idc = bitstream_read_bits(stream, 2);
	int nal_unit_type = bitstream_read_bits(stream, 5);

	if (0 != forbidden_zero_bit || nal_ref_idc < 1)
		return -1; // invalid h.264 data

	if (H264_NALU_SPS != nal_unit_type)
		return -1; // invalid NALU

	h264_sps(stream, sps);
	return 0;
}
