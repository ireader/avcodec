
#include "avdecoder.h"
#include "ffdecoder.h"
#include "ffhelper.h"
#include <string.h>
#include <assert.h>

//void* avdecoder_create(int codecId)
//{
//	AVCodecParameters param;
//	memset(&param, 0, sizeof(AVCodecParameters));
//	param.codec_id = codecId;
//	return ffdecoder_create(&param);
//}

void* avdecoder_create_h264(const void* extra, int bytes)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_VIDEO;
	param.codec_id = AV_CODEC_ID_H264;
	param.extradata = (void*)extra;
	param.extradata_size = bytes;
	ff = ffdecoder_create(&param, &opts);
	av_dict_free(&opts);
	return ff;
}

void* avdecoder_create_h265(const void* extra, int bytes)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
	param.codec_type = AVMEDIA_TYPE_VIDEO;
	param.codec_id = AV_CODEC_ID_H265;
	param.extradata = (void*)extra;
	param.extradata_size = bytes;
	ff = ffdecoder_create(&param, &opts);
	av_dict_free(&opts);
	return ff;
}

void* avdecoder_create_aac(const void* extradata, int extradata_size)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
    param.extradata = (void*)extradata;
    param.extradata_size = extradata_size;
	param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_AAC;
	ff = ffdecoder_create(&param, &opts);
	av_dict_free(&opts);
	return ff;
}

static void* avdecoder_create_audio(const struct avpacket_t* pkt)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters codecpar;
	memset(&codecpar, 0, sizeof(AVCodecParameters));
	codecpar.codec_type = AVMEDIA_TYPE_AUDIO;
	codecpar.codec_id = avpacket_to_ffmpeg_codecid(pkt->stream->codecid);
	codecpar.format = AV_SAMPLE_FMT_FLTP;
	codecpar.sample_rate = pkt->stream->sample_rate;
	codecpar.extradata = (uint8_t*)pkt->stream->extra;
	codecpar.extradata_size = pkt->stream->bytes;
#if LIBAVCODEC_VERSION_MAJOR < 59
	codecpar.channels = pkt->stream->channels;
	codecpar.channel_layout = av_get_default_channel_layout(codecpar.channels);
#else
	av_channel_layout_default(&codecpar.ch_layout, pkt->stream->channels);
#endif
	//codecpar.bit_rate = 128000;
	ff = ffdecoder_create(&codecpar, &opts);
	av_dict_free(&opts);
	return ff;
}

static void* avdecoder_create_video(const struct avpacket_t* pkt)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters codecpar;
	memset(&codecpar, 0, sizeof(AVCodecParameters));
	codecpar.codec_type = AVMEDIA_TYPE_VIDEO;
	codecpar.codec_id = avpacket_to_ffmpeg_codecid(pkt->stream->codecid);
	codecpar.extradata = (uint8_t*)pkt->stream->extra;
	codecpar.extradata_size = pkt->stream->bytes;
	ff = ffdecoder_create(&codecpar, &opts);
	av_dict_free(&opts);
	return ff;
}

void* avdecoder_create(const struct avpacket_t* pkt)
{
	switch (avstream_type(pkt->stream))
	{
	case AVSTREAM_AUDIO:
		return avdecoder_create_audio(pkt);
	case AVSTREAM_VIDEO:
		return avdecoder_create_video(pkt);
	default:
		return NULL;
	}
}

void avdecoder_destroy(void* ff)
{
	ffdecoder_destroy(ff);
}

/// @return 0-ok, other-error
int avdecoder_input(void* ff, const struct avpacket_t* pkt)
{
	AVPacket pkt2;
	if (pkt && pkt->data)
		avpacket_to_ffmpeg(pkt, 0, &pkt2);
	return ffdecoder_input(ff, pkt ? &pkt2 : NULL);
}

/// @param[in] frame must be memset to 0 or from av_frame_alloc()
/// @return >=0-got frame, <0-error
int avdecoder_getframe(void* ff, struct avframe_t** frame)
{
	AVFrame ffmpeg;
	memset(&ffmpeg, 0, sizeof(ffmpeg));
	int r = ffdecoder_getframe(ff, &ffmpeg);
	if (r >= 0)
	{
		*frame = ffmpeg_to_avframe(&ffmpeg);
		if (NULL == *frame)
			return -ENOMEM;
	}
	return r;
}

static void* aac_create(int format, int channels, int frequency, const void* extradata, int extradata_size)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
    param.extradata = (void*)extradata;
    param.extradata_size = extradata_size;
    param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_AAC;
    param.channels = channels;
    param.sample_rate = frequency;
	ff = ffdecoder_create(&param, &opts);
	av_dict_free(&opts);
	return ff;
}

static void* mp3_create(int format, int channels, int frequency, const void* extradata, int extradata_size)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
    param.extradata = (void*)extradata;
    param.extradata_size = extradata_size;
    param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_MP3;
	ff = ffdecoder_create(&param, &opts);
	av_dict_free(&opts);
	return ff;
}

void* g726_create(int format, int channels, int frequency, const void* extradata, int extradata_size)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
    param.extradata = (void*)extradata;
    param.extradata_size = extradata_size;
    param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_ADPCM_G726;
	param.sample_rate = 8000;
	param.bits_per_coded_sample = 2;
	param.channels = 1;
	ff = ffdecoder_create(&param, &opts);
	av_dict_free(&opts);
	return ff;
}

void* g729_create(int format, int channels, int frequency, const void* extradata, int extradata_size)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters param;
	memset(&param, 0, sizeof(AVCodecParameters));
    param.extradata = (void*)extradata;
    param.extradata_size = extradata_size;
	param.codec_type = AVMEDIA_TYPE_AUDIO;
	param.codec_id = AV_CODEC_ID_G729;
	param.sample_rate = 8000;
	param.channels = 1;
	ff = ffdecoder_create(&param, &opts);
	av_dict_free(&opts);
	return ff;
}

struct audio_decoder_t* aac_decoder()
{
	static struct audio_decoder_t s_decoder = {
		aac_create,
		avdecoder_destroy,
		avdecoder_input,
		avdecoder_getframe,
	};
	return &s_decoder;
}

struct audio_decoder_t* mp3_decoder()
{
	static struct audio_decoder_t s_decoder = {
		mp3_create,
		avdecoder_destroy,
		avdecoder_input,
		avdecoder_getframe,
	};
	return &s_decoder;
}
