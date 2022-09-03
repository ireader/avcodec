#include "ffdecoder.h"
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

void* ffdecoder_create(const AVCodecParameters* codecpar, AVDictionary** opts)
{
	int r;
	const AVCodec* codec = NULL;
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
	//av_dict_set(opts, "threads", "1"/*"auto"*/, 0); // disable multi-thread decode
	r = avcodec_open2(avctx, codec, opts);
	//av_dict_free(opts);
	if (r < 0)
	{
		printf("[%s] avcodec_open2(%d) => %d, %s.\n", __FUNCTION__, codec->id, r, av_err2str(r));
		avcodec_free_context(&avctx);
		return NULL;
	}

	// set packet timebase for the decoder
	avctx->time_base = av_make_q(1, avctx->sample_rate);

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
		printf("[%s] avcodec_send_packet(%d) => %d, %s\n", __FUNCTION__, pkt?pkt->size:0, ret, av_err2str(ret));
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
		frame->time_base = avctx->time_base;
	}

	//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	//	ret = 0;

	return ret;
}

AVCodecParameters* ffdecoder_getcodecpar(void* p)
{
	AVCodecContext* avctx;
	AVCodecParameters* codecpar;
	avctx = (AVCodecContext*)p;

	codecpar = avcodec_parameters_alloc();
	if (!codecpar)
		return NULL;

	if (0 != avcodec_parameters_from_context(codecpar, avctx))
	{
		avcodec_parameters_free(&codecpar);
		return NULL;
	}

	return codecpar;
}
