#ifndef _h265_pps_h_
#define _h265_pps_h_

#include <stdint.h>

struct h265_pps_t
{
	uint32_t pps_pic_parameter_set_id; // ue
	uint32_t pps_seq_parameter_set_id; // ue
};

#endif /* !_h265_pps_h_ */
