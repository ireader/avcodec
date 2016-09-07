#include "ffencoder.h"
#include "libavcodec/avcodec.h"
#include <stdio.h>
#include <assert.h>
#include <memory.h>

struct ffencoder_t
{
	AVCodecContext *avctx;
//	AVFrame *frame;
	AVPacket pkt;
};

void ffencoder_init(void)
{
	avcodec_register_all();
}

void ffencoder_clean(void)
{
}

static void ffencoder_setparameter(struct ffencoder_t* ff, h264_parameter_t* param)
{
	ff->avctx->time_base = av_make_q(1000, param->frame_rate); // fps
	ff->avctx->gop_size = param->gop_size;
	ff->avctx->max_b_frames = 1;
	ff->avctx->thread_count = 1;
	ff->avctx->pix_fmt = param->format;		
	ff->avctx->width = param->width;
	ff->avctx->height = param->height;
	ff->avctx->bit_rate = param->bitrate;
}

void* ffencoder_create(h264_parameter_t* param)
{
	int ret;
	AVCodec* codec = NULL;
	AVDictionary *opts = NULL;
	struct ffencoder_t* ff = NULL;
	ff = (struct ffencoder_t*)malloc(sizeof(*ff));
	if (NULL == ff)
		return NULL;
	memset(ff, 0, sizeof(*ff));

	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (NULL == codec)
	{
		printf("[%s] avcodec_find_encoder(%d) not found.\n", __FUNCTION__, AV_CODEC_ID_H264);
		ffencoder_destroy(ff);
		return NULL;
	}

//	ff->frame = av_frame_alloc();
	ff->avctx = avcodec_alloc_context3(codec);
	ffencoder_setparameter(ff, param);

	ret = avcodec_open2(ff->avctx, codec, &opts);
	av_dict_free(&opts);
	if (ret < 0)
	{
		printf("[%s] avcodec_open2(%d) => %d.\n", __FUNCTION__, AV_CODEC_ID_H264, ret);
		ffencoder_destroy(ff);
		return NULL;
	}

	return ff;
}

void ffencoder_destroy(void* p)
{
	struct ffencoder_t* ff;
	ff = (struct ffencoder_t*)p;
//	av_frame_free(&ff->frame);
	av_packet_unref(&ff->pkt);
	avcodec_free_context(&ff->avctx);
	free(ff);
}

int ffencoder_input(void* p, const picture_t* pic)
{
	int i, ret;
	AVFrame frame;
	struct ffencoder_t* ff;
	ff = (struct ffencoder_t*)p;

	assert(PICTURE_YUV420 == pic->format);
	av_frame_unref(&frame);
	frame.pts = pic->pts;
	frame.width = pic->width;
	frame.height = pic->height;
	frame.format = AV_PIX_FMT_YUV420P; // pic->format;
	for (i = 0; i < PICTURE_PLANAR_NUM; i++)
	{
		frame.data[i] = pic->data[i];
		frame.linesize[i] = pic->linesize[i];
	}

	ret = avcodec_send_frame(ff->avctx, &frame);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
	{
		printf("[%s] avcodec_send_frame() => %d\n", __FUNCTION__, ret);
		return ret;
	}
	//if (ret >= 0)
	//	pkt.size = 0;

	return 0;
}

int ffencoder_getpacket(void* p, avpacket_t* pkt)
{
	int ret;
	struct ffencoder_t* ff;
	ff = (struct ffencoder_t*)p;
	
	ret = avcodec_receive_packet(ff->avctx, &ff->pkt);
	if (ret >= 0)
	{
		// ok
		pkt->data = ff->pkt.data;
		pkt->bytes = ff->pkt.size;
		pkt->flags = ff->pkt.flags;

		pkt->pts = ff->pkt.pts;
		pkt->dts = ff->pkt.dts;
	}

	//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	//	ret = 0;

	return ret;
}
