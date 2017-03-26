#include "avdecoder.h"
#include "ffdecoder.h"
#include <memory.h>
#include <assert.h>

struct avdecoder_t
{
	void* ffdecoder;
	AVFrame* frame;
};

void* avdecoder_create(int codecId)
{
	AVCodecParameters param;
	struct avdecoder_t* decoder;
	decoder = (struct avdecoder_t*)malloc(sizeof(struct avdecoder_t));
	if (NULL == decoder)
		return NULL;

	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_id = codecId;

	memset(decoder, 0, sizeof(struct avdecoder_t));
	decoder->ffdecoder = ffdecoder_create(&param);
	decoder->frame = av_frame_alloc();
	if (NULL == decoder->ffdecoder || NULL == decoder->frame)
	{
		avdecoder_destroy(decoder);
		return NULL;
	}
	return decoder;
}

void avdecoder_destroy(void* ff)
{
	struct avdecoder_t* decoder;
	decoder = (struct avdecoder_t*)ff;
	if (decoder->ffdecoder)
	{
		ffdecoder_destroy(decoder->ffdecoder);
		decoder->ffdecoder = NULL;
	}
	if (decoder->frame)
		av_frame_free(&decoder->frame);
	free(decoder);
}

static void avpacket_to(const struct avpacket_t* avpkt, AVPacket* pkt)
{
	assert(avpkt->bytes > 0);
	//assert(avpkt->bytes > AV_INPUT_BUFFER_PADDING_SIZE);
	av_init_packet(pkt);
	pkt->data = avpkt->data;
	pkt->size = avpkt->bytes;
	pkt->pts = avpkt->pts;
	pkt->dts = avpkt->dts;
	pkt->flags = avpkt->flags;
}

//static void avframe_to(const struct avframe_t* avframe, AVFrame* frame)
//{
//	int i;
//	memset(frame, 0, sizeof(AVFrame));
//	frame->format = avframe->format;
//	frame->pkt_pts = avframe->pts;
//	frame->pkt_dts = avframe->dts;
//	frame->flags = avframe->flags;
//	frame->width = avframe->width;
//	frame->height = avframe->height;
//	frame->channels = avframe->channel;
//	frame->sample_rate = avframe->sample_rate;
//	for (i = 0; i < sizeof(avframe->data) / sizeof(avframe->data[0]); i++)
//	{
//		frame->data[i] = avframe->data[i];
//		frame->linesize[i] = avframe->linesize[i];
//	}
//}

static void avframe_from(const AVFrame* frame, struct avframe_t* avframe)
{
	int i;
	memset(avframe, 0, sizeof(struct avframe_t));
	avframe->format = frame->format; //AV_PIX_FMT_YUV420P;
	avframe->pts = frame->pkt_pts;
	avframe->dts = frame->pkt_dts;
	avframe->flags = frame->flags;
	avframe->width = frame->width;
	avframe->height = frame->height;
	avframe->channel = frame->channels;
	avframe->sample_bits = 8 * av_get_bytes_per_sample(frame->format);
	avframe->sample_rate = frame->sample_rate;
	for (i = 0; i < sizeof(avframe->data)/sizeof(avframe->data[0]); i++)
	{
		avframe->data[i] = frame->data[i];
		avframe->linesize[i] = frame->linesize[i];
	}
}

/// @return 0-ok, other-error
int avdecoder_input(void* ff, const struct avpacket_t* pkt)
{
	AVPacket pkt2;
	struct avdecoder_t* decoder;
	decoder = (struct avdecoder_t*)ff;

	avpacket_to(pkt, &pkt2);
	return ffdecoder_input(decoder->ffdecoder, &pkt2);
}

/// @param[in] frame must be memset to 0 or from av_frame_alloc()
/// @return >=0-got frame, <0-error
int avdecoder_getframe(void* ff, struct avframe_t* frame)
{
	int r;
	struct avdecoder_t* decoder;
	decoder = (struct avdecoder_t*)ff;
	r = ffdecoder_getframe(decoder->ffdecoder, decoder->frame);
	if (r >= 0)
	{
		avframe_from(decoder->frame, frame);
	}

	return r;
}

//void avdecoder_freeframe(void* ff, struct avframe_t* frame)
//{
//	AVFrame pic;
//	avframe_to(frame, &pic);
//	av_frame_unref(&pic);
//}
