#ifndef _h264_internal_h_
#define _h264_internal_h_

#include "h264-nal.h"
#include "h264-sps.h"
#include "h264-pps.h"
#include "h264-vui.h"
#include "h264-hrd.h"
#include "h264-sei.h"
#include "h264-scaling.h"
#include "h264-slice-header.h"
#include "bitstream.h"

#define sizeof_array(v) (sizeof(v) / sizeof((v)[0]))

struct h264_context_t
{
	struct h264_sps_t sps[32];
	struct h264_pps_t pps[256];
	struct h264_sei_t sei;

	struct h264_sps_t *_sps; // current sps
	struct h264_pps_t *_pps; // current pps
};

int h264_nal(bitstream_t* stream, struct h264_nal_t* nal);
int h264_vui(bitstream_t* stream, struct h264_vui_t* vui);
int h264_hrd(bitstream_t* stream, struct h264_hrd_t* hrd);
int h264_sps(bitstream_t* stream, struct h264_sps_t* sps);
int h264_pps(bitstream_t* stream, struct h264_context_t* h264, struct h264_pps_t* pps);
int h264_slice_header(bitstream_t* stream, struct h264_context_t* h264, struct h264_nal_t* nal, struct h264_slice_header_t* header);
int h264_sei(bitstream_t* stream, struct h264_context_t* h264);

int h264_rbsp_trailing_bits(bitstream_t* stream);
int h264_more_rbsp_data(bitstream_t* stream);

struct h264_sps_t* h264_sps_get(struct h264_context_t* h264, int id);
struct h264_pps_t* h264_pps_get(struct h264_context_t* h264, int id);

#endif /* !_h264_internal_h_ */
