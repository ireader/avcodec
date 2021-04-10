#ifndef _h265_sps_h_
#define _h265_sps_h_

#include "h265-vps.h"
#include <stdint.h>

struct h265_sps_t
{
	uint8_t sps_video_parameter_set_id; // u(4)
	uint8_t sps_max_sub_layers_minus1; // u(3)
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
	uint32_t log2_max_pic_order_cnt_lsb_minus4; // ue
	uint8_t sps_sub_layer_ordering_info_present_flag; // u(1)
};

int h265_sps_parse(const void* h265, uint32_t bytes, struct h265_sps_t* sps);

int h265_codec_rect(const struct h265_sps_t* sps, int* x, int *y, int *w, int *h);
int h265_display_rect(const struct h265_sps_t* sps, int* x, int *y, int *w, int *h);

#endif /* !_h265_sps_h_ */
