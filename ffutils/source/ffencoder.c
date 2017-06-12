#include "libavcodec/avcodec.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

//static void ffencoder_setparameter(struct ffencoder_t* ff, h264_parameter_t* param)
//{
//	ff->avctx->time_base = av_make_q(1000, param->frame_rate); // fps
//	ff->avctx->gop_size = param->gop_size;
//	ff->avctx->max_b_frames = 1;
//	ff->avctx->thread_count = 1;
//	ff->avctx->pix_fmt = param->format;		
//	ff->avctx->width = param->width;
//	ff->avctx->height = param->height;
//	ff->avctx->bit_rate = param->bitrate;
//	//ff->avctx->bit_rate_tolerance = CBR;
//	//ff->avctx->refs = 1;
//	//ff->avctx->slices = 1;
//	//ff->avctx->ticks_per_frame = 2; // Set to time_base ticks per frame. Default 1, e.g., H.264/MPEG-2 set it to 2.
//}

void ffencoder_destroy(void* p)
{
	AVCodecContext* avctx;
	avctx = (AVCodecContext*)p;
	avcodec_free_context(&avctx);
}

void* ffencoder_create(AVCodecParameters* codecpar)
{
	int ret;
	AVCodec* codec = NULL;
	AVDictionary *opts = NULL;
	AVCodecContext* avctx = NULL;

	codec = avcodec_find_encoder(codecpar->codec_id);
	if (NULL == codec)
	{
		printf("[%s] avcodec_find_encoder(%d) not found.\n", __FUNCTION__, codecpar->codec_id);
		return NULL;
	}

	avctx = avcodec_alloc_context3(codec);
	if (NULL == avctx)
		return NULL;

	ret = avcodec_parameters_to_context(avctx, codecpar);
	if (ret < 0)
	{
		avcodec_free_context(&avctx);
		return NULL;
	}

	ret = avcodec_open2(avctx, codec, &opts);
	av_dict_free(&opts);
	if (ret < 0)
	{
		printf("[%s] avcodec_open2(%d) => %d.\n", __FUNCTION__, codecpar->codec_id, ret);
		avcodec_free_context(&avctx);
		return NULL;
	}

	avcodec_parameters_from_context(codecpar, avctx);
	return avctx;
}

int ffencoder_input(void* p, const AVFrame* frame)
{
	int ret;
	AVCodecContext *avctx;
	avctx = (AVCodecContext*)p;

	ret = avcodec_send_frame(avctx, frame);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
	{
		printf("[%s] avcodec_send_frame() => %d\n", __FUNCTION__, ret);
		return ret;
	}
	//if (ret >= 0)
	//	pkt.size = 0;

	return 0;
}

int ffencoder_getpacket(void* p, AVPacket* pkt)
{
	int ret;
	AVCodecContext *avctx;
	avctx = (AVCodecContext*)p;

	ret = avcodec_receive_packet(avctx, pkt);
	if (ret >= 0)
	{
		// ok
	}

	//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	//	ret = 0;

	return ret;
}
