#include "h264-slice-header.h"
#include "h264-internal.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int h264_slice_header(bitstream_t* stream, struct h264_context_t* h264, struct h264_nal_t* nal, struct h264_slice_header_t* header)
{
	struct h264_pps_t* pps;
	struct h264_sps_t* sps;

	memset(header, 0, sizeof(struct h264_slice_header_t));
	header->first_mb_in_slice = bitstream_read_ue(stream);
	header->slice_type = bitstream_read_ue(stream);
	header->pic_parameter_set_id = bitstream_read_ue(stream);

	if (header->pic_parameter_set_id >= sizeof(h264->pps) / sizeof(h264->pps[0]))
		return -1; // invalid pic_parameter_set_id
	pps = h264->pps + header->pic_parameter_set_id;
	if (pps->seq_parameter_set_id >= sizeof(h264->sps) / sizeof(h264->sps[0]))
		return -1; // invalid pps
	assert(pps->seq_parameter_set_id < sizeof(h264->sps) / sizeof(h264->sps[0]));
	sps = h264->sps + pps->seq_parameter_set_id;

	if(1 == sps->chroma.separate_colour_plane_flag)
		header->colour_plane_id = bitstream_read_bits(stream, 2);
	
	header->frame_num = bitstream_read_bits(stream, sps->log2_max_frame_num_minus4 + 4);
	
	if (!sps->frame_mbs_only_flag)
	{
		header->field_pic_flag = (uint8_t)bitstream_read_bit(stream);
		if (header->field_pic_flag)
			header->bottom_field_flag = (uint8_t)bitstream_read_bit(stream);
	}

	if (H264_NAL_IDR == nal->nal_unit_type)
		header->idr_pic_id = bitstream_read_ue(stream);

	if (0 == sps->pic_order_cnt_type)
	{
		header->pic_order_cnt_lsb = bitstream_read_bits(stream, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
		if(pps->bottom_field_pic_order_in_frame_present_flag && !header->field_pic_flag)
			header->delta_pic_order_cnt_bottom = bitstream_read_se(stream);
	}

	if (1 == sps->pic_order_cnt_type && !sps->delta_pic_order_always_zero_flag)
	{
		header->delta_pic_order_cnt[0] = bitstream_read_se(stream);
		if(pps->bottom_field_pic_order_in_frame_present_flag && !header->field_pic_flag)
			header->delta_pic_order_cnt[1] = bitstream_read_se(stream);
	}

	if(pps->redundant_pic_cnt_present_flag)
		header->redundant_pic_cnt = bitstream_read_ue(stream);

	if (H264_SLICE_B == header->slice_type)
		header->direct_spatial_mv_pred_flag = (uint8_t)bitstream_read_bit(stream);

	if (H264_SLICE_P == header->slice_type || H264_SLICE_SP == header->slice_type || H264_SLICE_B == header->slice_type)
	{
		header->num_ref_idx_active_override_flag = (uint8_t)bitstream_read_bit(stream);
		if (header->num_ref_idx_active_override_flag)
		{
			header->num_ref_idx_l0_active_minus1 = bitstream_read_ue(stream);
			if (H264_SLICE_B == header->slice_type)
				header->num_ref_idx_l1_active_minus1 = bitstream_read_ue(stream);
		}
	}

	//if (H264_NAL_XVC == nal->nal_unit_type || H264_NAL_3D == nal->nal_unit_type)
	//	ref_pic_list_mvc_modification(); /* specified in Annex H */
	//else
	//	ref_pic_list_modification();

	//if ((pps->weighted_pred_flag && (H264_SLICE_P == header->slice_type || H264_SLICE_SP == header->slice_type))
	//	|| (1 == pps->weighted_bipred_idc && H264_SLICE_B == header->slice_type))
	//{
	//	pred_weight_table();
	//}

	//if (0 != nal->nal_ref_idc)
	//	dec_ref_pic_marking();

	//if (pps->entropy_coding_mode_flag && H264_SLICE_I != header->slice_type && H264_SLICE_SI != header->slice_type)
	//	header->cabac_init_idc = bitstream_read_ue(stream);

	//header->slice_qp_delta = bitstream_read_se(stream);
	//if (H264_SLICE_SP == header->slice_type || H264_SLICE_SI == header->slice_type)
	//{
	//	if (H264_SLICE_SP == header->slice_type)
	//		header->sp_for_switch_flag = bitstream_read_bit(stream);
	//	header->slice_qs_delta = bitstream_read_se(stream);
	//}

	//if (pps->deblocking_filter_control_present_flag)
	//{
	//	header->disable_deblocking_filter_idc = bitstream_read_ue(stream);
	//	if (1 != header->disable_deblocking_filter_idc)
	//	{
	//		header->slice_alpha_c0_offset_div2 = bitstream_read_se(stream);
	//		header->slice_beta_offset_div2 = bitstream_read_se(stream);
	//	}
	//}

	//if (pps->num_slice_groups_minus1 > 0 && pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
	//	header->slice_group_change_cycle = bitstream_read_bits(stream, Ceil(Log2(PicSizeInMapUnits / SliceGroupChangeRate + 1)));
	
	if (h264_more_rbsp_data(stream))
	{
	}

	return 0;
}
