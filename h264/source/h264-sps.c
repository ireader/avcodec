// Rec. ITU-T H.264 (02/2016)
// 7.3.2.1.1 Sequence parameter set data syntax (p66)

#include "h264-sps.h"
#include "h264-vui.h"
#include "h264-internal.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int h264_sps_parse(const void* h264, uint32_t bytes, struct h264_sps_t* sps)
{
	bitstream_t stream;
	struct h264_nal_t nal;
	bitstream_init(&stream, (const unsigned char*)h264, bytes);

	h264_nal(&stream, &nal);
	if (H264_NAL_SPS != nal.nal_unit_type || nal.nal_ref_idc < 1)
		return -1; // invalid NALU

	h264_sps(&stream, sps);
	return 0;
}

int h264_sps(bitstream_t* stream, struct h264_sps_t* sps)
{
	int i;
	memset(sps, 0, sizeof(struct h264_sps_t));
	sps->chroma_format_idc = 1;
	sps->profile_idc = (uint8_t)bitstream_read_bits(stream, 8);
	sps->constraint_set_flag = (uint8_t)bitstream_read_bits(stream, 8);
	sps->level_idc = (uint8_t)bitstream_read_bits(stream, 8);
	sps->seq_parameter_set_id = (uint8_t)bitstream_read_ue(stream);
	if( sps->profile_idc == 100 || sps->profile_idc == 110 ||
		sps->profile_idc == 122 || sps->profile_idc == 244 || sps->profile_idc == 44  ||
		sps->profile_idc == 83  || sps->profile_idc == 86  || sps->profile_idc == 118 ||
		sps->profile_idc == 128 || sps->profile_idc == 138 || sps->profile_idc == 139 ||
		sps->profile_idc == 134 || sps->profile_idc == 135)
	{
		sps->chroma_format_idc = (uint8_t)bitstream_read_ue(stream);
		if(3 == sps->chroma_format_idc)
			sps->chroma.separate_colour_plane_flag = (uint8_t)bitstream_read_bit(stream);
		sps->chroma.bit_depth_luma_minus8 = (uint8_t)bitstream_read_ue(stream);
		sps->chroma.bit_depth_chroma_minus8 = (uint8_t)bitstream_read_ue(stream);
		sps->chroma.qpprime_y_zero_transform_bypass_flag = (uint8_t)bitstream_read_bit(stream);
		sps->chroma.seq_scaling_matrix_present_flag = (uint8_t)bitstream_read_bit(stream);
		if(sps->chroma.seq_scaling_matrix_present_flag)
		{
			for (i = 0; i < ((sps->chroma_format_idc != 3) ? 8 : 12) && i < sizeof_array(sps->chroma.seq_scaling_list_present_flag); i++)
			{
				sps->chroma.seq_scaling_list_present_flag[ i ] = (uint8_t)bitstream_read_bit(stream);
				if(sps->chroma.seq_scaling_list_present_flag[ i ])
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
		sps->delta_pic_order_always_zero_flag = (uint8_t)bitstream_read_bit(stream);
		sps->offset_for_non_ref_pic = (int32_t)bitstream_read_se(stream);
		sps->offset_for_top_to_bottom_field = (int32_t)bitstream_read_se(stream);
		sps->num_ref_frames_in_pic_order_cnt_cycle = (uint8_t)bitstream_read_ue(stream);
		assert(sps->num_ref_frames_in_pic_order_cnt_cycle < sizeof(sps->offset_for_ref_frame) / sizeof(sps->offset_for_ref_frame[0]));
		for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle && i < sizeof_array(sps->offset_for_ref_frame); i++)
			sps->offset_for_ref_frame[i] = (int32_t)bitstream_read_se(stream);
	}

	sps->max_num_ref_frames = (uint32_t)bitstream_read_ue(stream);
	sps->gaps_in_frame_num_value_allowed_flag = (uint8_t)bitstream_read_bit(stream);
	sps->pic_width_in_mbs_minus1 = (uint32_t)bitstream_read_ue(stream);
	sps->pic_height_in_map_units_minus1 = (uint32_t)bitstream_read_ue(stream);
	sps->frame_mbs_only_flag = (uint8_t)bitstream_read_bit(stream);
	if(!sps->frame_mbs_only_flag)
		sps->mb_adaptive_frame_field_flag = (uint8_t)bitstream_read_bit(stream);
	sps->direct_8x8_inference_flag = (uint8_t)bitstream_read_bit(stream);
	sps->frame_cropping_flag = (uint8_t)bitstream_read_bit(stream);
	if(sps->frame_cropping_flag)
	{
		sps->frame_cropping.frame_crop_left_offset	= (int32_t)bitstream_read_ue(stream);
		sps->frame_cropping.frame_crop_right_offset	= (int32_t)bitstream_read_ue(stream);
		sps->frame_cropping.frame_crop_top_offset	= (int32_t)bitstream_read_ue(stream);
		sps->frame_cropping.frame_crop_bottom_offset= (int32_t)bitstream_read_ue(stream);
	}
	sps->vui_parameters_present_flag = (uint8_t)bitstream_read_bit(stream);
	if(sps->vui_parameters_present_flag)
	{
		memset(&sps->vui, 0, sizeof(sps->vui));
		h264_vui(stream, &sps->vui);
	}

	return h264_rbsp_trailing_bits(stream);
}

static int h264_crop_unit(const struct h264_sps_t* sps, int* x, int *y)
{
	const int SubWidthC[] = { 0 /*4:0:0*/, 2 /*4:2:0*/, 2 /*4:2:2*/, 1 /*4:4:4*/ };
	const int SubHeightC[] = { 0 /*4:0:0*/, 2 /*4:2:0*/, 1 /*4:2:2*/, 1 /*4:4:4*/ };
	int chroma_array_type;
	int frame_mbs_only_flag;

	frame_mbs_only_flag = sps->frame_mbs_only_flag ? 1 : 0;

	// Depending on the value of separate_colour_plane_flag, the value of the variable ChromaArrayType is assigned as follows:
	// - If separate_colour_plane_flag is equal to 0, ChromaArrayType is set equal to chroma_format_idc.
	// - Otherwise (separate_colour_plane_flag is equal to 1), ChromaArrayType is set equal to 0.
	chroma_array_type = sps->chroma.separate_colour_plane_flag ? 0 : sps->chroma_format_idc;
	if (chroma_array_type > 3)
		return -1;

	/*
	The variables CropUnitX and CropUnitY are derived as follows:
	- If ChromaArrayType is equal to 0, CropUnitX and CropUnitY are derived as:
		CropUnitX = 1
		CropUnitY = 2 - frame_mbs_only_flag
	- Otherwise (ChromaArrayType is equal to 1, 2, or 3), CropUnitX and CropUnitY are derived as:
		CropUnitX = SubWidthC
		CropUnitY = SubHeightC * ( 2 - frame_mbs_only_flag )
	*/
	if (0 == chroma_array_type)
	{
		*x = 1;
		*y = 2 - frame_mbs_only_flag;
	}
	else
	{
		*x = SubWidthC[chroma_array_type];
		*y = SubHeightC[chroma_array_type] * (2 - frame_mbs_only_flag);
	}

	return 0;
}

int h264_codec_rect(const struct h264_sps_t* sps, int* x, int *y, int *w, int *h)
{
	int dx, dy;
	int pic_width_in_mbs;
	int pic_height_in_mbs;
	int frame_mbs_only_flag;
	if (0 != h264_crop_unit(sps, &dx, &dy))
		return -1;

	frame_mbs_only_flag = sps->frame_mbs_only_flag ? 1 : 0;
	pic_width_in_mbs = sps->pic_width_in_mbs_minus1 + 1;
	pic_height_in_mbs = (sps->pic_height_in_map_units_minus1 + 1) * (2 - frame_mbs_only_flag);

	*x = 0;
	*y = 0;
	*w = pic_width_in_mbs * 16;
	*h = pic_height_in_mbs * 16;
	return 0;
}

int h264_display_rect(const struct h264_sps_t* sps, int* x, int *y, int *w, int *h)
{
	int dx, dy;
	int pic_width_in_mbs;
	int pic_height_in_mbs;
	int frame_mbs_only_flag;
	if (0 != h264_crop_unit(sps, &dx, &dy))
		return -1;

	frame_mbs_only_flag = sps->frame_mbs_only_flag ? 1 : 0;
	pic_width_in_mbs = sps->pic_width_in_mbs_minus1 + 1;
	pic_height_in_mbs = (sps->pic_height_in_map_units_minus1 + 1) * (2 - frame_mbs_only_flag);

	*x = sps->frame_cropping_flag ? sps->frame_cropping.frame_crop_left_offset * dx : 0;
	*y = sps->frame_cropping_flag ? sps->frame_cropping.frame_crop_top_offset * dy : 0;
	*w = sps->frame_cropping_flag ? sps->frame_cropping.frame_crop_right_offset * dx : 0;
	*h = sps->frame_cropping_flag ? sps->frame_cropping.frame_crop_bottom_offset * dy : 0;
	*w = pic_width_in_mbs * 16 - *w - *x;
	*h = pic_height_in_mbs * 16 - *h - *y;
	return 0;
}

#if defined(DEBUG) || defined(_DEBUG)
void h264_sps_print(const struct h264_sps_t* sps)
{
	int x, y, w, h;
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
	h264_codec_rect(sps, &x, &y, &w, &h);
	printf(" codec rect: %d/%d/%d/%d", x, y, w, h);
	h264_display_rect(sps, &x, &y, &w, &h);
	printf(" display rect: %d/%d/%d/%d\n", x, y, w, h);

	printf(" vui_parameters_present_flag: %s\n", sps->vui_parameters_present_flag ? "true" : "false");
}

static void h264_sps_vui_test()
{
	const uint8_t vui264[] = { 0x67, 0x64, 0x00, 0x1f, 0xac, 0x2c, 0x6a, 0x81, 0x40, 0x16, 0xe9, 0xb8, 0x08, 0x08, 0x0a, 0x00, 0x00, 0x07, 0xd0, 0x00, 0x01, 0x86, 0xa1, 0x08 };

	bitstream_t stream;
	struct h264_nal_t nal;
	struct h264_sps_t sps;
	memset(&sps, 0, sizeof(sps));
	bitstream_init(&stream, vui264, sizeof(vui264));

	assert(0 == h264_nal(&stream, &nal));
	assert(H264_NAL_SPS == nal.nal_unit_type && nal.nal_ref_idc > 0);
	assert(0 == h264_sps(&stream, &sps));
}

static void h264_sps_parse_test(const uint8_t* nalu, uint32_t bytes, const struct h264_sps_t* check)
{
	bitstream_t stream;
	struct h264_nal_t nal;
	struct h264_sps_t sps;
	memset(&sps, 0, sizeof(sps));
	bitstream_init(&stream, nalu, bytes);

	assert(0 == h264_nal(&stream, &nal));
	assert(H264_NAL_SPS == nal.nal_unit_type && nal.nal_ref_idc > 0);
	assert(0 == h264_sps(&stream, &sps));
	assert(0 == memcmp(check, &sps, sizeof(struct h264_sps_t)));
}

void h264_sps_test()
{
	//const uint8_t rawh264[] = { 0x67, 0x42, 0xa0, 0x1e, 0x97, 0x40, 0x58, 0x09, 0x22 };
    //const uint8_t rawh264[] = { 0x67, 0x64, 0x00, 0x1f, 0xad, 0x00, 0xce, 0x50, 0x14, 0x01, 0x6e, 0xc0, 0x44, 0x00, 0x00, 0x38, 0x40, 0x00, 0x0a, 0xfc, 0x81, 0x80, 0x00, 0x00, 0x35, 0x67, 0xe0, 0x00, 0x01, 0xab, 0x3f, 0x08, 0xbd, 0xf8, 0xc0, 0x00, 0x00, 0x1a, 0xb3, 0xf0, 0x00, 0x00, 0xd5, 0x9f, 0x84, 0x5e, 0xfc, 0x7b, 0x41, 0x10, 0x89, 0x4b };
    //const uint8_t rawh264[] = { 0x67, 0x64, 0x00, 0x33, 0xad, 0x84, 0x05, 0x45, 0x62, 0xb8, 0xac, 0x54, 0x74, 0x20, 0x2a, 0x2b, 0x15, 0xc5, 0x62, 0xa3, 0xa1, 0x01, 0x51, 0x58, 0xae, 0x2b, 0x15, 0x1d, 0x08, 0x0a, 0x8a, 0xc5, 0x71, 0x58, 0xa8, 0xe8, 0x40, 0x54, 0x56, 0x2b, 0x8a, 0xc5, 0x47, 0x42, 0x02, 0xa2, 0xb1, 0x5c, 0x56, 0x2a, 0x3a, 0x10, 0x24, 0x99, 0x39, 0x3c, 0x9f, 0x27, 0xe4, 0xfe, 0x4f, 0xc9, 0xf2, 0x79, 0xb9, 0xb3, 0x4d, 0x08, 0x12, 0x4c, 0x9c, 0x9e, 0x4f, 0x93, 0xf2, 0x7f, 0x27, 0xe4, 0xf9, 0x3c, 0xdc, 0xd9, 0xa6, 0xb4, 0x03, 0xc0, 0x11, 0x3f, 0x2a, };
	//const uint8_t xcrop[] = { 0x67, 0x4d, 0x40, 0x1f, 0xe8, 0x80, 0x6c, 0x1e, 0xf3, 0x78, 0x08, 0x80, 0x00, 0x01, 0xf4, 0x80, 0x00, 0x75, 0x30, 0x07, 0x8c, 0x18, 0x89 };
	//const uint8_t ycrop[] = { 0x67, 0x42, 0xc0, 0x1e, 0xda, 0x02, 0x80, 0xbf, 0xe5, 0x84, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00, 0xc0, 0x3c, 0x58, 0xba, 0x80 };
    //const uint8_t vuisar[] = {0x67, 0x64, 0x00, 0x1f, 0xac, 0xe4, 0x01, 0x40, 0x16, 0xec, 0x04, 0x40, 0x00, 0x00, 0x03, 0x00, 0x40, 0x00, 0x00, 0x0c, 0xb9, 0xa8, 0x00, 0x24, 0x9f, 0x00, 0x04, 0x93, 0xe9, 0x2c, 0xc0, 0x1e, 0x2c, 0x5c, 0x90, 0x01, 0x00, 0x04 };
    const uint8_t vuisar[] = {0x67, 0x64, 0x00, 0x1e, 0xac, 0xd9, 0x80, 0xa8, 0x39, 0xe2, 0x3f, 0xfc, 0x21, 0x88, 0x1f, 0x64, 0x40, 0x00, 0x00, 0x03, 0x00, 0x40, 0x00, 0x00, 0x0c, 0x83, 0xc5, 0x8b, 0x66, 0x80, 0x01, 0x00};
	struct h264_sps_t sps;
	memset(&sps, 0, sizeof(sps));
	h264_sps_vui_test();
	sps.chroma_format_idc = 1;
	sps.profile_idc = 66;
	sps.constraint_set_flag = 160;
	sps.level_idc = 30;
	sps.seq_parameter_set_id = 0;
	sps.log2_max_frame_num_minus4 = 4;
	sps.pic_order_cnt_type = 0;
	sps.max_num_ref_frames = 1;
	sps.gaps_in_frame_num_value_allowed_flag = 0;
	sps.pic_width_in_mbs_minus1 = 43;
	sps.pic_height_in_map_units_minus1 = 35;
	sps.frame_mbs_only_flag = 1;
	h264_sps_parse_test(vuisar, sizeof(vuisar), &sps);
}
#endif
