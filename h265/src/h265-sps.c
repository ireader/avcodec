#include "h265-sps.h"
#include "h265-parser.h"
#include <assert.h>

int h265_sps(bitstream_t* stream, struct h265_sps_t* sps)
{
	sps->sps_video_parameter_set_id = bitstream_read_bits(stream, 4);
	sps->sps_max_sub_layers_minus1 = bitstream_read_bits(stream, 3);
	sps->sps_temporal_id_nesting_flag = bitstream_read_bit(stream);
	h265_profile_tier_level(stream, &sps->profile, 1, sps->sps_max_sub_layers_minus1);
	sps->sps_seq_parameter_set_id = bitstream_read_ue(stream);
	sps->chroma_format_id = bitstream_read_ue(stream);
	if(3 == sps->chroma_format_id)
		sps->separate_colour_plane_flag = bitstream_read_bit(stream);

	sps->pic_width_in_luma_samples = bitstream_read_ue(stream);
	sps->pic_height_in_luma_samples = bitstream_read_ue(stream);
	sps->conformance_window_flag = bitstream_read_bit(stream);
	if(sps->conformance_window_flag)
	{
		sps->conf_win_left_offset = bitstream_read_ue(stream);
		sps->conf_win_right_offset = bitstream_read_ue(stream);
		sps->conf_win_top_offet = bitstream_read_ue(stream);
		sps->conf_win_bottom_offset = bitstream_read_ue(stream);
	}

	sps->bit_depth_luma_minus8 = bitstream_read_ue(stream);
	sps->bit_depth_chroma_minus8 = bitstream_read_ue(stream);
	sps->log2_max_pic_order_cnt_lsb_minus4 = bitstream_read_ue(stream);
	sps->sps_sub_layer_ordering_info_present_flag = bitstream_read_bit(stream);

	// TODO: parse more
	return 0;
}

int h265_display_rect(const struct h265_sps_t* sps, int *x, int *y, int *w, int* h)
{
	*x = 0;
	*y = 0;
	*w = sps->pic_width_in_luma_samples;
	*h = sps->pic_height_in_luma_samples;
	return 0;
}

int h265_codec_rect(const struct h265_sps_t* sps, int *x, int *y, int *w, int* h)
{
	// ITU H.265 Table 6-1 - SubWidthC, and SubHeightC values derived from chroma_format_idc and separate_colour_plane_flag
	const int SubWidthC[] = { 1 /*4:0:0*/, 2 /*4:2:0*/, 2 /*4:2:2*/, 1 /*4:4:4*/ };
	const int SubHeightC[] = { 1 /*4:0:0*/, 2 /*4:2:0*/, 1 /*4:2:2*/, 1 /*4:4:4*/ };

	int ux, uy;
	ux = SubWidthC[sps->chroma_format_id % 4];
	uy = SubHeightC[sps->chroma_format_id % 4];

	*x = ux * sps->conf_win_left_offset;
	*y = uy * sps->conf_win_top_offet;
	*w = sps->pic_width_in_luma_samples - ux * sps->conf_win_left_offset - *x;
	*h = sps->pic_height_in_luma_samples - uy * sps->conf_win_bottom_offset - *y;
	return 0;
}

void h265_sps_test(void)
{
	const uint8_t data[] = {
		0x42, 0x01, 0x02, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5d, 0x00, 0x00, 0xa0, 0x02, 0x80, 0x80, 0x2d, 0x1f, 0xe5, 0x97, 0x32, 0xc9, 0x26, 0x45, 0x25, 0x55, 0x6f, 0xb1, 0x28, 0x89, 0xe4,
	};

	bitstream_t stream;
	struct h265_nal_t nal;
	struct h265_sps_t sps;
	bitstream_init(&stream, (const unsigned char*)data, sizeof(data));

	h265_nal(&stream, &nal);
	assert(H265_NAL_SPS == nal.nal_unit_type);

	assert(0 == h265_sps(&stream, &sps));
}
