#ifndef _h265_hrd_h_
#define _h265_hrd_h_

#include <stdint.h>

struct h265_hrd_t
{
	unsigned int nal_hrd_parameters_present_flag : 1;
	unsigned int vcl_hrd_parameters_present_flag : 1;
	unsigned int sub_pic_hrd_params_present_flag : 1;

	uint8_t cpb_cnt_minus1; // [0, 32]
	uint32_t bit_rate_value_minus1[32];
	uint32_t cpb_size_value_minus1[32];
	uint8_t cbr_flag[32];

	// sub_pic_hrd_params_present_flag
	unsigned int tick_divisor_minus2 : 8;
	unsigned int du_cpb_removal_delay_increment_length_minus1 : 5;
	unsigned int sub_pic_cpb_params_in_pic_timing_sei_flag : 1;
	unsigned int dpb_output_delay_du_length_minus1 : 5;
	
	unsigned int bit_rate_scale : 4;
	unsigned int cpb_size_scale : 4;
	unsigned int cpb_size_du_scale : 4;

	unsigned int initial_cpb_removal_delay_length_minus1 : 5;
	unsigned int au_cpb_removal_delay_length_minus1 : 5;
	unsigned int dpb_output_delay_length_minus1 : 5;

	struct
	{
		unsigned int  fixed_pic_rate_general_flag : 1;
		unsigned int  fixed_pic_rate_within_cvs_flag : 1;
		unsigned int  elemental_duration_in_tc_minus1 : 16;
		unsigned int  low_delay_hrd_flag : 1;
		unsigned int  cpb_cnt_minus1 : 5; // ue [0, 31]
	} sub_layer_hdr_parameters[7];
};

#endif /* !_h265_hrd_h_ */
