#include "h265-nal.h"
#include "h265-parser.h"

int h265_nal(bitstream_t* stream, struct h265_nal_t* nal)
{
	bitstream_read_bit(stream); // forbidden_zero_bit
	nal->nal_unit_type = bitstream_read_bits(stream, 6);
	nal->nuh_layer_id = bitstream_read_bits(stream, 6);
	nal->nuh_temporal_id_plus1 = bitstream_read_bits(stream, 3);
	return 0;
}
