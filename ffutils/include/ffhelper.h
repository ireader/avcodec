#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#ifdef __cplusplus
}
#endif

#include "avframe.h"
#include "avstream.h"
#include "avpacket.h"
#include "avbuffer.h"
#include <assert.h>

inline enum AVCodecID avpacket_to_ffmpeg_codecid(enum AVPACKET_CODEC_ID id)
{
	switch (id)
	{
	case AVCODEC_VIDEO_H264:	return AV_CODEC_ID_H264;
	case AVCODEC_VIDEO_H265:	return AV_CODEC_ID_H265;
	case AVCODEC_VIDEO_MPEG4:	return AV_CODEC_ID_MPEG4;

	case AVCODEC_IMAGE_PNG:		return AV_CODEC_ID_PNG;
	case AVCODEC_IMAGE_BMP:		return AV_CODEC_ID_BMP;
	case AVCODEC_IMAGE_GIF:		return AV_CODEC_ID_GIF;
	case AVCODEC_IMAGE_JPEG:	return AV_CODEC_ID_JPEG2000;

	case AVCODEC_AUDIO_AAC:		return AV_CODEC_ID_AAC_LATM;
	case AVCODEC_AUDIO_MP3:		return AV_CODEC_ID_MP3;
	case AVCODEC_AUDIO_OPUS:	return AV_CODEC_ID_OPUS;
	case AVCODEC_AUDIO_G729:	return AV_CODEC_ID_G729;
	case AVCODEC_AUDIO_G711A:	return AV_CODEC_ID_PCM_ALAW;
	case AVCODEC_AUDIO_G711U:	return AV_CODEC_ID_PCM_MULAW;
	case AVCODEC_AUDIO_G726:	return AV_CODEC_ID_ADPCM_G726;

	default: assert(0);			return AV_CODEC_ID_NONE;
	}
}

inline enum AVPACKET_CODEC_ID ffmpeg_to_avpacket_codecid(enum AVCodecID id)
{
	switch (id)
	{
	case AV_CODEC_ID_H264:		return AVCODEC_VIDEO_H264;
	case AV_CODEC_ID_H265:		return AVCODEC_VIDEO_H265;
	case AV_CODEC_ID_MPEG4:		return AVCODEC_VIDEO_MPEG4;

	case AV_CODEC_ID_PNG:		return AVCODEC_IMAGE_PNG;
	case AV_CODEC_ID_BMP:		return AVCODEC_IMAGE_BMP;
	case AV_CODEC_ID_GIF:		return AVCODEC_IMAGE_GIF;
	case AV_CODEC_ID_JPEG2000:	return AVCODEC_IMAGE_JPEG;

	case AV_CODEC_ID_AAC_LATM:
	case AV_CODEC_ID_AAC:		return AVCODEC_AUDIO_AAC;
	case AV_CODEC_ID_MP3:		return AVCODEC_AUDIO_MP3;
	case AV_CODEC_ID_OPUS:		return AVCODEC_AUDIO_OPUS;
	case AV_CODEC_ID_G729:		return AVCODEC_AUDIO_G729;
	case AV_CODEC_ID_PCM_ALAW:	return AVCODEC_AUDIO_G711A;
	case AV_CODEC_ID_PCM_MULAW:	return AVCODEC_AUDIO_G711U;
	case AV_CODEC_ID_ADPCM_G726:return AVCODEC_AUDIO_G726;

	default: assert(0);			return AVCODEC_NONE;
	}
}

inline enum AVSampleFormat avpacket_to_ffmpeg_audio_format(int format)
{
	switch (format)
	{
	case PCM_SAMPLE_FMT_U8:		return AV_SAMPLE_FMT_U8;
	case PCM_SAMPLE_FMT_S16:	return AV_SAMPLE_FMT_S16;
	case PCM_SAMPLE_FMT_S32:	return AV_SAMPLE_FMT_S32;
	case PCM_SAMPLE_FMT_FLOAT:	return AV_SAMPLE_FMT_FLT;
	case PCM_SAMPLE_FMT_DOUBLE: return AV_SAMPLE_FMT_DBL;
	case PCM_SAMPLE_FMT_S16P:	return AV_SAMPLE_FMT_S16P;
	case PCM_SAMPLE_FMT_S32P:	return AV_SAMPLE_FMT_S32P;
	case PCM_SAMPLE_FMT_FLTP:	return AV_SAMPLE_FMT_FLTP;
	case PCM_SAMPLE_FMT_DBLP:	return AV_SAMPLE_FMT_DBLP;
	default: assert(0);			return AV_SAMPLE_FMT_S16;
	}
}

inline enum pcm_sample_format ffmpeg_to_avpacket_audio_format(int format)
{
	switch (format)
	{
	case AV_SAMPLE_FMT_U8:		return PCM_SAMPLE_FMT_U8;
	case AV_SAMPLE_FMT_S16:		return PCM_SAMPLE_FMT_S16;
	case AV_SAMPLE_FMT_S32:		return PCM_SAMPLE_FMT_S32;
	case AV_SAMPLE_FMT_FLT:		return PCM_SAMPLE_FMT_FLOAT;
	case AV_SAMPLE_FMT_DBL:		return PCM_SAMPLE_FMT_DOUBLE;
	case AV_SAMPLE_FMT_S16P:	return PCM_SAMPLE_FMT_S16P;
	case AV_SAMPLE_FMT_S32P:	return PCM_SAMPLE_FMT_S32P;
	case AV_SAMPLE_FMT_FLTP:	return PCM_SAMPLE_FMT_FLTP;
	case AV_SAMPLE_FMT_DBLP:	return PCM_SAMPLE_FMT_DBLP;
	default: assert(0);			return PCM_SAMPLE_FMT_S16;
	}
}

static inline void avbuffer_free_AVPacket(void* opaque, void* data)
{
	(void)data;
	av_packet_unref((AVPacket*)opaque);
}

inline struct avpacket_t* ffmpeg_to_avpacket(AVPacket* ff)
{
	AVPacket* ref;
	struct avbuffer_t* buf;
	struct avpacket_t* pkt = avpacket_alloc(sizeof(AVPacket));
	if (pkt)
	{
		ref = (AVPacket*)pkt->data;
		buf = (struct avbuffer_t*)pkt->opaque;
		buf->free = avbuffer_free_AVPacket;
		buf->opaque = ref;

		pkt->data = ff->data;
		pkt->size = ff->size;
		pkt->pts = ff->pts;
		pkt->dts = ff->dts;
		//pkt->codecid = ffmpeg_to_avpacket_codecid(codecid);
		pkt->flags = (ff->flags & AV_PKT_FLAG_KEY) ? AVPACKET_FLAG_KEY : 0;

		av_packet_move_ref(ref, ff);
	}

	return pkt;
}

inline int avpacket_to_ffmpeg(const struct avpacket_t* pkt, int stream_index, AVPacket* ff)
{
	av_init_packet(ff);
	ff->data = pkt->data;
	ff->size = pkt->size;
	ff->pts = pkt->pts;
	ff->dts = pkt->dts;
	ff->flags = (ff->flags & AVPACKET_FLAG_KEY) ? AV_PKT_FLAG_KEY : 0;
	ff->stream_index = stream_index;
	return 0;
}

static inline void avbuffer_free_AVFrame(void* opaque, void* data)
{
	(void)data;
	av_frame_unref((AVFrame*)opaque);
}

inline struct avframe_t* ffmpeg_to_avframe(AVFrame* ff)
{
	size_t i;
	AVFrame* ref;
	struct avbuffer_t* buf;
	struct avframe_t* frame = avframe_alloc(sizeof(AVFrame));
	if (frame)
	{
		ref = (AVFrame*)frame->data[0];
		buf = (struct avbuffer_t*)frame->opaque;
		buf->free = avbuffer_free_AVFrame;
		buf->opaque = ref;

		av_frame_move_ref(ref, ff);

		if (AV_PIX_FMT_YUVJ420P == ref->format)
			ref->format = AV_PIX_FMT_YUV420P;
		frame->format = ref->nb_samples > 0 ? ffmpeg_to_avpacket_audio_format(ref->format) : ref->format; //AV_PIX_FMT_YUV420P;
		frame->pts = AV_NOPTS_VALUE == ref->pts ? ref->pkt_dts : ref->pts;
		frame->dts = ref->pkt_dts;
		frame->flags = ref->flags;
		frame->width = ref->width;
		frame->height = ref->height;
		frame->channels = ref->channels;
		frame->samples = ref->nb_samples;
		frame->sample_bits = 8 * av_get_bytes_per_sample((enum AVSampleFormat)ref->format);
		frame->sample_rate = ref->sample_rate;
		for (i = 0; i < sizeof(frame->data) / sizeof(frame->data[0]); i++)
		{
			frame->data[i] = ref->data[i];
			frame->linesize[i] = ref->linesize[i];
		}
	}

	return frame;
}

inline void avframe_to_ffmpeg(struct avframe_t* frame, AVFrame* ff)
{
	size_t i;
	memset(ff, 0, sizeof(AVFrame));
	ff->format = frame->samples > 0 ? avpacket_to_ffmpeg_audio_format(frame->format) : frame->format;
	ff->pts = frame->pts;
	ff->pkt_pts = frame->pts;
	ff->pkt_dts = frame->dts;
	ff->flags = frame->flags;
	ff->width = frame->width;
	ff->height = frame->height;
	ff->channels = frame->channels;
	ff->nb_samples = frame->samples;
	ff->sample_rate = frame->sample_rate;
	for (i = 0; i < sizeof(frame->data) / sizeof(frame->data[0]); i++)
	{
		ff->data[i] = frame->data[i];
		ff->linesize[i] = frame->linesize[i];
	}
}
