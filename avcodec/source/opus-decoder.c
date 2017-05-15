#include "audio-decoder.h"
#include "opus/opus.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct opus_decoder_t
{
	OpusDecoder* opus;
	struct avframe_t pic;
	int capacity;
};

static void opus_destroy(void* audio)
{
	struct opus_decoder_t* dec;
	dec = (struct opus_decoder_t*)audio;
	if (dec->opus)
	{
		opus_decoder_destroy(dec->opus);
		dec->opus = NULL;
	}
	free(dec);
}

static void* opus_create(int format, int channels, int frequency)
{
	int r = OPUS_OK;
	struct opus_decoder_t* dec;

	r = frequency / 2 * channels * PCM_SAMPLE_BITS(format) / 8; // 500ms
	dec = (struct opus_decoder_t*)malloc(sizeof(*dec) + r);
	if (NULL == dec)
		return NULL;

	memset(dec, 0, sizeof(*dec));
	dec->pic.format = format;
	dec->pic.channels = channels;
	dec->pic.sample_rate = frequency;
	dec->pic.sample_bits = PCM_SAMPLE_BITS(format);
	dec->pic.data[0] = (uint8_t*)(dec + 1);
	dec->capacity = r;
	dec->opus = opus_decoder_create(frequency, channels, &r);
	if (NULL == dec->opus)
	{
		opus_destroy(dec);
		return NULL;
	}
	return dec;
}

static int opus_input(void* audio, const struct avpacket_t* pkt)
{
	struct opus_decoder_t* dec;
	dec = (struct opus_decoder_t*)audio;
	dec->pic.samples = opus_decode(dec->opus, (const unsigned char*)pkt->data, pkt->bytes, (opus_int16*)dec->pic.data[0], dec->capacity, 0);
	if (dec->pic.samples > 0)
	{
		dec->pic.pts = pkt->pts;
		dec->pic.dts = pkt->dts;
		dec->pic.linesize[0] = dec->pic.samples *  dec->pic.channels * dec->pic.sample_bits / 8;
	}
	return dec->pic.samples;
}

static int opus_getframe(void* audio, struct avframe_t* pic)
{
	struct opus_decoder_t* dec;
	dec = (struct opus_decoder_t*)audio;
	if (dec->pic.samples > 0)
	{
		memcpy(pic, &dec->pic, sizeof(*pic));
		dec->pic.samples = 0; // clear
		return 0;
	}
	return -1;
}

struct audio_decoder_t* opus_decoder()
{
	static struct audio_decoder_t s_decoder = {
		opus_create,
		opus_destroy,
		opus_input,
		opus_getframe,
	};
	return &s_decoder;
}
