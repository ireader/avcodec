#include "ffdecoder.h"
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

void* ffdecoder_create(const AVCodecParameters* codecpar)
{
	int r;
	AVCodec* codec = NULL;
	AVDictionary *opts = NULL;
	AVCodecContext *avctx = NULL;

	avctx = avcodec_alloc_context3(NULL);
	if (!avctx)
		return NULL;

	r = avcodec_parameters_to_context(avctx, codecpar);
	if (r < 0)
	{
		avcodec_free_context(&avctx);
		return NULL;
	}

	//av_codec_set_pkt_timebase(avctx, ic->streams[stream_index]->time_base);
	codec = avcodec_find_decoder(avctx->codec_id);
	if (NULL == codec)
	{
		printf("[%s] avcodec_find_decoder(%d) not found.\n", __FUNCTION__, avctx->codec_id);
		avcodec_free_context(&avctx);
		return NULL;
	}

	//ff->avctx->codec_id = codec->id;
	assert(avctx->codec_id == codec->id);
	av_dict_set(&opts, "threads", "1"/*"auto"*/, 0); // disable multi-thread decode
	r = avcodec_open2(avctx, codec, &opts);
	av_dict_free(&opts);
	if (r < 0)
	{
		char errmsg[128] = { 0 };
		av_strerror(r, errmsg, sizeof(errmsg));
		printf("[%s] avcodec_open2(%d) => %d, %s.\n", __FUNCTION__, codec->id, r, errmsg);
		avcodec_free_context(&avctx);
		return NULL;
	}

	return avctx;
}

void ffdecoder_destroy(void* p)
{
	AVCodecContext *avctx;
	avctx = (AVCodecContext*)p;
	if (avctx)
	{
		avcodec_close(avctx);
		avcodec_free_context(&avctx);
	}
}

int ffdecoder_input(void* p, const AVPacket* pkt)
{
	int ret;
	AVCodecContext *avctx;
	avctx = (AVCodecContext*)p;

	ret = avcodec_send_packet(avctx, pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
	{
		char errmsg[128] = { 0 };
		av_strerror(ret, errmsg, sizeof(errmsg));
		printf("[%s] avcodec_send_packet(%d) => %d, %s\n", __FUNCTION__, pkt?pkt->size:0, ret, errmsg);
		return ret;
	}
	//if (ret >= 0)
	//	pkt->size = 0;

	return 0;
}

int ffdecoder_getframe(void* p, AVFrame* frame)
{
	int ret;
	AVCodecContext *avctx;
	avctx = (AVCodecContext*)p;

	//memset(frame, 0, sizeof(AVFrame));
	ret = avcodec_receive_frame(avctx, frame);
	if (ret >= 0)
	{
		// got picture
		//assert(AV_PIX_FMT_YUV420P == frame->format || AV_PIX_FMT_YUVJ420P == frame->format);
	}

	//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	//	ret = 0;

	return ret;
}
