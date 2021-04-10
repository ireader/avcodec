#include "avpbs.h"
#include "h265-sps.h"
#include "mpeg4-hevc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define H265_NAL_SPS		33

#define h265_STARTCODE_PADDING 16  // 3->4 startcode

struct avpbs_h265_t
{
	int avs;
	struct mpeg4_hevc_t hevc;
	struct avstream_t* stream;
	avpbs_onpacket onpacket;
	void* param;
};

static int avpbs_h265_destroy(void** pp)
{
	struct avpbs_h265_t* bs;
	if (pp && *pp)
	{
		bs = (struct avpbs_h265_t*)*pp;
		avstream_release(bs->stream);
		free(bs);
		*pp = NULL;
	}
	return 0;
}

static void* avpbs_h265_create(int stream, AVPACKET_CODEC_ID codec, const uint8_t* extra, int bytes, avpbs_onpacket onpacket, void* param)
{
	struct avpbs_h265_t* bs;
	bs = calloc(1, sizeof(*bs));
	if (!bs) return NULL;

	// can be failure
	mpeg4_hevc_decoder_configuration_record_load(extra, bytes, &bs->hevc);
	assert(AVCODEC_VIDEO_H265 == codec);
	bs->onpacket = onpacket;
	bs->param = param;
	bs->avs = stream;
	return bs;
}

static int avpbs_h265_create_stream(struct avpbs_h265_t* bs)
{
	int x, y;
	struct h265_sps_t sps;

	avstream_release(bs->stream);
	bs->stream = avstream_alloc(bs->hevc.off + 2 * h265_STARTCODE_PADDING);
	if (!bs->stream)
		return -1;

	for (x = 0; x < bs->hevc.numOfArrays; x++)
	{
		if (H265_NAL_SPS != bs->hevc.nalu[x].type)
			continue;

		memset(&sps, 0, sizeof(sps));
		h265_sps_parse(bs->hevc.nalu[x].data, bs->hevc.nalu[x].bytes, &sps);
		h265_display_rect(&sps, &x, &y, &bs->stream->width, &bs->stream->height);
		break;
	}

	bs->stream->stream = bs->avs;
	bs->stream->codecid = AVCODEC_VIDEO_H265;
	bs->stream->bytes = mpeg4_hevc_decoder_configuration_record_save(&bs->hevc, bs->stream->extra, bs->stream->bytes);
	return bs->stream->bytes > 0 ? 0 : -1;
}

static int avpbs_h265_input(void* param, int64_t pts, int64_t dts, const uint8_t* nalu, int bytes, int flags)
{
	int r, vcl, update;
	struct avpacket_t* pkt;
	struct avpbs_h265_t* bs;	

	bs = (struct avpbs_h265_t*)param;
	pkt = avpacket_alloc(bytes + h265_STARTCODE_PADDING);
	if (!pkt) return -1;

	pkt->size = h265_annexbtomp4(&bs->hevc, nalu, bytes, pkt->data, pkt->size, &vcl, &update);
	if ((!bs->stream || update) && bs->hevc.numOfArrays >= 3)
		avpbs_h265_create_stream(bs);

	pkt->pts = pts;
	pkt->dts = dts;
	pkt->flags = flags | (1 == vcl ? AVPACKET_FLAG_KEY : 0);
	pkt->stream = bs->stream;
	avstream_addref(bs->stream);

	r = bs->onpacket(bs->param, bs->stream ? pkt : NULL);
	avpacket_release(pkt);
	return r;
}

struct avpbs_t* avpbs_h265(void)
{
	static struct avpbs_t bs = {
		.destroy = avpbs_h265_destroy,
		.create = avpbs_h265_create,
		.input = avpbs_h265_input,
	};
	return &bs;
}
