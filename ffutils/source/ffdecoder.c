#include "ffdecoder.h"
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <memory.h>

struct ffdecoder_t
{
	AVCodecContext *avctx;
};

static int ffdecoder_open(struct ffdecoder_t* ff, const AVCodecParameters* codecpar)
{
	int r;
	AVCodec* codec = NULL;
	AVDictionary *opts = NULL;

	ff->avctx = avcodec_alloc_context3(NULL);
	if (!ff->avctx)
		return ENOMEM;

	r = avcodec_parameters_to_context(ff->avctx, codecpar);
	if (r < 0)
		return r;

	//av_codec_set_pkt_timebase(ff->avctx, ic->streams[stream_index]->time_base);
	codec = avcodec_find_decoder(ff->avctx->codec_id);
	if (NULL == codec)
	{
		printf("[%s] avcodec_find_decoder(%d) not found.\n", __FUNCTION__, ff->avctx->codec_id);
		return -1;
	}

	//ff->avctx->codec_id = codec->id;
	assert(ff->avctx->codec_id == codec->id);
	av_dict_set(&opts, "threads", "1"/*"auto"*/, 0); // disable multi-thread decode
	r = avcodec_open2(ff->avctx, codec, &opts);
	av_dict_free(&opts);
	if (r < 0)
	{
		char errmsg[128] = { 0 };
		av_strerror(r, errmsg, sizeof(errmsg));
		printf("[%s] avcodec_open2(%d) => %d, %s.\n", __FUNCTION__, codec->id, r, errmsg);
		return r;
	}

	return 0;
}

void* ffdecoder_create(const AVCodecParameters* codecpar)
{
	struct ffdecoder_t* ff = NULL;
	ff = (struct ffdecoder_t*)malloc(sizeof(*ff));
	if (NULL == ff)
		return NULL;
	memset(ff, 0, sizeof(*ff));

	if (0 != ffdecoder_open(ff, codecpar))
	{
		ffdecoder_destroy(ff);
		return NULL;
	}

	return ff;
}

void ffdecoder_destroy(void* p)
{
	struct ffdecoder_t* ff;
	ff = (struct ffdecoder_t*)p;
	if (ff->avctx)
	{
		avcodec_close(ff->avctx);
		avcodec_free_context(&ff->avctx);
	}
	free(ff);
}

int ffdecoder_input(void* p, const AVPacket* pkt)
{
	int ret;
	struct ffdecoder_t* ff;
	ff = (struct ffdecoder_t*)p;

	ret = avcodec_send_packet(ff->avctx, pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
	{
		char errmsg[128] = { 0 };
		av_strerror(ret, errmsg, sizeof(errmsg));
		printf("[%s] avcodec_send_packet(%d) => %d, %s\n", __FUNCTION__, pkt->size, ret, errmsg);
		return ret;
	}
	//if (ret >= 0)
	//	pkt->size = 0;

	return 0;
}

int ffdecoder_getframe(void* p, AVFrame* frame)
{
	int ret;
	struct ffdecoder_t* ff;
	ff = (struct ffdecoder_t*)p;

	//memset(frame, 0, sizeof(AVFrame));
	ret = avcodec_receive_frame(ff->avctx, frame);
	if (ret >= 0)
	{
		// got picture
		//assert(AV_PIX_FMT_YUV420P == frame->format || AV_PIX_FMT_YUVJ420P == frame->format);
	}

	//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	//	ret = 0;

	return ret;
}
