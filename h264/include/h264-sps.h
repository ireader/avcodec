#ifndef _h264_sps_h_
#define _h264_sps_h_

#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>

typedef unsigned char bool_t;

struct h264_sps_t
{
	uint8_t profile_idc;
	uint8_t constraint_set_flag;
	uint8_t level_idc;
	uint8_t seq_parameter_set_id; // [0, 31]
	uint8_t chroma_format_idc; // 0-4:0:0, 1-4:2:0, 2-4:2:2, 3-4:4:4, defalut: 1, see more: Table 6-1 p42

	struct { // profile_idc=[100, 110, 122, 244, 44, 83, 86, 118, 128, 138, 139, 134]
		bool_t separate_colour_plane_flag;
		uint8_t bit_depth_luma_minus8; // [0, 6]
		uint8_t bit_depth_chroma_minus8; // [0, 6]
		bool_t qpprime_y_zero_transform_bypass_flag; // bool
		bool_t seq_scaling_matrix_present_flag; // bool
												//int ScalingList4x4[6][16];
												//int ScalingList8x8[6][64];
												//int UseDefaultScalingMatrix4x4Flag[6];
												//int UseDefaultScalingMatrix8x8Flag[6];
												//int pic_scaling_list_present_flag[12];
	} chroma;

	uint8_t log2_max_frame_num_minus4; // [0, 12]
	uint8_t pic_order_cnt_type; // [0, 2]
	uint8_t log2_max_pic_order_cnt_lsb_minus4; // [0, 12]
	bool_t	delta_pic_order_always_zero_flag;
	int32_t offset_for_non_ref_pic;
	int32_t offset_for_top_to_bottom_field;
	uint8_t num_ref_frames_in_pic_order_cnt_cycle; // [0, 255]
	int32_t *offset_for_ref_frame;
	uint32_t max_num_ref_frames; // [0, MaxDpbFrames]
	bool_t	gaps_in_frame_num_value_allowed_flag;
	uint32_t pic_width_in_mbs_minus1;
	uint32_t pic_height_in_map_units_minus1;
	bool_t frame_mbs_only_flag;
	bool_t mb_adaptive_frame_field_flag;
	bool_t direct_8x8_inference_flag;

	bool_t frame_cropping_flag;
	struct {
		int32_t frame_crop_left_offset;
		int32_t frame_crop_right_offset;
		int32_t frame_crop_top_offset;
		int32_t frame_crop_bottom_offset;
	} frame_cropping;

	bool_t vui_parameters_present_flag;
};

int h264_parse_sps(const void* data, uint32_t bytes, struct h264_sps_t* sps);

#endif /* !_h264_sps_h_ */
