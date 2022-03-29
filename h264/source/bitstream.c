#include "bitstream.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BIT_NUM 8
//#define BIT_NUM (sizeof(char)*8)

void bitstream_init(bitstream_t* stream, const unsigned char* rbsp, size_t bytes)
{
	memset(stream, 0, sizeof(bitstream_t));
	stream->rbsp = rbsp;
	stream->size = bytes;
	stream->bits = 0;
	stream->error = 0;
}

int bitstream_error(bitstream_t* stream)
{
	return stream->error;
}

static inline void bitstream_move_next_bit(bitstream_t* stream)
{
	size_t offset;
	if (0 != (++stream->bits % BIT_NUM))
		return;

	// check 0x00 0x00 0x03
	offset = stream->bits / BIT_NUM;
	if (offset < stream->size && offset > 1
		&& 0x03 == stream->rbsp[offset]
		&& 0x00 == stream->rbsp[offset - 1]
		&& 0x00 == stream->rbsp[offset - 2])
		stream->bits += BIT_NUM; // skip 0x03
}

int bitstream_get_offset(bitstream_t* stream, size_t* bits)
{
	*bits = stream->bits;
	return 0;
}

int bitstream_set_offset(bitstream_t* stream, size_t bits)
{
	if(bits > stream->size * BIT_NUM)
	{
		stream->error = -1;
		return -1; // throw exception
	}

	stream->bits = bits;
	return 0;
}

int bitstream_next_bit(bitstream_t* stream)
{
	assert(stream && stream->rbsp && stream->size > 0);
	if(stream->bits >= stream->size * BIT_NUM)
	{
		stream->error = -1;
		return 0; // throw exception
	}

	return (stream->rbsp[stream->bits / BIT_NUM] >> (BIT_NUM - 1 - (stream->bits % BIT_NUM))) & 0x01;
}

int bitstream_next_bits(bitstream_t* stream, int bits)
{
	bitstream_t s;
	bitstream_init(&s, stream->rbsp, stream->size);
	s.bits = stream->bits;
	return bitstream_read_bits(&s, bits);
}

int bitstream_read_bit(bitstream_t* stream)
{
	int bit;
	assert(stream && stream->rbsp && stream->size > 0);
	if (stream->bits >= stream->size * BIT_NUM)
	{
		stream->error = -1;
		return 0; // throw exception
	}

	bit = (stream->rbsp[stream->bits / BIT_NUM] >> (BIT_NUM - 1 - (stream->bits % BIT_NUM))) & 0x01;
	bitstream_move_next_bit(stream); // update offset
	assert(0 == bit || 1 == bit);
	return bit;
}

int64_t bitstream_read_bits(bitstream_t* stream, int num)
{
	int i, bit, value;

	assert(stream && num >= 0 && num <= 64);
	value = 0;
	for(i=0, bit = 0; i < num && 0 == stream->error; i++)
	{
		bit = bitstream_read_bit(stream);
		value = (value << 1) | bit;
	}
	return value;
}

int bitstream_read_ue(bitstream_t* stream)
{
	int bit;
	int leadingZeroBits = -1;
	for(bit = 0; !bit && 0 == stream->error; ++leadingZeroBits)
	{
		bit = bitstream_read_bit(stream);
		assert(0 == bit || 1 == bit);
	}

	bit = 0;
	if(leadingZeroBits > 0)
	{
		assert(leadingZeroBits < 32);
		bit = bitstream_read_bits(stream, leadingZeroBits);
	}
	return (1 << leadingZeroBits) -1 + bit;
}

int bitstream_read_se(bitstream_t* stream)
{
	int v = bitstream_read_ue(stream);
	return (0==v%2 ? -1 : 1) * ((v + 1) / 2);
}

int bitstream_read_me(bitstream_t* stream, int chroma_format_idc, int coded_block_pattern)
{
	static const int intra[48] = {0, 16, 1, 2, 4, 8, 32, 3, 5, 10, 12, 15, 47, 7, 11, 13, 14, 6, 9, 31, 35, 37, 42, 44, 33, 34, 36, 40, 39, 43, 45, 46, 17, 18, 20, 24, 19, 21, 26, 28, 23, 27, 29, 30, 22, 25, 38, 41};
	static const int intra_4x4_8x8[48] = {47, 31, 15, 0, 23, 27, 29, 30, 7, 11, 13, 14, 39, 43, 45, 46, 16, 3, 5, 10, 12, 19, 21, 26, 28, 35, 37, 42, 44, 1, 2, 4, 8, 17, 18, 20, 24, 6, 9, 22, 25, 32, 33, 34, 36, 40, 38, 41};

	static const int chroma_intra[16] = {15, 0, 7, 11, 13, 14, 3, 5, 10, 12, 1, 2, 4, 8, 6, 9};
	static const int chroma_intra_4x4_8x8[16] = {0, 1, 2, 4, 8, 3, 5, 10, 12, 15, 7, 11, 13, 14, 6, 9};

	int v = bitstream_read_ue(stream);
	if(chroma_format_idc)
	{
		assert(v >= 0 && v < 48);
		return coded_block_pattern ? intra[v] : intra_4x4_8x8[v];
	}
	else
	{
		assert(v >= 0 && v < 16);
		return coded_block_pattern ? chroma_intra[v] : chroma_intra_4x4_8x8[v];
	}
}

int bitstream_read_te(bitstream_t* stream)
{
	int v = bitstream_read_ue(stream);
	if(v != 1)
		return v;
	else
		return bitstream_read_bit(stream) ? 0 : 1;
}
