#ifndef _bitstream_h_
#define _bitstream_h_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _bitstream_t
{
	const unsigned char* rbsp;
	size_t size;
	size_t bits; // offset bit
	int error;
} bitstream_t;

void bitstream_init(bitstream_t* stream, const unsigned char* rbsp, size_t bytes);

/// @return 0-ok, other-error
int bitstream_error(bitstream_t* stream);

int bitstream_get_offset(bitstream_t* stream, size_t* bits);
int bitstream_set_offset(bitstream_t* stream, size_t bits);

/// get 1-bit from bit stream(don't offset position)
/// @param[in] stream bit stream
/// @return -1-error, 1-value, 0-value
int bitstream_next_bit(bitstream_t* stream);

/// read n-bit(n <= 32) from bit stream(don't offset position)
/// @param[in] stream bit stream
/// @return -1-error, 1-value, 0-value
int bitstream_next_bits(bitstream_t* stream, int bits);

/// read 1-bit from bit stream(offset position)
/// @param[in] stream bit stream
/// @return -1-error, 1-value, 0-value
int bitstream_read_bit(bitstream_t* stream);

/// read n-bit(n <= 64) from bit stream(offset position)
/// @param[in] stream bit stream
/// @return -1-error, 1-value, 0-value
int64_t bitstream_read_bits(bitstream_t* stream, int bits);

int bitstream_read_ue(bitstream_t* stream);
int bitstream_read_se(bitstream_t* stream);
int bitstream_read_me(bitstream_t* stream, int chroma_format_idc, int coded_block_pattern);
int bitstream_read_te(bitstream_t* stream);


#ifdef __cplusplus
}
#endif

#endif /* !_bitstream_h_ */
