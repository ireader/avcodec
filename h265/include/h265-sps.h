#ifndef _h265_sps_h_
#define _h265_sps_h_

#include "h265-vps.h"
#include "h265-vui.h"
#include <stdint.h>

struct h265_sps_t
{
	uint8_t sps_video_parameter_set_id; // u(4)
	uint8_t sps_max_sub_layers_minus1; // u(3) [0, 6]
	uint8_t sps_temporal_id_nesting_flag; // u(1)

	struct h265_profile_tier_level_t profile;
	uint32_t sps_seq_parameter_set_id; // ue
	uint8_t chroma_format_id; // ue
	uint8_t separate_colour_plane_flag; // u(1)
	uint32_t pic_width_in_luma_samples; // ue
	uint32_t pic_height_in_luma_samples; // ue
	uint8_t conformance_window_flag; // u(1)
	uint32_t conf_win_left_offset; // ue
	uint32_t conf_win_right_offset; // ue
	uint32_t conf_win_top_offet; // ue
	uint32_t conf_win_bottom_offset; // ue

	uint8_t bit_depth_luma_minus8; // ue
	uint8_t bit_depth_chroma_minus8; // ue
	uint8_t log2_max_pic_order_cnt_lsb_minus4; // ue
	uint8_t sps_sub_layer_ordering_info_present_flag; // u(1)
	uint32_t sps_max_dec_pic_buffering_minus1[7]; // ue [0, MaxDpbSize - 1]
	uint32_t sps_max_num_reorder_pics[7]; // ue [0, SpsMaxDecPicBufferingMinus1[i] ]
	uint32_t sps_max_latency_increase_plus1[7]; // ue [0, 2^32- 2]

	uint8_t log2_min_luma_coding_block_size_minus3; // ue
	uint8_t log2_diff_max_min_luma_coding_block_size; // ue
	uint8_t log2_min_luma_transform_block_size_minus2; // ue
	uint8_t log2_diff_max_min_luma_transform_block_size; // ue
	uint32_t max_transform_hierarchy_depth_inter; // ue [0, CtbLog2SizeY - MinTbLog2SizeY]
	uint32_t max_transform_hierarchy_depth_intra; // ue [0, CtbLog2SizeY - MinTbLog2SizeY]
	uint8_t scaling_list_enabled_flag; // u(1)
	uint8_t sps_scaling_list_data_present_flag; // u(1)
	uint8_t scaling_list_pred_mode_flag[4][6]; // u(1)
	uint32_t scaling_list_pred_matrix_id_delta[4][6]; // ue
	int32_t scaling_list_dc_coef_minus8[2][6]; // se
	int32_t scaling_list_delta_coef[4][6][64]; // se
	int32_t ScalingList[4][6][64]; // nextCoef

	uint8_t amp_enabled_flag; // u(1)
	uint8_t sample_adaptive_offset_enabled_flag; // u(1)

	uint8_t pcm_enabled_flag; // u(1)
	uint8_t pcm_sample_bit_depth_luma_minus1; // u(4)
	uint8_t pcm_sample_bit_depth_chroma_minus1; // u(4)
	uint8_t log2_min_pcm_luma_coding_block_size_minus3; // ue
	uint8_t log2_diff_max_min_pcm_luma_coding_block_size; // ue
	uint8_t pcm_loop_filter_disabled_flag; // u(1)

	uint8_t num_short_term_ref_pic_sets; // ue [0, 64]
	uint8_t inter_ref_pic_set_prediction_flag[64]; // u(1)
	uint32_t delta_idx_minus1[64]; // ue
	uint8_t delta_rps_sign[64]; // u(1)
	uint32_t abs_delta_rps_minus1[64]; // ue
	uint8_t used_by_curr_pic_flag[64][128]; // u(1)
	uint8_t use_delta_flag[64][128]; // u(1)
	uint8_t num_negative_pics[64]; // ue [0, SpsMaxDecPicBufferingMinus1[ sps_max_sub_layers_minus1 ] ]
	uint8_t num_positive_pics[64]; // ue [0, SpsMaxDecPicBufferingMinus1[ sps_max_sub_layers_minus1 ] - NumNegativePics ]
	uint32_t delta_poc_s0_minus1[64][64]; // ue
	uint8_t used_by_curr_pic_s0_flag[64][64]; // u(1)
	uint32_t delta_poc_s1_minus1[64][32]; // ue
	uint8_t used_by_curr_pic_s1_flag[64][32]; // u(1)
	uint8_t long_term_ref_pics_present_flag; // u(1)
	uint8_t num_long_term_ref_pics_sps; // ue [0, 32]
	uint32_t lt_ref_pic_poc_lsb_sps[33]; // u(v)
	uint8_t used_by_curr_pic_lt_sps_flag[33]; // u(1)

	uint8_t sps_temporal_mvp_enabled_flag; // u(1)
	uint8_t strong_intra_smoothing_enabled_flag; // u(1)
	uint8_t vui_parameters_present_flag; // u(1)
	struct h265_vui_t vui;
};

int h265_sps_parse(const void* h265, uint32_t bytes, struct h265_sps_t* sps);

int h265_codec_rect(const struct h265_sps_t* sps, int* x, int *y, int *w, int *h);
int h265_display_rect(const struct h265_sps_t* sps, int* x, int *y, int *w, int *h);

#endif /* !_h265_sps_h_ */
