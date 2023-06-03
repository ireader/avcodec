#include "h265-sps.h"
#include "h265-parser.h"
#include "h265-internal.h"
#include <assert.h>

int h265_vui(bitstream_t* stream, struct h265_vui_t* vui, int sps_max_sub_layers_minus1);

static void h265_scaling_list_data(bitstream_t* stream, struct h265_sps_t* sps);
static void h265_st_ref_pic_set(bitstream_t* stream, struct h265_sps_t* sps, int stRpsIdx);

int h265_sps_parse(const void* h265, uint32_t bytes, struct h265_sps_t* sps)
{
	bitstream_t stream;
	struct h265_nal_t nal;
	bitstream_init(&stream, (const unsigned char*)h265, bytes);

	h265_nal(&stream, &nal);
	if (H265_NAL_SPS != nal.nal_unit_type)
		return -1; // invalid NALU

	h265_sps(&stream, sps);
	return 0;
}

int h265_sps(bitstream_t* stream, struct h265_sps_t* sps)
{
	int i;
	sps->sps_video_parameter_set_id = (uint8_t)bitstream_read_bits(stream, 4);
	sps->sps_max_sub_layers_minus1 = (uint8_t)bitstream_read_bits(stream, 3);
	sps->sps_temporal_id_nesting_flag = (uint8_t)bitstream_read_bit(stream);
	h265_profile_tier_level(stream, &sps->profile, 1, sps->sps_max_sub_layers_minus1);
	sps->sps_seq_parameter_set_id = bitstream_read_ue(stream);
	sps->chroma_format_id = (uint8_t)bitstream_read_ue(stream);
	if(3 == sps->chroma_format_id)
		sps->separate_colour_plane_flag = (uint8_t)bitstream_read_bit(stream);

	sps->pic_width_in_luma_samples = bitstream_read_ue(stream);
	sps->pic_height_in_luma_samples = bitstream_read_ue(stream);
	sps->conformance_window_flag = (uint8_t)bitstream_read_bit(stream);
	if(sps->conformance_window_flag)
	{
		sps->conf_win_left_offset = bitstream_read_ue(stream);
		sps->conf_win_right_offset = bitstream_read_ue(stream);
		sps->conf_win_top_offet = bitstream_read_ue(stream);
		sps->conf_win_bottom_offset = bitstream_read_ue(stream);
	}

	sps->bit_depth_luma_minus8 = (uint8_t)bitstream_read_ue(stream);
	sps->bit_depth_chroma_minus8 = (uint8_t)bitstream_read_ue(stream);
	sps->log2_max_pic_order_cnt_lsb_minus4 = bitstream_read_ue(stream);
	sps->sps_sub_layer_ordering_info_present_flag = (uint8_t)bitstream_read_bit(stream);
	for (i = (sps->sps_sub_layer_ordering_info_present_flag ? 0 : sps->sps_max_sub_layers_minus1); i <= sps->sps_max_sub_layers_minus1 && i < sizeof_array(sps->sps_max_dec_pic_buffering_minus1); i++)
	{
		assert(i < sizeof(sps->sps_max_dec_pic_buffering_minus1) / sizeof(sps->sps_max_dec_pic_buffering_minus1[0]));
		sps->sps_max_dec_pic_buffering_minus1[i] = bitstream_read_ue(stream);
		sps->sps_max_num_reorder_pics[i] = bitstream_read_ue(stream);
		sps->sps_max_latency_increase_plus1[i] = bitstream_read_ue(stream);
	}

	sps->log2_min_luma_coding_block_size_minus3 = bitstream_read_ue(stream);
	sps->log2_diff_max_min_luma_coding_block_size = bitstream_read_ue(stream);
	sps->log2_min_luma_transform_block_size_minus2 = bitstream_read_ue(stream);
	sps->log2_diff_max_min_luma_transform_block_size = bitstream_read_ue(stream);
	sps->max_transform_hierarchy_depth_inter = bitstream_read_ue(stream);
	sps->max_transform_hierarchy_depth_intra = bitstream_read_ue(stream);
	sps->scaling_list_enabled_flag = (uint8_t)bitstream_read_bit(stream);
	if(sps->scaling_list_enabled_flag) 
	{
		sps->sps_scaling_list_data_present_flag = (uint8_t)bitstream_read_bit(stream);
		if (sps->sps_scaling_list_data_present_flag)
			h265_scaling_list_data(stream, sps);
	}

	sps->amp_enabled_flag = (uint8_t)bitstream_read_bit(stream);
	sps->sample_adaptive_offset_enabled_flag = (uint8_t)bitstream_read_bit(stream);
	sps->pcm_enabled_flag = (uint8_t)bitstream_read_bit(stream);
	if (sps->pcm_enabled_flag)
	{
		sps->pcm_sample_bit_depth_luma_minus1 = (uint8_t)bitstream_read_bits(stream, 4);
		sps->pcm_sample_bit_depth_chroma_minus1 = (uint8_t)bitstream_read_bits(stream, 4);
		sps->log2_min_pcm_luma_coding_block_size_minus3 = bitstream_read_ue(stream);
		sps->log2_diff_max_min_pcm_luma_coding_block_size = bitstream_read_ue(stream);
		sps->pcm_loop_filter_disabled_flag = (uint8_t)bitstream_read_bit(stream);
	}

	sps->num_short_term_ref_pic_sets = (uint8_t)bitstream_read_ue(stream);
	for (i = 0; i < sps->num_short_term_ref_pic_sets && i < sizeof_array(sps->inter_ref_pic_set_prediction_flag); i++)
		h265_st_ref_pic_set(stream, sps, i);

	sps->long_term_ref_pics_present_flag = (uint8_t)bitstream_read_bit(stream);
	if (sps->long_term_ref_pics_present_flag)
	{
		sps->num_long_term_ref_pics_sps = (uint8_t)bitstream_read_ue(stream);
		for (i = 0; i < sps->num_long_term_ref_pics_sps && i < sizeof_array(sps->lt_ref_pic_poc_lsb_sps); i++)
		{
			sps->lt_ref_pic_poc_lsb_sps[i] = bitstream_read_bits(stream, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
			sps->used_by_curr_pic_lt_sps_flag[i] = (uint8_t)bitstream_read_bit(stream);
		}
	}

	sps->sps_temporal_mvp_enabled_flag = (uint8_t)bitstream_read_bit(stream);
	sps->strong_intra_smoothing_enabled_flag = (uint8_t)bitstream_read_bit(stream);
	sps->vui_parameters_present_flag = (uint8_t)bitstream_read_bit(stream);
	if (sps->vui_parameters_present_flag)
		h265_vui(stream, &sps->vui, sps->sps_max_sub_layers_minus1);
	
	// TODO: parse more
	return 0;
}

static void h265_scaling_list_data(bitstream_t* stream, struct h265_sps_t* sps)
{
	int i, sizeId, matrixId;
	int nextCoef, coefNum;
	for (sizeId = 0; sizeId < 4; sizeId++)
	{
		for (matrixId = 0; matrixId < 6; matrixId += (sizeId == 3 ? 3 : 1))
		{
			sps->scaling_list_pred_mode_flag[sizeId][matrixId] = (uint8_t)bitstream_read_bit(stream);
			if (!sps->scaling_list_pred_mode_flag[sizeId][matrixId])
			{
				sps->scaling_list_pred_matrix_id_delta[sizeId][matrixId] = bitstream_read_ue(stream);
			}
			else
			{
				nextCoef = 8;
				coefNum = 1 << (4 + (sizeId << 1));
				coefNum = coefNum > 64 ? 64 : coefNum;

				if (sizeId > 1)
				{
					sps->scaling_list_dc_coef_minus8[sizeId - 2][matrixId] = bitstream_read_se(stream);
					nextCoef = sps->scaling_list_dc_coef_minus8[sizeId - 2][matrixId] + 8;
				}

				for (i = 0; i < coefNum; i++)
				{
					sps->scaling_list_delta_coef[sizeId][matrixId][i] = bitstream_read_se(stream);
					nextCoef = (nextCoef + sps->scaling_list_delta_coef[sizeId][matrixId][i] + 256) % 256;
					sps->ScalingList[sizeId][matrixId][i] = nextCoef;
				}
			}
		}
	}
}

static void h265_st_ref_pic_set(bitstream_t* stream, struct h265_sps_t* sps, int stRpsIdx)
{
	int i, j, NumDeltaPocs;

	assert(stRpsIdx < sizeof(sps->inter_ref_pic_set_prediction_flag) / sizeof(sps->inter_ref_pic_set_prediction_flag[0]));
	sps->inter_ref_pic_set_prediction_flag[stRpsIdx] = stRpsIdx ? (uint8_t)bitstream_read_bit(stream) : 0;

	if (sps->inter_ref_pic_set_prediction_flag[stRpsIdx]) {
		if (stRpsIdx == sps->num_short_term_ref_pic_sets)
			sps->delta_idx_minus1[stRpsIdx] = bitstream_read_ue(stream);

		sps->delta_rps_sign[stRpsIdx] = (uint8_t)bitstream_read_bit(stream);
		sps->abs_delta_rps_minus1[stRpsIdx] = bitstream_read_ue(stream);

		// NumNegativePics[ stRpsIdx ] = num_negative_pics
		// NumPositivePics[ stRpsIdx ] = num_positive_pics
		// NumDeltaPocs[ stRpsIdx ] = NumNegativePics[ stRpsIdx ] + NumPositivePics[ stRpsIdx ]   //(7-71)
		NumDeltaPocs = sps->num_negative_pics[stRpsIdx] + sps->num_positive_pics[stRpsIdx];
		for (j = 0; j < NumDeltaPocs && j < sizeof_array(sps->used_by_curr_pic_flag[0]); j++)
		{
			assert(j < sizeof(sps->used_by_curr_pic_flag[stRpsIdx]) / sizeof(sps->used_by_curr_pic_flag[stRpsIdx][0]));
			sps->used_by_curr_pic_flag[stRpsIdx][j] = (uint8_t)bitstream_read_bit(stream);
			if (sps->used_by_curr_pic_flag[stRpsIdx][j])
				sps->use_delta_flag[stRpsIdx][j] = (uint8_t)bitstream_read_bit(stream);
		}
	}
	else
	{
		sps->num_negative_pics[stRpsIdx] = bitstream_read_ue(stream);
		sps->num_positive_pics[stRpsIdx] = bitstream_read_ue(stream);
		for (i = 0; i < sps->num_negative_pics[stRpsIdx] && i < sizeof_array(sps->delta_poc_s0_minus1[0]); i++)
		{
			assert(i < sizeof(sps->delta_poc_s0_minus1[stRpsIdx]) / sizeof(sps->delta_poc_s0_minus1[stRpsIdx][0]));
			sps->delta_poc_s0_minus1[stRpsIdx][i] = bitstream_read_ue(stream);
			sps->used_by_curr_pic_s0_flag[stRpsIdx][i] = (uint8_t)bitstream_read_bit(stream);
		}

		for (i = 0; i < sps->num_positive_pics[stRpsIdx] && i < sizeof_array(sps->delta_poc_s1_minus1[0]); i++)
		{
			assert(i < sizeof(sps->delta_poc_s1_minus1[stRpsIdx]) / sizeof(sps->delta_poc_s1_minus1[stRpsIdx][0]));
			sps->delta_poc_s1_minus1[stRpsIdx][i] = bitstream_read_ue(stream);
			sps->used_by_curr_pic_s1_flag[stRpsIdx][i] = (uint8_t)bitstream_read_bit(stream);
		}
	}
}

int h265_codec_rect(const struct h265_sps_t* sps, int *x, int *y, int *w, int* h)
{
	*x = 0;
	*y = 0;
	*w = sps->pic_width_in_luma_samples;
	*h = sps->pic_height_in_luma_samples;
	return 0;
}

int h265_display_rect(const struct h265_sps_t* sps, int *x, int *y, int *w, int* h)
{
	// ITU H.265 Table 6-1 - SubWidthC, and SubHeightC values derived from chroma_format_idc and separate_colour_plane_flag
	const int SubWidthC[] = { 1 /*4:0:0*/, 2 /*4:2:0*/, 2 /*4:2:2*/, 1 /*4:4:4*/ };
	const int SubHeightC[] = { 1 /*4:0:0*/, 2 /*4:2:0*/, 1 /*4:2:2*/, 1 /*4:4:4*/ };

	int ux, uy;
	ux = SubWidthC[sps->chroma_format_id % 4];
	uy = SubHeightC[sps->chroma_format_id % 4];
	*w = sps->pic_width_in_luma_samples - ux * (sps->conf_win_right_offset - sps->conf_win_left_offset);
	*h = sps->pic_height_in_luma_samples - uy * (sps->conf_win_bottom_offset - sps->conf_win_top_offet);
	return 0;
}

#if defined(DEBUG) || defined(_DEBUG)
void h265_sps_test(void)
{
	const uint8_t data[] = {
		0x42, 0x01, 0x02, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5d, 0x00, 0x00, 0xa0, 0x02, 0x80, 0x80, 0x2d, 0x1f, 0xe5, 0x97, 0x32, 0xc9, 0x26, 0x45, 0x25, 0x55, 0x6f, 0xb1, 0x28, 0x89, 0xe4,
	};

	/*
[trace_headers @ 0000020B6571EF50] Sequence Parameter Set
[trace_headers @ 0000020B6571EF50] 0           forbidden_zero_bit                                          0 = 0
[trace_headers @ 0000020B6571EF50] 1           nal_unit_type                                          100001 = 33
[trace_headers @ 0000020B6571EF50] 7           nuh_layer_id                                           000000 = 0
[trace_headers @ 0000020B6571EF50] 13          nuh_temporal_id_plus1                                     001 = 1
[trace_headers @ 0000020B6571EF50] 16          sps_video_parameter_set_id                               0000 = 0
[trace_headers @ 0000020B6571EF50] 20          sps_max_sub_layers_minus1                                 000 = 0
[trace_headers @ 0000020B6571EF50] 23          sps_temporal_id_nesting_flag                                1 = 1
[trace_headers @ 0000020B6571EF50] 24          general_profile_space                                      00 = 0
[trace_headers @ 0000020B6571EF50] 26          general_tier_flag                                           1 = 1
[trace_headers @ 0000020B6571EF50] 27          general_profile_idc                                     00001 = 1
[trace_headers @ 0000020B6571EF50] 32          general_profile_compatibility_flag[0]                       0 = 0
[trace_headers @ 0000020B6571EF50] 33          general_profile_compatibility_flag[1]                       1 = 1
[trace_headers @ 0000020B6571EF50] 34          general_profile_compatibility_flag[2]                       0 = 0
[trace_headers @ 0000020B6571EF50] 35          general_profile_compatibility_flag[3]                       0 = 0
[trace_headers @ 0000020B6571EF50] 36          general_profile_compatibility_flag[4]                       0 = 0
[trace_headers @ 0000020B6571EF50] 37          general_profile_compatibility_flag[5]                       0 = 0
[trace_headers @ 0000020B6571EF50] 38          general_profile_compatibility_flag[6]                       0 = 0
[trace_headers @ 0000020B6571EF50] 39          general_profile_compatibility_flag[7]                       0 = 0
[trace_headers @ 0000020B6571EF50] 40          general_profile_compatibility_flag[8]                       0 = 0
[trace_headers @ 0000020B6571EF50] 41          general_profile_compatibility_flag[9]                       0 = 0
[trace_headers @ 0000020B6571EF50] 42          general_profile_compatibility_flag[10]                      0 = 0
[trace_headers @ 0000020B6571EF50] 43          general_profile_compatibility_flag[11]                      0 = 0
[trace_headers @ 0000020B6571EF50] 44          general_profile_compatibility_flag[12]                      0 = 0
[trace_headers @ 0000020B6571EF50] 45          general_profile_compatibility_flag[13]                      0 = 0
[trace_headers @ 0000020B6571EF50] 46          general_profile_compatibility_flag[14]                      0 = 0
[trace_headers @ 0000020B6571EF50] 47          general_profile_compatibility_flag[15]                      0 = 0
[trace_headers @ 0000020B6571EF50] 48          general_profile_compatibility_flag[16]                      0 = 0
[trace_headers @ 0000020B6571EF50] 49          general_profile_compatibility_flag[17]                      0 = 0
[trace_headers @ 0000020B6571EF50] 50          general_profile_compatibility_flag[18]                      0 = 0
[trace_headers @ 0000020B6571EF50] 51          general_profile_compatibility_flag[19]                      0 = 0
[trace_headers @ 0000020B6571EF50] 52          general_profile_compatibility_flag[20]                      0 = 0
[trace_headers @ 0000020B6571EF50] 53          general_profile_compatibility_flag[21]                      0 = 0
[trace_headers @ 0000020B6571EF50] 54          general_profile_compatibility_flag[22]                      0 = 0
[trace_headers @ 0000020B6571EF50] 55          general_profile_compatibility_flag[23]                      0 = 0
[trace_headers @ 0000020B6571EF50] 56          general_profile_compatibility_flag[24]                      0 = 0
[trace_headers @ 0000020B6571EF50] 57          general_profile_compatibility_flag[25]                      0 = 0
[trace_headers @ 0000020B6571EF50] 58          general_profile_compatibility_flag[26]                      0 = 0
[trace_headers @ 0000020B6571EF50] 59          general_profile_compatibility_flag[27]                      0 = 0
[trace_headers @ 0000020B6571EF50] 60          general_profile_compatibility_flag[28]                      0 = 0
[trace_headers @ 0000020B6571EF50] 61          general_profile_compatibility_flag[29]                      0 = 0
[trace_headers @ 0000020B6571EF50] 62          general_profile_compatibility_flag[30]                      0 = 0
[trace_headers @ 0000020B6571EF50] 63          general_profile_compatibility_flag[31]                      0 = 0
[trace_headers @ 0000020B6571EF50] 64          general_progressive_source_flag                             0 = 0
[trace_headers @ 0000020B6571EF50] 65          general_interlaced_source_flag                              0 = 0
[trace_headers @ 0000020B6571EF50] 66          general_non_packed_constraint_flag                          0 = 0
[trace_headers @ 0000020B6571EF50] 67          general_frame_only_constraint_flag                          0 = 0
[trace_headers @ 0000020B6571EF50] 68          general_reserved_zero_43bits         000000000000000000000000 = 0
[trace_headers @ 0000020B6571EF50] 92          general_reserved_zero_43bits              0000000000000000000 = 0
[trace_headers @ 0000020B6571EF50] 111         general_inbld_flag                                          0 = 0
[trace_headers @ 0000020B6571EF50] 112         general_level_idc                                    10011001 = 153
[trace_headers @ 0000020B6571EF50] 120         sps_seq_parameter_set_id                                    1 = 0
[trace_headers @ 0000020B6571EF50] 121         chroma_format_idc                                         010 = 1
[trace_headers @ 0000020B6571EF50] 124         pic_width_in_luma_samples             00000000000111100000001 = 3840
[trace_headers @ 0000020B6571EF50] 147         pic_height_in_luma_samples            00000000000100010000001 = 2176
[trace_headers @ 0000020B6571EF50] 170         conformance_window_flag                                     1 = 1
[trace_headers @ 0000020B6571EF50] 171         conf_win_left_offset                                        1 = 0
[trace_headers @ 0000020B6571EF50] 172         conf_win_right_offset                                       1 = 0
[trace_headers @ 0000020B6571EF50] 173         conf_win_top_offset                                         1 = 0
[trace_headers @ 0000020B6571EF50] 174         conf_win_bottom_offset                                0001001 = 8
[trace_headers @ 0000020B6571EF50] 181         bit_depth_luma_minus8                                       1 = 0
[trace_headers @ 0000020B6571EF50] 182         bit_depth_chroma_minus8                                     1 = 0
[trace_headers @ 0000020B6571EF50] 183         log2_max_pic_order_cnt_lsb_minus4                       00101 = 4
[trace_headers @ 0000020B6571EF50] 188         sps_sub_layer_ordering_info_present_flag                    1 = 1
[trace_headers @ 0000020B6571EF50] 189         sps_max_dec_pic_buffering_minus1[0]                     00101 = 4
[trace_headers @ 0000020B6571EF50] 194         sps_max_num_reorder_pics[0]                               010 = 1
[trace_headers @ 0000020B6571EF50] 197         sps_max_latency_increase_plus1[0]                           1 = 0
[trace_headers @ 0000020B6571EF50] 198         log2_min_luma_coding_block_size_minus3                    010 = 1
[trace_headers @ 0000020B6571EF50] 201         log2_diff_max_min_luma_coding_block_size                  010 = 1
[trace_headers @ 0000020B6571EF50] 204         log2_min_luma_transform_block_size_minus2                   1 = 0
[trace_headers @ 0000020B6571EF50] 205         log2_diff_max_min_luma_transform_block_size             00100 = 3
[trace_headers @ 0000020B6571EF50] 210         max_transform_hierarchy_depth_inter                     00100 = 3
[trace_headers @ 0000020B6571EF50] 215         max_transform_hierarchy_depth_intra                     00100 = 3
[trace_headers @ 0000020B6571EF50] 220         scaling_list_enabled_flag                                   0 = 0
[trace_headers @ 0000020B6571EF50] 221         amp_enabled_flag                                            1 = 1
[trace_headers @ 0000020B6571EF50] 222         sample_adaptive_offset_enabled_flag                         1 = 1
[trace_headers @ 0000020B6571EF50] 223         pcm_enabled_flag                                            0 = 0
[trace_headers @ 0000020B6571EF50] 224         num_short_term_ref_pic_sets                               010 = 1
[trace_headers @ 0000020B6571EF50] 227         num_negative_pics                                       00101 = 4
[trace_headers @ 0000020B6571EF50] 232         num_positive_pics                                           1 = 0
[trace_headers @ 0000020B6571EF50] 233         delta_poc_s0_minus1[0]                                      1 = 0
[trace_headers @ 0000020B6571EF50] 234         used_by_curr_pic_s0_flag[0]                                 1 = 1
[trace_headers @ 0000020B6571EF50] 235         delta_poc_s0_minus1[1]                                      1 = 0
[trace_headers @ 0000020B6571EF50] 236         used_by_curr_pic_s0_flag[1]                                 1 = 1
[trace_headers @ 0000020B6571EF50] 237         delta_poc_s0_minus1[2]                                      1 = 0
[trace_headers @ 0000020B6571EF50] 238         used_by_curr_pic_s0_flag[2]                                 0 = 0
[trace_headers @ 0000020B6571EF50] 239         delta_poc_s0_minus1[3]                                      1 = 0
[trace_headers @ 0000020B6571EF50] 240         used_by_curr_pic_s0_flag[3]                                 0 = 0
[trace_headers @ 0000020B6571EF50] 241         long_term_ref_pics_present_flag                             0 = 0
[trace_headers @ 0000020B6571EF50] 242         sps_temporal_mvp_enabled_flag                               0 = 0
[trace_headers @ 0000020B6571EF50] 243         strong_intra_smoothing_enabled_flag                         0 = 0
[trace_headers @ 0000020B6571EF50] 244         vui_parameters_present_flag                                 1 = 1
[trace_headers @ 0000020B6571EF50] 245         aspect_ratio_info_present_flag                              1 = 1
[trace_headers @ 0000020B6571EF50] 246         aspect_ratio_idc                                     00000001 = 1
[trace_headers @ 0000020B6571EF50] 254         overscan_info_present_flag                                  0 = 0
[trace_headers @ 0000020B6571EF50] 255         video_signal_type_present_flag                              1 = 1
[trace_headers @ 0000020B6571EF50] 256         video_format                                              101 = 5
[trace_headers @ 0000020B6571EF50] 259         video_full_range_flag                                       0 = 0
[trace_headers @ 0000020B6571EF50] 260         colour_description_present_flag                             1 = 1
[trace_headers @ 0000020B6571EF50] 261         colour_primaries                                     00001001 = 9
[trace_headers @ 0000020B6571EF50] 269         transfer_characteristics                             00010010 = 18
[trace_headers @ 0000020B6571EF50] 277         matrix_coefficients                                  00001001 = 9
[trace_headers @ 0000020B6571EF50] 285         chroma_loc_info_present_flag                                0 = 0
[trace_headers @ 0000020B6571EF50] 286         neutral_chroma_indication_flag                              0 = 0
[trace_headers @ 0000020B6571EF50] 287         field_seq_flag                                              0 = 0
[trace_headers @ 0000020B6571EF50] 288         frame_field_info_present_flag                               0 = 0
[trace_headers @ 0000020B6571EF50] 289         default_display_window_flag                                 0 = 0
[trace_headers @ 0000020B6571EF50] 290         vui_timing_info_present_flag                                1 = 1
[trace_headers @ 0000020B6571EF50] 291         vui_num_units_in_tick        00000000000000000000000000000001 = 1
[trace_headers @ 0000020B6571EF50] 323         vui_time_scale               00000000000000000000000000110010 = 50
[trace_headers @ 0000020B6571EF50] 355         vui_poc_proportional_to_timing_flag                         0 = 0
[trace_headers @ 0000020B6571EF50] 356         vui_hrd_parameters_present_flag                             1 = 1
[trace_headers @ 0000020B6571EF50] 357         nal_hrd_parameters_present_flag                             1 = 1
[trace_headers @ 0000020B6571EF50] 358         vcl_hrd_parameters_present_flag                             0 = 0
[trace_headers @ 0000020B6571EF50] 359         sub_pic_hrd_params_present_flag                             0 = 0
[trace_headers @ 0000020B6571EF50] 360         bit_rate_scale                                           0000 = 0
[trace_headers @ 0000020B6571EF50] 364         cpb_size_scale                                           0000 = 0
[trace_headers @ 0000020B6571EF50] 368         initial_cpb_removal_delay_length_minus1                 10111 = 23
[trace_headers @ 0000020B6571EF50] 373         au_cpb_removal_delay_length_minus1                      01111 = 15
[trace_headers @ 0000020B6571EF50] 378         dpb_output_delay_length_minus1                          00101 = 5
[trace_headers @ 0000020B6571EF50] 383         fixed_pic_rate_general_flag[0]                              0 = 0
[trace_headers @ 0000020B6571EF50] 384         fixed_pic_rate_within_cvs_flag[0]                           0 = 0
[trace_headers @ 0000020B6571EF50] 385         low_delay_hrd_flag[0]                                       0 = 0
[trace_headers @ 0000020B6571EF50] 386         cpb_cnt_minus1[0]                                           1 = 0
[trace_headers @ 0000020B6571EF50] 387         bit_rate_value_minus1[0]  0000000000000000001111010000100100000 = 499999
[trace_headers @ 0000020B6571EF50] 424         cpb_size_value_minus1[0]  0000000000000000000001011111010111100001000 = 3124999
[trace_headers @ 0000020B6571EF50] 467         cbr_flag[0]                                                 0 = 0
[trace_headers @ 0000020B6571EF50] 468         bitstream_restriction_flag                                  0 = 0
[trace_headers @ 0000020B6571EF50] 469         sps_extension_present_flag                                  0 = 0
[trace_headers @ 0000020B6571EF50] 470         rbsp_stop_one_bit                                           1 = 1
[trace_headers @ 0000020B6571EF50] 471         rbsp_alignment_zero_bit                                     0 = 0
	*/
	const uint8_t hdr[] = {
		0x42, 0x01, 0x01, 0x21, 0x40, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x99, 0xa0, 0x01, 0xe0, 0x20, 0x02, 0x20, 0x7c, 0x4e, 0x59, 0x55, 0x29, 0x08, 0x46, 0x45, 0xfd, 0x0c, 0x05, 0xa8, 0x48, 0x90, 0x48, 0x20, 0x00, 0x00, 0x03, 0x00, 0x20, 0x00, 0x00, 0x06, 0x4c, 0x00, 0xbb, 0xca, 0x20, 0x00, 0x07, 0xa1, 0x20, 0x00, 0x00, 0x05, 0xf5, 0xe1, 0x02, 
	};

	bitstream_t stream;
	struct h265_nal_t nal;
	struct h265_sps_t sps;
	bitstream_init(&stream, (const unsigned char*)hdr, sizeof(hdr));

	h265_nal(&stream, &nal);
	assert(H265_NAL_SPS == nal.nal_unit_type);

	assert(0 == h265_sps(&stream, &sps));
}
#endif
