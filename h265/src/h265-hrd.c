#include "h265-hrd.h"
#include "h265-vui.h"
#include "h265-parser.h"
#include "h265-internal.h"
#include <stdlib.h>
#include <string.h>

static void h265_sub_layer_hrd_parameters(bitstream_t* stream, struct h265_hrd_t* hrd, int subLayerId, int CpbCnt, int subPicHrdParamsPresentFlag) 
{
	int i;
	for (i = 0; i < CpbCnt; i++)
	{
		/*bit_rate_value_minus1[i] =*/ bitstream_read_ue(stream);
		/*cpb_size_value_minus1[i] =*/ bitstream_read_ue(stream);

		if (subPicHrdParamsPresentFlag) {
			/*cpb_size_du_value_minus1[i] =*/ bitstream_read_ue(stream);
			/*bit_rate_du_value_minus1[i] =*/ bitstream_read_ue(stream);
		}

		/*cbr_flag[i] =*/bitstream_read_bit(stream);
	}
}

int h265_hrd(bitstream_t* stream, struct h265_hrd_t* hrd, int commonInfPresentFlag, int maxNumSubLayersMinus1)
{
	int i;
	uint8_t CpbCnt;

	if (commonInfPresentFlag)
	{
		hrd->nal_hrd_parameters_present_flag = bitstream_read_bit(stream);
		hrd->vcl_hrd_parameters_present_flag = bitstream_read_bit(stream);
		if (hrd->nal_hrd_parameters_present_flag || hrd->vcl_hrd_parameters_present_flag)
		{
			hrd->sub_pic_hrd_params_present_flag = bitstream_read_bit(stream);
			if (hrd->sub_pic_hrd_params_present_flag)
			{
				hrd->tick_divisor_minus2 = bitstream_read_bits(stream, 8);
				hrd->du_cpb_removal_delay_increment_length_minus1 = bitstream_read_bits(stream, 5);
				hrd->sub_pic_cpb_params_in_pic_timing_sei_flag = bitstream_read_bits(stream, 1);
				hrd->dpb_output_delay_du_length_minus1 = bitstream_read_bits(stream, 5);
			}

			hrd->bit_rate_scale = bitstream_read_bits(stream, 4);
			hrd->cpb_size_scale = bitstream_read_bits(stream, 4);

			if (hrd->sub_pic_hrd_params_present_flag) {
				hrd->cpb_size_du_scale = bitstream_read_bits(stream, 4);
			}

			hrd->initial_cpb_removal_delay_length_minus1 = bitstream_read_bits(stream, 5);
			hrd->au_cpb_removal_delay_length_minus1 = bitstream_read_bits(stream, 5);
			hrd->dpb_output_delay_length_minus1 = bitstream_read_bits(stream, 5);
		}
	}

	for (i = 0; i <= maxNumSubLayersMinus1 && i < sizeof_array(hrd->sub_layer_hdr_parameters); i++)
	{
		memset(&hrd->sub_layer_hdr_parameters[i], 0, sizeof(hrd->sub_layer_hdr_parameters[i]));
		hrd->sub_layer_hdr_parameters[i].fixed_pic_rate_within_cvs_flag = bitstream_read_bit(stream);
		if (!hrd->sub_layer_hdr_parameters[i].fixed_pic_rate_within_cvs_flag)
			hrd->sub_layer_hdr_parameters[i].fixed_pic_rate_within_cvs_flag = bitstream_read_bit(stream);

		if (hrd->sub_layer_hdr_parameters[i].fixed_pic_rate_within_cvs_flag)
			hrd->sub_layer_hdr_parameters[i].elemental_duration_in_tc_minus1 = bitstream_read_ue(stream);
		else
			hrd->sub_layer_hdr_parameters[i].low_delay_hrd_flag = bitstream_read_bit(stream);

		if (!hrd->sub_layer_hdr_parameters[i].low_delay_hrd_flag)
			hrd->sub_layer_hdr_parameters[i].cpb_cnt_minus1 = bitstream_read_ue(stream);

		CpbCnt = hrd->sub_layer_hdr_parameters[i].cpb_cnt_minus1 + 1;
		if (hrd->nal_hrd_parameters_present_flag)
			h265_sub_layer_hrd_parameters(stream, hrd, i, CpbCnt, hrd->sub_pic_hrd_params_present_flag);

		if (hrd->vcl_hrd_parameters_present_flag)
			h265_sub_layer_hrd_parameters(stream, hrd, i, CpbCnt, hrd->sub_pic_hrd_params_present_flag);
	}

	return 0;
}
