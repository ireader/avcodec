#include "h264-parser.h"
#include "h264-internal.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>

struct h264_parser_t
{
	struct h264_context_t ctx;
	uint32_t frame_num;
	int flags;
};

void* h264_parser_create()
{
	struct h264_parser_t* parser;
	parser = (struct h264_parser_t*)malloc(sizeof(*parser));
	if (NULL == parser)
		return NULL;
	memset(parser, 0, sizeof(*parser));
	return parser;
}

void h264_parser_destroy(void* p)
{
	struct h264_parser_t* parser;
	parser = (struct h264_parser_t*)p;
	free(parser);
}

struct h264_sps_t* h264_sps_get(struct h264_context_t* h264, int id)
{
	if (id >= 0 && id < sizeof(h264->sps) / sizeof(struct h264_sps_t))
		return h264->sps + id;
	return NULL;
}

struct h264_pps_t* h264_pps_get(struct h264_context_t* h264, int id)
{
	if (id >= 0 && id < sizeof(h264->pps) / sizeof(struct h264_pps_t))
		return h264->pps + id;
	return NULL;
}

struct h264_sps_t* h264_sps_from_pps_id(struct h264_context_t* h264, int id)
{
	struct h264_pps_t* pps;
	pps = h264_pps_get(h264, id);
	if (pps)
		return h264_sps_get(h264, pps->seq_parameter_set_id);
	return NULL;
}

uint32_t h264_frame_num(struct h264_sps_t* sps)
{
	return 1 << (sps->log2_max_frame_num_minus4 + 4);
}

int h264_parser_input(void* p, const void* nalu, size_t bytes)
{
	int r = -1;
	bitstream_t stream;
	struct h264_nal_t nal;
	struct h264_sps_t sps;
	struct h264_pps_t pps;
	struct h264_parser_t* parser;
	parser = (struct h264_parser_t*)p;

	bitstream_init(&stream, (const unsigned char*)nalu, bytes);

	if (0 != h264_nal(&stream, &nal))
	{
		printf("%s: h264_nalu(%02X) error.\n", __FUNCTION__, (int)*(unsigned char*)nalu);
		return -1;
	}

	assert(0 == nal.forbidden_zero_bit);
	switch (nal.nal_unit_type)
	{
	case H264_NAL_SPS:
		assert(nal.nal_ref_idc > 0);
		r = h264_sps(&stream, &sps);
		assert(sps.seq_parameter_set_id < sizeof(parser->ctx.sps) / sizeof(struct h264_sps_t));
		if (0 == r && sps.seq_parameter_set_id < sizeof(parser->ctx.sps) / sizeof(struct h264_sps_t))
			memcpy(parser->ctx.sps + sps.seq_parameter_set_id, &sps, sizeof(struct h264_sps_t));
		break;

	case H264_NAL_PPS:
		assert(nal.nal_ref_idc > 0);
		r = h264_pps(&stream, &parser->ctx, &pps);
		assert(pps.pic_parameter_set_id < sizeof(parser->ctx.pps) / sizeof(struct h264_pps_t));
		if (0 == r && pps.pic_parameter_set_id < sizeof(parser->ctx.pps) / sizeof(struct h264_pps_t))
			memcpy(parser->ctx.pps + pps.pic_parameter_set_id, &pps, sizeof(struct h264_pps_t));
		break;

	default:
		if (nal.nal_unit_type > 0 && nal.nal_unit_type <= H264_NAL_IDR)
		{
			struct h264_slice_header_t header;
			r = h264_slice_header(&stream, &parser->ctx, &nal, &header);
			if (0 == r)
			{
				// check framenum
				if (nal.nal_unit_type == H264_NAL_IDR)
					parser->flags = 1;
				else if(1 == parser->flags)
					parser->flags = header.frame_num == parser->frame_num || header.frame_num == ((parser->frame_num + 1) % h264_frame_num(h264_sps_from_pps_id(&parser->ctx, header.pic_parameter_set_id)));
				
				parser->frame_num = header.frame_num;
			}
		}
		break;
	}
	return r;
}

int h264_parser_getflags(void* p)
{
	struct h264_parser_t* parser;
	parser = (struct h264_parser_t*)p;
	return parser->flags;
}
