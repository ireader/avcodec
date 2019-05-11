#include "h265-parser.h"


/// @param[in] nalu H.265 nalu stream, don't include startcode(0x000001)
int h265_parse(uint8_t *nalu, int bytes)
{
	bitstream_t stream;
	struct h265_nal_t nal;
	struct h265_vps_t vps;
	struct h265_sps_t sps;
	struct h265_pps_t pps;
	bitstream_init(&stream, (const unsigned char*)nalu, bytes);

	h265_nal(&stream, &nal);

	switch(nal.nal_unit_type)
	{
	case H265_NAL_VPS:
		h265_vps(&stream, &vps);
		break;

	case H265_NAL_SPS:
		h265_sps(&stream, &sps);
		break;

	case H265_NAL_PPS:
		h265_pps(&stream, &pps);
		break;
	}

	return 0;
}
