#include "ffdecoder.h"
#include "libavcodec/avcodec.h"
#include <stdio.h>
#include <assert.h>
#include <memory.h>

struct ffdecoder_t
{
	AVCodecContext *avctx;
	AVFrame *frame;
	AVPacket pkt;
};

void ffdecoder_init(void)
{
	avcodec_register_all();
}

void ffdecoder_clean(void)
{
}

void* ffdecoder_create()
{
	int ret;
	AVCodec* codec = NULL;
	AVDictionary *opts = NULL;
	struct ffdecoder_t* ff = NULL;
	ff = (struct ffdecoder_t*)malloc(sizeof(*ff));
	if (NULL == ff)
		return NULL;
	memset(ff, 0, sizeof(*ff));

	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (NULL == codec)
	{
		printf("[%s] avcodec_find_decoder(%d) not found.\n", __FUNCTION__, AV_CODEC_ID_H264);
		ffdecoder_destroy(ff);
		return NULL;
	}

	ff->frame = av_frame_alloc();
	ff->avctx = avcodec_alloc_context3(codec);

	av_dict_set(&opts, "threads", "1"/*"auto"*/, 0); // disable multi-thread decode
	ret = avcodec_open2(ff->avctx, codec, &opts);
	av_dict_free(&opts);
	if (ret < 0)
	{
		printf("[%s] avcodec_open2(%d) => %d.\n", __FUNCTION__, AV_CODEC_ID_H264, ret);
		ffdecoder_destroy(ff);
		return NULL;
	}

	return ff;
}

void ffdecoder_destroy(void* p)
{
	struct ffdecoder_t* ff;
	ff = (struct ffdecoder_t*)p;
	av_frame_free(&ff->frame);
	av_packet_unref(&ff->pkt);
	avcodec_free_context(&ff->avctx);
	free(ff);
}

int ffdecoder_input(void* p, const avpacket_t* pkt)
{
	int ret;
	struct ffdecoder_t* ff;
	ff = (struct ffdecoder_t*)p;

	av_init_packet(&ff->pkt);
	ff->pkt.data = pkt->data;
	ff->pkt.size = pkt->bytes;
	ff->pkt.pts = pkt->pts;
	ff->pkt.dts = pkt->dts;
	ret = avcodec_send_packet(ff->avctx, &ff->pkt);
	if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
	{
		printf("[%s] avcodec_send_packet(%u) => %d\n", __FUNCTION__, (unsigned int)pkt->bytes, ret);
		return ret;
	}
	//if (ret >= 0)
	//	pkt.size = 0;

	return 0;
}

int ffdecoder_getpicture(void* p, picture_t* pic)
{
	int i, ret;
	struct ffdecoder_t* ff;
	ff = (struct ffdecoder_t*)p;
	ret = avcodec_receive_frame(ff->avctx, ff->frame);
	if (ret >= 0)
	{
		// got picture
		assert(AV_PIX_FMT_YUV420P == ff->frame->format || AV_PIX_FMT_YUVJ420P == ff->frame->format);
		pic->pts = ff->frame->pkt_pts;
		pic->dts = ff->frame->pkt_dts;
		pic->width = ff->frame->width;
		pic->height = ff->frame->height;
		pic->flags = ff->frame->flags;
		pic->format = PICTURE_YUV420;
		for (i = 0; i < PICTURE_PLANAR_NUM; i++)
		{
			pic->linesize[i] = ff->frame->linesize[i];
			pic->data[i] = ff->frame->data[i];
		}
	}

	//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	//	ret = 0;

	return ret;
}
