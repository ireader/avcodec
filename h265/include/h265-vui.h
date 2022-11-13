#ifndef _h265_vui_h_
#define _h265_vui_h_

#include "h265-hrd.h"

struct h265_vui_t
{
	unsigned int aspect_ratio_info_present_flag : 1;
	unsigned int overscan_info_present_flag : 1;
	unsigned int video_signal_type_present_flag : 1;
	unsigned int chroma_loc_info_present_flag : 1;
	unsigned int neutral_chroma_indication_flag : 1;
	unsigned int field_seq_flag : 1;
	unsigned int frame_field_info_present_flag : 1;
	unsigned int default_display_window_flag : 1;
	unsigned int vui_timing_info_present_flag : 1;
	unsigned int bitstream_restriction_flag : 1;

	// video_signal_type_present_flag
	unsigned int video_format : 3;
	unsigned int video_full_range_flag : 1;
	unsigned int colour_description_present_flag : 1;
	unsigned int colour_primaries : 8;
	unsigned int transfer_characteristics : 8;
	unsigned int matrix_coefficients : 8;

	struct h265_hrd_t hrd;
};

#endif /* !_h265_vui_h_ */
