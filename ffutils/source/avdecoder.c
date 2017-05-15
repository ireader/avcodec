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

static int ffmpeg_audio_format_from(int format)
{
	switch (format)
	{
	case PCM_SAMPLE_FMT_U8: return AV_SAMPLE_FMT_U8;
	case PCM_SAMPLE_FMT_S16: return AV_SAMPLE_FMT_S16;
	case PCM_SAMPLE_FMT_S32: return AV_SAMPLE_FMT_S32;
	case PCM_SAMPLE_FMT_FLOAT: return AV_SAMPLE_FMT_FLT;
	case PCM_SAMPLE_FMT_DOUBLE: return AV_SAMPLE_FMT_DBL;
	case PCM_SAMPLE_FMT_S16P: return AV_SAMPLE_FMT_S16P;
	case PCM_SAMPLE_FMT_S32P: return AV_SAMPLE_FMT_S32P;
	case PCM_SAMPLE_FMT_FLTP: return AV_SAMPLE_FMT_FLTP;
	case PCM_SAMPLE_FMT_DBLP: return AV_SAMPLE_FMT_DBLP;
	default: return AV_SAMPLE_FMT_S16;
	}
}

static int ffmpeg_audio_format_to(int ffmpeg)
{
	switch (ffmpeg)
	{
	case AV_SAMPLE_FMT_U8: return PCM_SAMPLE_FMT_U8;
	case AV_SAMPLE_FMT_S16: return PCM_SAMPLE_FMT_S16;
	case AV_SAMPLE_FMT_S32: return PCM_SAMPLE_FMT_S32;
	case AV_SAMPLE_FMT_FLT: return PCM_SAMPLE_FMT_FLOAT;
	case AV_SAMPLE_FMT_DBL: return PCM_SAMPLE_FMT_DOUBLE;
	case AV_SAMPLE_FMT_S16P: return PCM_SAMPLE_FMT_S16P;
	case AV_SAMPLE_FMT_S32P: return PCM_SAMPLE_FMT_S32P;
	case AV_SAMPLE_FMT_FLTP: return PCM_SAMPLE_FMT_FLTP;
	case AV_SAMPLE_FMT_DBLP: return PCM_SAMPLE_FMT_DBLP;
	default: return PCM_SAMPLE_FMT_S16;
	}
}

static void avframe_to(const struct avframe_t* avframe, AVFrame* ffmpeg)
{
	int i;
	memset(ffmpeg, 0, sizeof(AVFrame));
	ffmpeg->format = avframe->samples > 0 ? ffmpeg_audio_format_from(avframe->format) : avframe->format;
	ffmpeg->pkt_pts = avframe->pts;
	ffmpeg->pkt_dts = avframe->dts;
	ffmpeg->flags = avframe->flags;
	ffmpeg->width = avframe->width;
	ffmpeg->height = avframe->height;
	ffmpeg->channels = avframe->channels;
	ffmpeg->nb_samples = avframe->samples;
	ffmpeg->sample_rate = avframe->sample_rate;
	for (i = 0; i < sizeof(avframe->data) / sizeof(avframe->data[0]); i++)
	{
		ffmpeg->data[i] = avframe->data[i];
		ffmpeg->linesize[i] = avframe->linesize[i];
	}
}

static void avframe_from(const AVFrame* ffmpeg, struct avframe_t* avframe)
{
	int i;
	memset(avframe, 0, sizeof(struct avframe_t));
	avframe->format = ffmpeg->nb_samples > 0 ? ffmpeg_audio_format_to(ffmpeg->format) : ffmpeg->format; //AV_PIX_FMT_YUV420P;
	avframe->pts = ffmpeg->pkt_pts;
	avframe->dts = ffmpeg->pkt_dts;
	avframe->flags = ffmpeg->flags;
	avframe->width = ffmpeg->width;
	avframe->height = ffmpeg->height;
	avframe->channels = ffmpeg->channels;
	avframe->samples = ffmpeg->nb_samples;
	avframe->sample_bits = 8 * av_get_bytes_per_sample(ffmpeg->format);
	avframe->sample_rate = ffmpeg->sample_rate;
	for (i = 0; i < sizeof(avframe->data)/sizeof(avframe->data[0]); i++)
	{
		avframe->data[i] = ffmpeg->data[i];
		avframe->linesize[i] = ffmpeg->linesize[i];
	}
}

/// @return 0-ok, other-error
int avdecoder_input(void* ff, const struct avpacket_t* pkt)
{
	AVPacket pkt2;
	if (NULL != pkt)
		avpacket_to(pkt, &pkt2);
	return ffdecoder_input(ff, NULL == pkt ? NULL : &pkt2);
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
