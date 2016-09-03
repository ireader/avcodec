#ifndef _h264_slice_header_h_
#define _h264_slice_header_h_

#include <stdint.h>

#define H264_SLICE_P	0
#define H264_SLICE_B	1
#define H264_SLICE_I	2
#define H264_SLICE_SP	3
#define H264_SLICE_SI	4

struct h264_slice_header_t
{
	uint32_t first_mb_in_slice; // [0, PicSizeInMbs - 1]
	uint32_t slice_type; // [0, 9]
	uint32_t pic_parameter_set_id; // [0, 255]

	uint32_t colour_plane_id; // [0, 2]
	uint32_t frame_num; // [0, log2_max_frame_num_minus4 + 4];
	uint8_t field_pic_flag; // bool
	uint8_t bottom_field_flag; // bool
	uint32_t idr_pic_id; // [0, 65535]
	uint32_t pic_order_cnt_lsb; // [0, MaxPicOrderCntLsb - 1]
	uint32_t delta_pic_order_cnt_bottom;
	uint32_t delta_pic_order_cnt[2];
	uint32_t redundant_pic_cnt; // [0, 127]
	uint8_t direct_spatial_mv_pred_flag; // bool
	uint8_t num_ref_idx_active_override_flag; // bool
	uint32_t num_ref_idx_l0_active_minus1;
	uint32_t num_ref_idx_l1_active_minus1;
	uint32_t cabac_init_idc; // [0, 2]
	int32_t slice_qp_delta;
	uint8_t sp_for_switch_flag; // bool
	int32_t slice_qs_delta;
	uint32_t disable_deblocking_filter_idc;
	int32_t slice_alpha_c0_offset_div2;
	int32_t slice_beta_offset_div2;
	uint32_t slice_group_change_cycle;
};

#endif /* !_h264_slice_header_h_ */
