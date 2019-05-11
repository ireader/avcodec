#include "h265-pps.h"
#include "h265-parser.h"
#include <assert.h>

int h265_pps(bitstream_t* stream, struct h265_pps_t* pps)
{
	pps->pps_pic_parameter_set_id = (uint32_t)(bitstream_read_ue(stream));
	pps->pps_seq_parameter_set_id = (uint32_t)(bitstream_read_ue(stream));
	return 0;
}

void h265_pps_test(void)
{
	const uint8_t data[] = {
		0x44, 0x01, 0xc1, 0x54, 0xf0, 0x79, 0x22, 0x40,
	};

	bitstream_t stream;
	struct h265_nal_t nal;
	struct h265_pps_t pps;
	bitstream_init(&stream, (const unsigned char*)data, sizeof(data));

	h265_nal(&stream, &nal);
	assert(H265_NAL_PPS == nal.nal_unit_type);

	assert(0 == h265_pps(&stream, &pps));
}
