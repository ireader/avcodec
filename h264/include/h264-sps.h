#ifndef _h264_sps_h_
#define _h264_sps_h_

#include "h264-vui.h"
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>

struct h264_sps_t
{
	uint8_t profile_idc;
	uint8_t constraint_set_flag;
	uint8_t level_idc;
	uint8_t seq_parameter_set_id; // [0, 31]
	uint8_t chroma_format_idc; // 0-4:0:0, 1-4:2:0, 2-4:2:2, 3-4:4:4, defalut: 1, see more: Table 6-1 p42

	struct { // profile_idc=[100, 110, 122, 244, 44, 83, 86, 118, 128, 138, 139, 134]
		uint8_t separate_colour_plane_flag; // bool
		uint8_t bit_depth_luma_minus8; // [0, 6]
		uint8_t bit_depth_chroma_minus8; // [0, 6]
		uint8_t qpprime_y_zero_transform_bypass_flag; // bool

		uint8_t seq_scaling_matrix_present_flag; // bool
		//if( seq_scaling_matrix_present_flag ) {
			uint8_t seq_scaling_list_present_flag[12]; // bool
			uint8_t UseDefaultScalingMatrix4x4Flag[6]; // bool
			uint8_t UseDefaultScalingMatrix8x8Flag[6]; // bool
			int32_t ScalingList4x4[6][16];
			int32_t ScalingList8x8[6][64];
		//}
	} chroma;

	uint8_t log2_max_frame_num_minus4; // [0, 12]
	uint8_t pic_order_cnt_type; // [0, 2]
	uint8_t log2_max_pic_order_cnt_lsb_minus4; // [0, 12]
	uint8_t	delta_pic_order_always_zero_flag; // bool
	int32_t offset_for_non_ref_pic;
	int32_t offset_for_top_to_bottom_field;
	uint8_t num_ref_frames_in_pic_order_cnt_cycle; // [0, 255]
	int32_t offset_for_ref_frame[64];
	uint32_t max_num_ref_frames; // [0, MaxDpbFrames]
	uint8_t	gaps_in_frame_num_value_allowed_flag; // bool
	uint32_t pic_width_in_mbs_minus1;
	uint32_t pic_height_in_map_units_minus1;
	uint8_t frame_mbs_only_flag; // bool
	uint8_t mb_adaptive_frame_field_flag; // bool
	uint8_t direct_8x8_inference_flag; // bool

	uint8_t frame_cropping_flag; // bool
	struct {
		int32_t frame_crop_left_offset;
		int32_t frame_crop_right_offset;
		int32_t frame_crop_top_offset;
		int32_t frame_crop_bottom_offset;
	} frame_cropping;

	uint8_t vui_parameters_present_flag; // bool
	struct h264_vui_t vui;
};

int h264_sps_parse(const void* data, uint32_t bytes, struct h264_sps_t* sps);

int h264_codec_rect(const struct h264_sps_t* sps, int* x, int *y, int *w, int *h);
int h264_display_rect(const struct h264_sps_t* sps, int* x, int *y, int *w, int *h);

#endif /* !_h264_sps_h_ */
