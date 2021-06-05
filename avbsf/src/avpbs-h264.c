#include "avpbs.h"
#include "h264-sps.h"
#include "mpeg4-avc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define H264_STARTCODE_PADDING 16  // 3->4 startcode

struct avpbs_h264_t
{
	int avs;
	struct mpeg4_avc_t avc;
	struct avstream_t* stream;
	avpbs_onpacket onpacket;
	void* param;
};

static int avpbs_h264_create_stream(struct avpbs_h264_t* bs)
{
	int x, y;
	struct h264_sps_t sps;

	avstream_release(bs->stream);
	bs->stream = avstream_alloc(bs->avc.off + 8 * H264_STARTCODE_PADDING);
	if (!bs->stream)
		return -1;

	memset(&sps, 0, sizeof(sps));
	h264_sps_parse(bs->avc.sps[0].data, bs->avc.sps[0].bytes, &sps);
	h264_display_rect(&sps, &x, &y, &bs->stream->width, &bs->stream->height);

	bs->stream->stream = bs->avs;
	bs->stream->codecid = AVCODEC_VIDEO_H264;
	bs->stream->bytes = mpeg4_avc_decoder_configuration_record_save(&bs->avc, bs->stream->extra, bs->stream->bytes);
	return bs->stream->bytes > 0 ? 0 : -1;
}

static int avpbs_h264_destroy(void** pp)
{
	struct avpbs_h264_t* bs;
	if (pp && *pp)
	{
		bs = (struct avpbs_h264_t*)*pp;
		avstream_release(bs->stream);
		free(bs);
		*pp = NULL;
	}
	return 0;
}

static void* avpbs_h264_create(int stream, AVPACKET_CODEC_ID codec, const uint8_t* extra, int bytes, avpbs_onpacket onpacket, void* param)
{
	int n;
	struct avpbs_h264_t* bs;
	bs = calloc(1, sizeof(*bs));
	if (!bs) return NULL;

	// can be failure
	assert(AVCODEC_VIDEO_H264 == codec);
	n = mpeg4_h264_bitstream_format(extra, bytes);
	if (n > 0)
		mpeg4_avc_decoder_configuration_record_load(extra, bytes, &bs->avc);
	else if (bytes > 4)
		h264_annexbtomp4(&bs->avc, extra, bytes, NULL, 0, NULL, NULL);

	if (bs->avc.nb_sps > 0 && bs->avc.nb_pps > 0)
		avpbs_h264_create_stream(bs);
	bs->onpacket = onpacket;
	bs->param = param;
	bs->avs = stream;
	return bs;
}

static int avpbs_h264_input(void* param, int64_t pts, int64_t dts, const uint8_t* nalu, int bytes, int flags)
{
	int r, vcl, update;
	struct avpacket_t* pkt;
	struct avpbs_h264_t* bs;
	
	bs = (struct avpbs_h264_t*)param;
	pkt = avpacket_alloc(bytes + H264_STARTCODE_PADDING);
	if (!pkt) return -1;

	pkt->size = h264_annexbtomp4(&bs->avc, nalu, bytes, pkt->data, pkt->size, &vcl, &update);
	if (update && bs->avc.nb_sps > 0 && bs->avc.nb_pps > 0 && 0 != avpbs_h264_create_stream(bs))
		return -1;

	if (!bs->stream)
		return -1;

	pkt->pts = pts;
	pkt->dts = dts;
	pkt->flags = flags | (1 == vcl ? AVPACKET_FLAG_KEY : 0);
	pkt->stream = bs->stream;
	avstream_addref(bs->stream);

	r = bs->onpacket(bs->param, bs->stream ? pkt : NULL);
	avpacket_release(pkt);
	return r;
}

struct avpbs_t* avpbs_h264(void)
{
	static struct avpbs_t bs = {
		.destroy = avpbs_h264_destroy,
		.create = avpbs_h264_create,
		.input = avpbs_h264_input,
	};
	return &bs;
}
