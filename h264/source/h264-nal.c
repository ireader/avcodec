#include "h264-nal.h"
#include "h264-internal.h"
#include <assert.h>
#include <string.h>

int h264_nal(bitstream_t* stream, struct h264_nal_t* nal)
{
	memset(nal, 0, sizeof(struct h264_nal_t));
	nal->forbidden_zero_bit = bitstream_read_bit(stream);
	nal->nal_ref_idc = bitstream_read_bits(stream, 2);
	nal->nal_unit_type = bitstream_read_bits(stream, 5);
	assert(0 == nal->forbidden_zero_bit);

	if (H264_NAL_PREFIX == nal->nal_unit_type ||
		H264_NAL_XVC == nal->nal_unit_type ||
		H264_NAL_3D == nal->nal_unit_type)
	{
		if (H264_NAL_3D != nal->nal_unit_type)
			nal->svc_extension_flag = bitstream_read_bit(stream);
		else
			nal->avc_3d_extension_flag = bitstream_read_bit(stream);

		if (nal->svc_extension_flag)
		{
			//nal_unit_header_svc_extension(); /* specified in Annex G */
		}
		else if (nal->avc_3d_extension_flag)
		{
			//nal_unit_header_3davc_extension(); /* specified in Annex J */
		}
		else
		{
			//nal_unit_header_mvc_extension(); /* specified in Annex H */
		}
	}

	return 0;
}
