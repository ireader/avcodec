#include "ffencoder.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/audio_fifo.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct ffencoder_t
{
	AVAudioFifo* fifo;
	AVCodecContext* avctx;
	int64_t pts;
};

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

static int ffencoder_open(struct ffencoder_t* ff, const AVCodecParameters* codecpar, AVDictionary** opts)
{
	int ret;
	const AVCodec* codec = NULL;
	//AVDictionary* opts = NULL;
	AVCodecContext* avctx = NULL;

	codec = avcodec_find_encoder(codecpar->codec_id);
	if (NULL == codec)
	{
		printf("[%s] avcodec_find_encoder(%d) not found.\n", __FUNCTION__, codecpar->codec_id);
		return -1;
	}

	avctx = avcodec_alloc_context3(codec);
	if (NULL == avctx)
		return -1;

	ret = avcodec_parameters_to_context(avctx, codecpar);
	if (ret < 0)
	{
		avcodec_free_context(&avctx);
		return ret;
	}

	avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		avctx->time_base = av_make_q(1, 90000); // 90kHZ
		//avctx->max_b_frames = 1;
		//avctx->thread_count = 4;
		//avctx->gop_size = 25;
		//av_dict_set(&opts, "preset", "fast", 0);
		//av_dict_set(&opts, "crt", "23", 0);
	}
	else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
	{
		avctx->time_base = av_make_q(1, codecpar->sample_rate);
	}

	ret = avcodec_open2(avctx, codec, opts);
//	av_dict_free(opts);
	if (ret < 0)
	{
		printf("[%s] avcodec_open2(%d) => %s.\n", __FUNCTION__, codecpar->codec_id, av_err2str(ret));
		avcodec_free_context(&avctx);
		return ret;
	}

	//avcodec_parameters_from_context(codecpar, avctx);
	ff->avctx = avctx;
	return 0;
}

void* ffencoder_create(const AVCodecParameters* codecpar, AVDictionary** opts)
{
	struct ffencoder_t* ff;
	ff = (struct ffencoder_t*)malloc(sizeof(*ff));
	if (!ff)
		return NULL;

	memset(ff, 0, sizeof(*ff));

	if (0 != ffencoder_open(ff, codecpar, opts))
	{
		ffencoder_destroy(ff);
		return NULL;
	}

	if (AVMEDIA_TYPE_AUDIO == codecpar->codec_type && ff->avctx->frame_size != 0)
	{
#if LIBAVCODEC_VERSION_MAJOR < 59
		ff->fifo = av_audio_fifo_alloc(codecpar->format, codecpar->channels, codecpar->sample_rate / 2); // 500ms
#else
		ff->fifo = av_audio_fifo_alloc(codecpar->format, codecpar->ch_layout.nb_channels, codecpar->sample_rate / 2); // 500ms
#endif
		if (!ff->fifo)
		{
			ffencoder_destroy(ff);
			return NULL;
		}
	}

	return ff;
}

void ffencoder_destroy(void* p)
{
	struct ffencoder_t* ff;
	ff = (struct ffencoder_t*)p;
	avcodec_free_context(&ff->avctx);
	if(ff->fifo)
		av_audio_fifo_free(ff->fifo);
	free(ff);
}

static int ffencoder_encode_audio(struct ffencoder_t* ff)
{
	int r;
	AVFrame* audio;
	audio = av_frame_alloc();
	audio->nb_samples = ff->avctx->frame_size;
	audio->sample_rate = ff->avctx->sample_rate;
	audio->format = ff->avctx->sample_fmt;
#if LIBAVCODEC_VERSION_MAJOR < 59
	audio->channels = ff->avctx->channels;
	audio->channel_layout = ff->avctx->channel_layout;
#else
	av_channel_layout_copy(&audio->ch_layout, &ff->avctx->ch_layout);
#endif
	r = av_frame_get_buffer(audio, 0);
	if (r < 0)
	{
		av_frame_free(&audio);
		printf("[%s] av_frame_get_buffer() => %s\n", __FUNCTION__, av_err2str(r));
		return r;
	}

	if (av_audio_fifo_read(ff->fifo, audio->data, ff->avctx->frame_size) != ff->avctx->frame_size)
	{
		av_frame_free(&audio);
		return -1; // error
	}

	ff->pts += audio->nb_samples;
	audio->pts = audio->pkt_dts = ff->pts; // frame->pkt_dts + frame->nb_samples - av_audio_fifo_size(ff->fifo) /* - ff->avctx->frame_size */;
	audio->time_base = ff->avctx->time_base;

	r = avcodec_send_frame(ff->avctx, audio);
	av_frame_free(&audio);
	return r;
}

int ffencoder_input(void* p, const AVFrame* frame)
{
	int ret;
	struct ffencoder_t* ff;
	ff = (struct ffencoder_t*)p;

	if (0 == ff->pts)
		ff->pts = frame->pts;

	if (AVMEDIA_TYPE_AUDIO == ff->avctx->codec_type && ff->fifo)
	{
		ret = av_audio_fifo_realloc(ff->fifo, av_audio_fifo_size(ff->fifo) + frame->nb_samples);
		if (ret < 0)
		{
			printf("[%s] av_audio_fifo_realloc(%d, %d) => %s\n", __FUNCTION__, av_audio_fifo_size(ff->fifo), frame->nb_samples, av_err2str(ret));
			return ret;
		}

		ret = av_audio_fifo_write(ff->fifo, (void**)frame->data, frame->nb_samples);
		if (ret < 0)
		{
			printf("[%s] av_audio_fifo_write(%d) => %s\n", __FUNCTION__, frame->nb_samples, av_err2str(ret));
			return ret;
		}
		else if (ret != frame->nb_samples)
		{
			printf("[%s] av_audio_fifo_write() buffer full %d != %d\n", __FUNCTION__, ret, frame->nb_samples);
			return ret;
		}

		while (av_audio_fifo_size(ff->fifo) >= ff->avctx->frame_size)
		{
			ret = ffencoder_encode_audio(ff);
			if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			{
				printf("[%s] avcodec_send_frame() => %s\n", __FUNCTION__, av_err2str(ret));
				return ret;
			}
			else if (ret >= 0)
			{
				return ret; // got a encoded packet
			}
		}
	}
	else
	{
		ret = avcodec_send_frame(ff->avctx, frame);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		{
			printf("[%s] avcodec_send_frame() => %s\n", __FUNCTION__, av_err2str(ret));
			return ret;
		}
		//if (ret >= 0)
		//	pkt.size = 0;
	}

	return 0;
}

int ffencoder_getpacket(void* p, AVPacket* pkt)
{
	int ret;
	struct ffencoder_t* ff;
	ff = (struct ffencoder_t*)p;

	ret = avcodec_receive_packet(ff->avctx, pkt);
	if (ret >= 0)
	{
		// ok
		pkt->time_base = ff->avctx->time_base;

		// drain buffer
		if(ff->fifo && av_audio_fifo_size(ff->fifo) >= ff->avctx->frame_size)
			ffencoder_encode_audio(ff);
	}

	//if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	//	ret = 0;

	return ret;
}

AVCodecParameters* ffencoder_getcodecpar(void* p)
{
	struct ffencoder_t* ff;
	AVCodecParameters* codecpar;
	ff = (struct ffencoder_t*)p;
	
	codecpar = avcodec_parameters_alloc();
	if (!codecpar)
		return NULL;
	
	if (0 != avcodec_parameters_from_context(codecpar, ff->avctx))
	{
		avcodec_parameters_free(&codecpar);
		return NULL;
	}

	return codecpar;
}
