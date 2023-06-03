#include "h265-vps.h"
#include "h265-parser.h"
#include "h265-internal.h"
#include <assert.h>

int h265_profile_tier_level(bitstream_t* stream, struct h265_profile_tier_level_t* profile, int profilePresentFlag, int maxNumSubLayersMinus1)
{
	int i;
	if(profilePresentFlag)
	{
		profile->general_profile_idc = (uint8_t)bitstream_read_bits(stream, 2);
		profile->general_tier_flag = (uint8_t)bitstream_read_bit(stream);
		profile->general_profile_idc = (uint8_t)bitstream_read_bits(stream, 5);
		profile->general_profile_compatibility_flag = bitstream_read_bits(stream, 32);
		profile->general_constraint_flags = (((uint64_t)bitstream_read_bits(stream, 32)) << 16) | bitstream_read_bits(stream, 16);
	}
	
	profile->general_level_idc = (uint8_t)bitstream_read_bits(stream, 8);

	for(i = 0; i < maxNumSubLayersMinus1 && i < sizeof_array(profile->sub_layer); i++)
	{
		profile->sub_layer[i].sub_layer_profile_present_flag = (uint8_t)bitstream_read_bit(stream);
		profile->sub_layer[i].sub_layer_level_present_flag = (uint8_t)bitstream_read_bit(stream);
	}

	// align to byte
	if(maxNumSubLayersMinus1 > 0)
	{
		for(i = maxNumSubLayersMinus1; i < 8; i++)
			bitstream_read_bits(stream, 2); // reserved_zero_2bits
	}

	for(i = 0; i < maxNumSubLayersMinus1 && i < sizeof_array(profile->sub_layer); i++)
	{
		if(profile->sub_layer[i].sub_layer_profile_present_flag)
		{
			profile->sub_layer[i].sub_layer_profile_space = (uint8_t)bitstream_read_bits(stream, 2);
			profile->sub_layer[i].sub_layer_tier_flag = (uint8_t)bitstream_read_bit(stream);
			profile->sub_layer[i].sub_layer_profile_idc = (uint8_t)bitstream_read_bits(stream, 5);
			profile->sub_layer[i].sub_layer_profile_compatibility_flag = bitstream_read_bits(stream, 32);
			profile->sub_layer[i].sub_layer_constraint_flags = (((uint64_t)bitstream_read_bits(stream, 32)) << 16) | bitstream_read_bits(stream, 16);
		}

		if(profile->sub_layer[i].sub_layer_level_present_flag)
			profile->sub_layer[i].sub_layer_level_idc = (uint8_t)bitstream_read_bits(stream, 8);
	}

	return 0;
}

int h265_vps(bitstream_t* stream, struct h265_vps_t* vps)
{
	vps->vps_video_parameter_set_id = (uint8_t)bitstream_read_bits(stream, 4);
	vps->vps_base_layer_internal_flag = (uint8_t)bitstream_read_bit(stream);
	vps->vps_base_layer_available_flag = (uint8_t)bitstream_read_bit(stream);
	vps->vps_max_layers_minus1 = (uint8_t)bitstream_read_bits(stream, 6);
	vps->vps_max_sub_layers_minus1 = (uint8_t)bitstream_read_bits(stream, 3);
	vps->vps_temporal_id_nesting_flag = (uint8_t)bitstream_read_bit(stream);
	bitstream_read_bits(stream, 16); //vps_reserved_0xffff_16bits

	h265_profile_tier_level(stream, &vps->profile, 1, vps->vps_max_sub_layers_minus1);

	// TODO: parse more
	return 0;
}

void h265_vps_test(void)
{
	const uint8_t data[] = {
		0x40, 0x01, 0x0c, 0x02, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5d, 0x00, 0x00, 0x97, 0x32, 0x81, 0x20,
	};

	bitstream_t stream;
	struct h265_nal_t nal;
	struct h265_vps_t vps;
	bitstream_init(&stream, (const unsigned char*)data, sizeof(data));

	h265_nal(&stream, &nal);
	assert(H265_NAL_VPS == nal.nal_unit_type);

	assert(0 == h265_vps(&stream, &vps));
}
