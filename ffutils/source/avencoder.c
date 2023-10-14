#include "avencoder.h"
#include "ffencoder.h"
#include "ffhelper.h"
#include <string.h>
#include <assert.h>

void* avencoder_create_h264(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters* codecpar;
	codecpar = avcodec_parameters_alloc();
	codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	codecpar->codec_id = AV_CODEC_ID_H264;
	codecpar->format = AV_PIX_FMT_YUV420P;
	codecpar->width = width;
	codecpar->height = height;
	codecpar->bit_rate = bitrate;

	if (tune) av_dict_set(&opts, "tune", tune, 0);
	if (preset) av_dict_set(&opts, "preset", preset, 0);
	if (profile) av_dict_set(&opts, "profile", profile, 0);
	//av_dict_set(&opts, "preset", "fast", 0);
	//av_dict_set(&opts, "crt", "23", 0);
	av_dict_set_int(&opts, "g", gop, 0);
	av_dict_set(&opts, "threads", "4", 0);
	//av_dict_set(&opts, "x264-params", "annexb=0", 0);
	//av_dict_set(&opts, "x264-params", "color_trc=bt709", 0);

	ff = ffencoder_create(codecpar, &opts);
	avcodec_parameters_free(&codecpar);
	av_dict_free(&opts);
	return ff;
}

void* avencoder_create_aac(int sample_rate, int channel, int bitrate)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters* codecpar;
	codecpar = avcodec_parameters_alloc();
	codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	codecpar->codec_id = AV_CODEC_ID_AAC;
	codecpar->format = AV_SAMPLE_FMT_FLTP; // ffmpeg aac, S16-fdk
	codecpar->sample_rate = sample_rate;
#if LIBAVCODEC_VERSION_MAJOR < 59
	codecpar->channels = channel;
	codecpar->channel_layout = av_get_default_channel_layout(channel);
#else
	av_channel_layout_default(&codecpar->ch_layout, channel);
#endif
	codecpar->bit_rate = bitrate;

	ff = ffencoder_create(codecpar, &opts);
	avcodec_parameters_free(&codecpar);
	av_dict_free(&opts);
	return ff;
}

void* avencoder_create_opus(int sample_rate, int channel, int bitrate)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters *codecpar;
	codecpar = avcodec_parameters_alloc();
	codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	codecpar->codec_id = AV_CODEC_ID_OPUS;
#if defined(FFMPEG_OPUS)
	codecpar->format = AV_SAMPLE_FMT_FLTP; // with ffmpeg opus
	av_dict_set(&opts, "strict", "experimental", 0);
#else
	codecpar->format = AV_SAMPLE_FMT_FLT;
#endif
	codecpar->sample_rate = sample_rate;
#if LIBAVCODEC_VERSION_MAJOR < 59
	codecpar->channels = channel;
	codecpar->channel_layout = av_get_default_channel_layout(channel);
#else
	av_channel_layout_default(&codecpar->ch_layout, channel);
#endif
	codecpar->bit_rate = bitrate;

	ff = ffencoder_create(codecpar, &opts);
	avcodec_parameters_free(&codecpar);
	av_dict_free(&opts);
	return ff;
}


void avencoder_destroy(void* ff)
{
	ffencoder_destroy(ff);
}

/// @return 0-ok, other-error
int avencoder_input(void* ff, const struct avframe_t* frame)
{
	AVFrame frame2;
	avframe_to_ffmpeg(frame, &frame2);
	return ffencoder_input(ff, &frame2);
}

/// @param[out] pkt alloc memory internal, use avpacket_release to free memory
/// @return >=0-got packet, <0-error
int avencoder_getpacket(void* ff, struct avpacket_t** pkt)
{
	int r;
	AVPacket pkt2;
	memset(&pkt2, 0, sizeof(pkt2));
	r = ffencoder_getpacket(ff, &pkt2);
	if (r >= 0)
	{
		*pkt = ffmpeg_to_avpacket(&pkt2);
		if (NULL == *pkt)
			return -ENOMEM;
	}
	return r;
}
