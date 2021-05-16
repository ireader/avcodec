#include "avpbs.h"
#include "mpeg4-aac.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct avpbs_aac_t
{
	int avs;
	struct mpeg4_aac_t aac;
	struct avstream_t* stream;
	avpbs_onpacket onpacket;
	void* param;
};

static int avpbs_aac_destroy(void** pp)
{
	struct avpbs_aac_t* bs;
	if (pp && *pp)
	{
		bs = (struct avpbs_aac_t*)*pp;
		avstream_release(bs->stream);
		free(bs);
		*pp = NULL;
	}
	return 0;
}

static void* avpbs_aac_create(int stream, AVPACKET_CODEC_ID codec, const uint8_t* extra, int bytes, avpbs_onpacket onpacket, void* param)
{
	struct avpbs_aac_t* bs;
	bs = calloc(1, sizeof(*bs));
	if (!bs) return NULL;

	// can be failure
	mpeg4_aac_audio_specific_config_load(extra, bytes, &bs->aac);
	assert(AVCODEC_AUDIO_AAC == codec);
	bs->onpacket = onpacket;
	bs->param = param;
	bs->avs = stream;
	return bs;
}

static int avpbs_aac_create_stream(struct avpbs_aac_t* bs)
{
	avstream_release(bs->stream);
	bs->stream = avstream_alloc(2 + bs->aac.npce);
	if (!bs->stream)
		return -1;

	bs->stream->stream = bs->avs;
	bs->stream->codecid = AVCODEC_AUDIO_AAC;
	bs->stream->channels = bs->aac.channels;
	bs->stream->sample_rate = bs->aac.sampling_frequency;
	bs->stream->sample_bits = 16;
	bs->stream->bytes = mpeg4_aac_audio_specific_config_save(&bs->aac, bs->stream->extra, bs->stream->bytes);
	return bs->stream->bytes > 0 ? 0 : -1;
}

static int avpbs_aac_input(void* param, int64_t pts, int64_t dts, const uint8_t* data, int bytes, int flags)
{
	int r;
	struct avpacket_t* pkt;
	struct avpbs_aac_t* bs;

	bs = (struct avpbs_aac_t*)param;
	pkt = avpacket_alloc(bytes);
	if (!pkt) return -1;

	r = 0;
	if (bytes >= 7 && 0xFF == data[0] && 0xF0 == (data[1] & 0xF0))
	{
		r = mpeg4_aac_adts_load(data, bytes, &bs->aac);
		if (r < 0) return r;
	}
	
	if (!bs->stream || bs->stream->channels != bs->aac.channels
		|| bs->stream->sample_rate != (int)bs->aac.sampling_frequency)
	{
		if (0 != avpbs_aac_create_stream(bs))
			return -1;
	}

	memcpy(pkt->data, data + r, bytes - r);
	pkt->size = bytes - r;
	pkt->pts = pts;
	pkt->dts = dts;
	pkt->flags = flags;
	pkt->stream = bs->stream;
	avstream_addref(bs->stream);

	r = bs->onpacket(bs->param, bs->stream ? pkt : NULL);
	avpacket_release(pkt);
	return r;
}

struct avpbs_t* avpbs_aac(void)
{
	static struct avpbs_t bs = {
		.destroy = avpbs_aac_destroy,
		.create = avpbs_aac_create,
		.input = avpbs_aac_input,
	};
	return &bs;
}
