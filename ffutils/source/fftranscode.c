#include "fftranscode.h"
#include "ffencoder.h"
#include "ffdecoder.h"
#include "ffresample.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"

struct fftranscode_t
{
	void* encoder;
	void* decoder;
	void* resampler;
	AVCodecParameters *encpar;
	AVCodecParameters *decpar;
};

void* fftranscode_create(const AVCodecParameters* decode, const AVCodecParameters* encode, AVDictionary** opts)
{
	AVDictionary* decopts = NULL;
	struct fftranscode_t* fft;
	fft = (struct fftranscode_t*)calloc(1, sizeof(*fft));
	if (!fft)
		return NULL;

	fft->decoder = ffdecoder_create(decode, &decopts);
	fft->encoder = ffencoder_create(encode, opts);
	if (!fft->encoder || !fft->decoder)
	{
		fftranscode_destroy(fft);
		return NULL;
	}
	
	fft->encpar = ffencoder_getcodecpar(fft->encoder);
	fft->decpar = ffdecoder_getcodecpar(fft->decoder);
	if (!fft->encpar || !fft->decpar)
	{
		fftranscode_destroy(fft);
		return NULL;
	}
	av_dict_free(&decopts);

#if LIBAVCODEC_VERSION_MAJOR < 59
	fft->resampler = ffresample_create((enum AVSampleFormat)fft->decpar->format, fft->decpar->sample_rate, fft->decpar->channels, (enum AVSampleFormat)fft->encpar->format, fft->encpar->sample_rate, fft->encpar->channels);
#else
	fft->resampler = ffresample_create((enum AVSampleFormat)(fft->decpar->format), fft->decpar->sample_rate, fft->decpar->ch_layout.nb_channels, (enum AVSampleFormat)fft->encpar->format, fft->encpar->sample_rate, fft->encpar->ch_layout.nb_channels);
#endif
	if (!fft->resampler)
	{
		fftranscode_destroy(fft);
		return NULL;
	}

	return fft;
}

int fftranscode_destroy(void* transcode)
{
	struct fftranscode_t* fft;
	fft = (struct fftranscode_t*)transcode;

	if (fft && fft->encoder)
	{
		ffencoder_destroy(fft->encoder);
		fft->encoder = NULL;
	}

	if (fft && fft->decoder)
	{
		ffdecoder_destroy(fft->decoder);
		fft->decoder = NULL;
	}

	if (fft && fft->resampler)
	{
		ffresample_destroy(fft->resampler);
		fft->resampler = NULL;
	}

	if (fft && fft->encpar)
		avcodec_parameters_free(&fft->encpar);
	if (fft && fft->decpar)
		avcodec_parameters_free(&fft->decpar);

	if (fft)
		free(fft);
	return 0;
}

AVCodecParameters* fftranscode_getcodecpar(void* transcode)
{
	struct fftranscode_t* fft;
	fft = (struct fftranscode_t*)transcode;
	return ffencoder_getcodecpar(fft->encoder);
}

static int fftranscode_resample(struct fftranscode_t* fft, const AVFrame* in, AVFrame* out)
{
	int r;
	//uint8_t** samples;

	//r = av_samples_alloc_array_and_samples(&samples, NULL, fft->encpar.ch_layout.nb_channels, in->nb_samples, fft->encpar.format, 0);
	//if (r < 0)
	//	return r;

	//r = ffresample_convert(fft->resampler, (const uint8_t**)in->extended_data, in->nb_samples, samples, in->nb_samples);
	//if (r >= 0)
	//	out->nb_samples = r;

	//if (samples) {
	//	av_freep(&samples[0]);
	//	free(samples);
	//}

	out->format = fft->encpar->format;
#if LIBAVCODEC_VERSION_MAJOR < 59
	out->channels = fft->encpar.channels;
	out->channel_layout = av_get_default_channel_layout(fft->encpar.channels);
#else
	av_channel_layout_copy(&out->ch_layout, &fft->encpar->ch_layout);
#endif
	out->sample_rate = fft->encpar->sample_rate;
	out->time_base = av_make_q(1, fft->encpar->sample_rate);
	out->nb_samples = (int)av_rescale_q_rnd(in->nb_samples, in->time_base, out->time_base, AV_ROUND_UP);
	out->pkt_dts = av_rescale_q(in->pkt_dts, in->time_base, out->time_base);
	out->pts = out->pkt_dts;
	av_frame_get_buffer(out, 0);
	//av_frame_make_writable(frame);

	r = ffresample_convert(fft->resampler, (const uint8_t**)in->extended_data, in->nb_samples, out->extended_data, out->nb_samples);
	if (r < 0)
	{
		av_frame_unref(out);
		return r;
	}
	
	out->nb_samples = r;
	return r;
}

int fftranscode_input(void* transcode, const AVPacket* in)
{
	AVFrame decode, encode;
	struct fftranscode_t* fft;
	fft = (struct fftranscode_t*)transcode;

	int r = ffdecoder_input(fft->decoder, in);
	if (r < 0)
		return r;

	memset(&decode, 0, sizeof(decode));
	r = ffdecoder_getframe(fft->decoder, &decode);
	if (r < 0)
		return r;

	memset(&encode, 0, sizeof(encode));
	r = fftranscode_resample(fft, &decode, &encode);
	av_frame_unref(&decode);
	if (r < 0)
		return r;

	r = ffencoder_input(fft->encoder, &encode);
	av_frame_unref(&encode);
	return r;
}

int fftranscode_getpacket(void* transcode, AVPacket * out)
{
	struct fftranscode_t* fft;
	fft = (struct fftranscode_t*)transcode;
	return ffencoder_getpacket(fft->encoder, out);
}

void* fftranscode_create_opus(const AVCodecParameters* decode, int sample_rate, int channel, int bitrate)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters codecpar;

	memset(&codecpar, 0, sizeof(AVCodecParameters));
	codecpar.codec_type = AVMEDIA_TYPE_AUDIO;
	codecpar.codec_id = AV_CODEC_ID_OPUS;
#if defined(FFMPEG_OPUS)
	codecpar.format = AV_SAMPLE_FMT_FLTP; // with ffmpeg opus
	av_dict_set(&opts, "strict", "experimental", 0);
#else
	codecpar.format = AV_SAMPLE_FMT_FLT;
#endif
	codecpar.sample_rate = sample_rate;
#if LIBAVCODEC_VERSION_MAJOR < 59
	codecpar.channels = channel;
	codecpar.channel_layout = av_get_default_channel_layout(channel);
#else
	av_channel_layout_default(&codecpar.ch_layout, channel);
#endif
	codecpar.bit_rate = bitrate;

	ff = fftranscode_create(decode, &codecpar, &opts);
	av_dict_free(&opts);
	return ff;
}

void* fftranscode_create_aac(const AVCodecParameters* decode, int sample_rate, int channel, int bitrate)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters codecpar;

	memset(&codecpar, 0, sizeof(AVCodecParameters));
	codecpar.codec_type = AVMEDIA_TYPE_AUDIO;
	codecpar.codec_id = AV_CODEC_ID_AAC;
	codecpar.format = AV_SAMPLE_FMT_FLTP; // ffmpeg aac, S16-fdk
	codecpar.sample_rate = sample_rate;
#if LIBAVCODEC_VERSION_MAJOR < 59
	codecpar.channels = channel;
	codecpar.channel_layout = av_get_default_channel_layout(channel);
#else
	av_channel_layout_default(&codecpar.ch_layout, channel);
#endif
	codecpar.bit_rate = bitrate;

	ff = fftranscode_create(decode, &codecpar, &opts);
	av_dict_free(&opts);
	return ff;
}

void* fftranscode_create_h264(const AVCodecParameters* decode, const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters codecpar;

	memset(&codecpar, 0, sizeof(AVCodecParameters));
	codecpar.codec_type = AVMEDIA_TYPE_AUDIO;
	codecpar.codec_id = AV_CODEC_ID_H264;
	codecpar.format = AV_PIX_FMT_YUV420P;
	codecpar.width = width;
	codecpar.height = height;
	codecpar.bit_rate = bitrate;

	if (tune) av_dict_set(&opts, "tune", tune, 0);
	if (preset) av_dict_set(&opts, "preset", preset, 0);
	if (profile) av_dict_set(&opts, "profile", profile, 0);
	//av_dict_set(&opts, "preset", "fast", 0);
	//av_dict_set(&opts, "crt", "23", 0);
	av_dict_set_int(&opts, "g", gop, 0);

	ff = fftranscode_create(decode, &codecpar, &opts);
	av_dict_free(&opts);
	return ff;
}
