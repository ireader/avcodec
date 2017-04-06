#include "avdecoder.h"
#include "ffdecoder.h"
#include <string.h>
#include <assert.h>

//void* avdecoder_create(int codecId)
//{
//	AVCodecParameters param;
//	memset(&param, 0, sizeof(AVCodecParameters));
//	param.codec_id = codecId;
//	return ffdecoder_create(&param);
//}

void* avdecoder_create_h264()
{
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_VIDEO;
	param.codec_id = AV_CODEC_ID_H264;
	return ffdecoder_create(&param);
}

void* avdecoder_create_aac()
{
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_AAC;
	return ffdecoder_create(&param);
}

void avdecoder_destroy(void* ff)
{
	ffdecoder_destroy(ff);
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

static void avframe_to(const struct avframe_t* avframe, AVFrame* frame)
{
	int i;
	memset(frame, 0, sizeof(AVFrame));
	frame->format = avframe->format;
	frame->pkt_pts = avframe->pts;
	frame->pkt_dts = avframe->dts;
	frame->flags = avframe->flags;
	frame->width = avframe->width;
	frame->height = avframe->height;
	frame->channels = avframe->channel;
	frame->nb_samples = avframe->samples;
	frame->sample_rate = avframe->sample_rate;
	for (i = 0; i < sizeof(avframe->data) / sizeof(avframe->data[0]); i++)
	{
		frame->data[i] = avframe->data[i];
		frame->linesize[i] = avframe->linesize[i];
	}
}

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
	avframe->samples = frame->nb_samples;
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
	if(NULL != pkt)
		avpacket_to(pkt, &pkt2);
	return ffdecoder_input(ff, NULL==pkt ? NULL : &pkt2);
}

/// @param[in] frame must be memset to 0 or from av_frame_alloc()
/// @return >=0-got frame, <0-error
void* avdecoder_getframe(void* ff)
{
	int r;
	AVFrame* avframe;
	avframe = av_frame_alloc();
	r = ffdecoder_getframe(ff, avframe);
	if (r >= 0)
		return avframe;

	av_frame_free(&avframe);
	return NULL;
}

void avdecoder_freeframe(void* ff, void* frame)
{
	//AVFrame pic;
	//avframe_to(frame, &pic);
	//av_frame_unref(&pic);

	AVFrame *p = (AVFrame*)frame;
	av_frame_free(&p);
}

void avdecoder_frame_to(const void* frame, struct avframe_t* to)
{
	const AVFrame* avframe = (const AVFrame*)frame;
	avframe_from(avframe, to);
}
