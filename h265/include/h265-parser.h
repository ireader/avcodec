#ifndef _h265_parser_h_
#define _h265_parser_h_

#include "bitstream.h"
#include "h265-vps.h"
#include "h265-sps.h"
#include "h265-pps.h"
#include "h265-nal.h"

int h265_nal(bitstream_t* stream, struct h265_nal_t* nal);
int h265_vps(bitstream_t* stream, struct h265_vps_t* vps);
int h265_sps(bitstream_t* stream, struct h265_sps_t* sps);
int h265_pps(bitstream_t* stream, struct h265_pps_t* pps);

int h265_profile_tier_level(bitstream_t* stream, struct h265_profile_tier_level_t* profile, int profilePresentFlag, int maxNumSubLayersMinus1);

#endif /* !_h265_parser_h_ */
