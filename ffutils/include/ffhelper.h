#ifndef _ffhelper_h_
#define _ffhelper_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"

#include "avframe.h"
#include "avstream.h"
#include "avpacket.h"
#include "avbuffer.h"
#include <assert.h>

static inline enum AVCodecID avpacket_to_ffmpeg_codecid(enum AVPACKET_CODEC_ID id)
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

	case AVCODEC_AUDIO_AAC:		return AV_CODEC_ID_AAC;
	case AVCODEC_AUDIO_MP3:		return AV_CODEC_ID_MP3;
	case AVCODEC_AUDIO_OPUS:	return AV_CODEC_ID_OPUS;
	case AVCODEC_AUDIO_G729:	return AV_CODEC_ID_G729;
	case AVCODEC_AUDIO_G711A:	return AV_CODEC_ID_PCM_ALAW;
	case AVCODEC_AUDIO_G711U:	return AV_CODEC_ID_PCM_MULAW;
	case AVCODEC_AUDIO_G726:	return AV_CODEC_ID_ADPCM_G726;

	default: assert(0);			return AV_CODEC_ID_NONE;
	}
}

static inline enum AVPACKET_CODEC_ID ffmpeg_to_avpacket_codecid(enum AVCodecID id)
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

static inline enum AVSampleFormat avpacket_to_ffmpeg_audio_format(int format)
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

static inline enum pcm_sample_format ffmpeg_to_avpacket_audio_format(int format)
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

struct avpacket_t* ffmpeg_to_avpacket(AVPacket* ff);
int avpacket_to_ffmpeg(const struct avpacket_t* pkt, int stream_index, AVPacket* ff);

struct avframe_t* ffmpeg_to_avframe(AVFrame* ff);
void avframe_to_ffmpeg(struct avframe_t* frame, AVFrame* ff);

struct avstream_t* ffmpeg_to_avstream(const AVCodecParameters* codecpar);

#ifdef __cplusplus
}
#endif
#endif /* !_ffhelper_h_ */
