#include "h264-parser.h"
#include "h264-internal.h"
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <errno.h>

int h264_parser(const void* payload, size_t bytes)
{
	int r = -1;
	bitstream_t stream;
	struct h264_nal_t nal;
	struct h264_sps_t sps;
	struct h264_pps_t pps;
	bitstream_init(&stream, (const unsigned char*)payload, bytes);

	if (0 != h264_nal(&stream, &nal))
	{
		printf("%s: h264_nalu(%02X) error.\n", __FUNCTION__, (int)*(unsigned char*)payload);
		return -1;
	}

	assert(0 == nal.forbidden_zero_bit);
	switch (nal.nal_unit_type)
	{
	case H264_NAL_SPS:
		assert(nal.nal_ref_idc > 0);
		r = h264_sps(&stream, &sps);
		break;

	case H264_NAL_PPS:
		assert(nal.nal_ref_idc > 0);
		//r = h264_pps(&stream, NULL, &pps);
		break;

	default:
		break;
	}
	return r;
}

int h264_rbsp_trailing_bits(bitstream_t* stream)
{
	int rbsp_stop_one_bit;
	int rbsp_alignment_zero_bit;
	size_t bytes, bits, i;

	rbsp_stop_one_bit = bitstream_read_bit(stream);
	assert(1 == rbsp_stop_one_bit);

	bitstream_get_offset(stream, &bytes, &bits);
	for (i = bits; i < 8; i++)
	{
		rbsp_alignment_zero_bit = bitstream_read_bit(stream);
		assert(0 == rbsp_alignment_zero_bit);
	}
	return 0;
}

int h264_more_rbsp_data(bitstream_t* stream)
{
	int rbsp_next_bits;
	size_t bytes, bits, n;

	bitstream_get_offset(stream, &bytes, &bits);
	if (bytes + 1 >= stream->bytes && bits + 1 >= 8)
		return 0; // no more data

	n = bits < 8 ? 8 - bits : 8;
	rbsp_next_bits = bitstream_next_bits(stream, n);
	return rbsp_next_bits != (1 << (n - 1));
}
