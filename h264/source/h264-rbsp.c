#include "h264-internal.h"
#include <assert.h>

int h264_rbsp_trailing_bits(bitstream_t* stream)
{
	int rbsp_stop_one_bit;
	int rbsp_alignment_zero_bit;
	size_t bytes, bits, i;

	rbsp_stop_one_bit = bitstream_read_bit(stream);
	assert(1 == rbsp_stop_one_bit);

	bitstream_get_offset(stream, &bytes, &bits);
	for (i = bits; i < 8 && bytes < stream->bytes; i++)
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
	rbsp_next_bits = bitstream_next_bits(stream, (int)n);
	return rbsp_next_bits != (1 << (n - 1));
}
