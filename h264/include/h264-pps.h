#ifndef _h264_pps_h_
#define _h264_pps_h_

#include <stdint.h>

#define H264_SLICE_GROUPS 8

enum h264_slice_group_map_type_enum
{
	h264_slice_group_map_type_interleaved = 0, // interleaved slice groups
	h264_slice_group_map_type_dispersed, // dispersed slice group mapping
	h264_slice_group_map_type_foreground, // one or more "foreground" slice groups and a "leftover" slice group
	h264_slice_group_map_type_3,
	h264_slice_group_map_type_4,
	h264_slice_group_map_type_5,
	h264_slice_group_map_type_explicit, // explicit assignment of a slice group to each slice group map unit
};

struct h264_pps_t
{
	uint32_t pic_parameter_set_id; // [0, 255]
	uint32_t seq_parameter_set_id; // [0, 31]
	uint8_t entropy_coding_mode_flag; // 0-Exp-Golomb coded/CAVLC, 1-CABAC
	uint8_t bottom_field_pic_order_in_frame_present_flag; // bool
	uint32_t num_slice_groups_minus1; // Baseline/Extended profile[0-7], otherwise-0 only
	
	//if(num_slice_groups_minus1 > 0) {
	uint32_t slice_group_map_type; // [0, 6], 0-interleaved slice groups, 1-dispersed slice group mapping, ...
		union slice_group_map
		{
			// slice_group_map_type 0
			uint32_t run_length_minus1[H264_SLICE_GROUPS]; // [0, PicSizeInMapUnits - 1]
			
			// slice_group_map_type 2
			struct slice_group_map_dispersed
			{
				uint32_t top_left[H264_SLICE_GROUPS];  // [0, PicSizeInMapUnits - 1]
				uint32_t bottom_right[H264_SLICE_GROUPS];  // [0, PicSizeInMapUnits - 1]
			} dispersed;
			
			// slice_group_map_type 3/4/5
			struct slice_group_map_direction
			{
				uint8_t slice_group_change_direction_flag; // bool
				uint32_t slice_group_change_rate_minus1;  // [0, PicSizeInMapUnits - 1]
			} direction;

			// slice_group_map_type 6
			struct slice_group_map_explicit
			{
				uint32_t pic_size_in_map_units_minus1;  // [0, PicSizeInMapUnits - 1]
				uint32_t *slice_group_id; // [0, num_slice_groups_minus1]
			} groups;
		} group;
	//}

	uint32_t num_ref_idx_l0_default_active_minus1; // [0, 31]
	uint32_t num_ref_idx_l1_default_active_minus1; // [0, 31]
	uint8_t weighted_pred_flag; // bool
	uint32_t weighted_bipred_idc; // [0-2], 0-default weighted prediction, 1-explicit weighted prediction, 2-implicit weighted prediction
	int32_t pic_init_qp_minus26; // [-(26 + QpBdOffsetY), +25]
	int32_t pic_init_qs_minus26; // [-26, +25]
	int32_t chroma_qp_index_offset; // [-12, 12]
	uint8_t deblocking_filter_control_present_flag; // bool
	uint8_t constrained_intra_pred_flag; // bool
	uint8_t redundant_pic_cnt_present_flag; // bool

	//if( more_rbsp_data( ) ) {
		uint8_t transform_8x8_mode_flag; // bool
		uint8_t pic_scaling_matrix_present_flag; // bool
		//if( pic_scaling_matrix_present_flag ) {
			uint8_t pic_scaling_list_present_flag[12]; // bool
			uint8_t UseDefaultScalingMatrix4x4Flag[6]; // bool
			uint8_t UseDefaultScalingMatrix8x8Flag[6]; // bool
			int32_t ScalingList4x4[6][16];
			int32_t ScalingList8x8[6][64];
		//}
		int32_t second_chroma_qp_index_offset;  // [-12, 12]
	//}
};

#endif /* !_h264_pps_h_ */
