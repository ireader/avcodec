#include "h264-internal.h"
#include <assert.h>

int h264_rbsp_trailing_bits(bitstream_t* stream)
{
	int rbsp_stop_one_bit;
	int rbsp_alignment_zero_bit;
	size_t bits;

	rbsp_stop_one_bit = bitstream_read_bit(stream);
	assert(1 == rbsp_stop_one_bit);

	bitstream_get_offset(stream, &bits);
	rbsp_alignment_zero_bit = bitstream_read_bits(stream, 8 - (bits % 8));
	assert(0 == rbsp_alignment_zero_bit);

	return (1 == rbsp_stop_one_bit && 0 == rbsp_alignment_zero_bit) ? 0 : -1;
}

int h264_more_rbsp_data(bitstream_t* stream)
{
    size_t bits, n;
    int rbsp_next_bits;

    bitstream_get_offset(stream, &bits);
    if (bits >= stream->size * 8)
        return 0; // no more data

    n = 8 - (bits % 8);
    rbsp_next_bits = bitstream_next_bits(stream, (int)n);
    return rbsp_next_bits != (1 << (n - 1));
}
