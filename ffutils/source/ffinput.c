#include "ffinput.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "libavcodec/bsf.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "ffhelper.h"

struct mpeg4_aac_t
{
	uint8_t profile; // 0-NULL, 1-AAC Main, 2-AAC LC, 2-AAC SSR, 3-AAC LTP

	uint8_t sampling_frequency_index; // 0-96000, 1-88200, 2-64000, 3-48000, 4-44100, 5-32000, 6-24000, 7-22050, 8-16000, 9-12000, 10-11025, 11-8000, 12-7350, 13/14-reserved, 15-frequency is written explictly

	uint8_t channel_configuration; // 0-AOT, 1-1channel,front-center, 2-2channels, front-left/right, 3-3channels: front center/left/right, 4-4channels: front-center/left/right, back-center, 5-5channels: front center/left/right, back-left/right, 6-6channels: front center/left/right, back left/right LFE-channel, 7-8channels
};

struct ffinput_t
{
	AVFormatContext* ic;

	struct mpeg4_aac_t aac;

	// H.264/H.265 only
	AVBSFContext* h264bsf;
	AVBSFContext* h265bsf;
};

static int ffinput_open(struct ffinput_t* ff, const char* url)
{
	int r;
	AVDictionary* opt = NULL;
	ff->ic = avformat_alloc_context();
	if (NULL == ff->ic)
	{
		printf("%s(%s): avformat_alloc_context failed.\n", __FUNCTION__, url);
		return -ENOMEM;
	}

	//if (!av_dict_get(ff->opt, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
	//	av_dict_set(&ff->opt, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
	//	scan_all_pmts_set = 1;
	//}

	av_dict_set(&opt, "timeout", "5000000" /*microsecond*/, 0);
	r = avformat_open_input(&ff->ic, url, NULL, &opt);
	if (0 != r)
	{
		printf("%s: avformat_open_input(%s) => %s\n", __FUNCTION__, url, av_err2str(r));
		return r;
	}

	//if (scan_all_pmts_set)
	//	av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

	//ff->ic->probesize = 100 * 1024;
	//ff->ic->max_analyze_duration = 5 * AV_TIME_BASE;

	/* If not enough info to get the stream parameters, we decode the
	first frames to get it. (used in mpeg case for example) */
	r = avformat_find_stream_info(ff->ic, NULL/*&opt*/);
	if (r < 0) {
		printf("%s(%s): could not find codec parameters: %s\n", __FUNCTION__, url, av_err2str(r));
		return r;
	}

	av_dict_free(&opt);
	return 0;
}

static int mpeg4_aac_audio_specific_config_load(const uint8_t* data, size_t bytes, struct mpeg4_aac_t* aac)
{
	if (bytes < 2) return -1;

	aac->profile = (data[0] >> 3) & 0x1F;
	aac->sampling_frequency_index = ((data[0] & 0x7) << 1) | ((data[1] >> 7) & 0x01);
	aac->channel_configuration = (data[1] >> 3) & 0x0F;
	assert(aac->profile > 0 && aac->profile < 31);
	assert(aac->channel_configuration >= 0 && aac->channel_configuration <= 7);
	assert(aac->sampling_frequency_index >= 0 && aac->sampling_frequency_index <= 0xc);
	return 2;
}

/// @return >=0-adts header length, <0-error
static int mpeg4_aac_adts_save(const struct mpeg4_aac_t* aac, size_t payload, uint8_t* data, size_t bytes)
{
	const uint8_t ID = 0; // 0-MPEG4/1-MPEG2
	size_t len = payload + 7;
	if (bytes < 7 || len >= (1 << 12)) return -1;

	assert(aac->profile > 0 && aac->profile < 31);
	assert(aac->channel_configuration >= 0 && aac->channel_configuration <= 7);
	assert(aac->sampling_frequency_index >= 0 && aac->sampling_frequency_index <= 0xc);
	data[0] = 0xFF; /* 12-syncword */
	data[1] = 0xF0 /* 12-syncword */ | (ID << 3)/*1-ID*/ | (0x00 << 2) /*2-layer*/ | 0x01 /*1-protection_absent*/;
	data[2] = ((aac->profile - 1) << 6) | ((aac->sampling_frequency_index & 0x0F) << 2) | ((aac->channel_configuration >> 2) & 0x01);
	data[3] = ((aac->channel_configuration & 0x03) << 6) | ((len >> 11) & 0x03); /*0-original_copy*/ /*0-home*/ /*0-copyright_identification_bit*/ /*0-copyright_identification_start*/
	data[4] = (uint8_t)(len >> 3);
	data[5] = ((len & 0x07) << 5) | 0x1F;
	data[6] = 0xFC | ((len / 1024) & 0x03);
	return 7;
}

static int ffmpeg_bsf_init(AVBSFContext** ctx, const char* name, const AVCodecParameters* codecpar, const AVRational time_base)
{
	int ret = -1;
	const AVBitStreamFilter *bsf;

	// h264_mp4toannexb
	// hevc_mp4toannexb
	// aac_adtstoasc
	bsf = av_bsf_get_by_name(name);
	if (NULL == bsf)
		return -1;

	ret = av_bsf_alloc(bsf, ctx);
	if (ret < 0)
		return ret;

	ret = avcodec_parameters_copy((*ctx)->par_in, codecpar);
	if (ret < 0)
		return ret;

	(*ctx)->time_base_in = time_base;

	return av_bsf_init(*ctx);
}

static void ffinput_filter_init(struct ffinput_t* ff)
{
	unsigned int i;
	
	for (i = 0; i < ff->ic->nb_streams; i++)
	{
		switch (ff->ic->streams[i]->codecpar->codec_id)
		{
		case AV_CODEC_ID_H264:
			ffmpeg_bsf_init(&ff->h264bsf, "h264_mp4toannexb", ff->ic->streams[i]->codecpar, ff->ic->streams[i]->time_base);
			break;

		case AV_CODEC_ID_H265:
			ffmpeg_bsf_init(&ff->h264bsf, "hevc_mp4toannexb", ff->ic->streams[i]->codecpar, ff->ic->streams[i]->time_base);
			break;

		case AV_CODEC_ID_AAC:
		case AV_CODEC_ID_AAC_LATM:
			mpeg4_aac_audio_specific_config_load(ff->ic->streams[i]->codecpar->extradata, ff->ic->streams[i]->codecpar->extradata_size, &ff->aac);
			break;

		default:
			// do nothing
			break;
		}
	}
}

static int ffinput_filter_input(AVBSFContext* ctx, AVPacket* pkt)
{
	int ret;
	ret = av_bsf_send_packet(ctx, pkt);
	if (ret < 0)
		return ret;

	ret = av_bsf_receive_packet(ctx, pkt);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		return ret; // return 0
	else if (ret < 0)
		return ret;

	return ret;
}

static struct avpacket_t* ffmpeg_aac_to_adts(const struct mpeg4_aac_t* aac, AVPacket* ffmpeg)
{
	struct avpacket_t* p = avpacket_alloc(ffmpeg->size + 7);
	if (p)
	{
		p->size = ffmpeg->size + 7;
		p->pts = ffmpeg->pts;
		p->dts = ffmpeg->dts;
		//p->codecid = AVCODEC_AUDIO_AAC;
		p->flags = (ffmpeg->flags & AV_PKT_FLAG_KEY) ? AVPACKET_FLAG_KEY : 0;
		mpeg4_aac_adts_save(aac, ffmpeg->size, p->data, p->size);
		memcpy(p->data + 7, ffmpeg->data, ffmpeg->size);
	}

	return p;
}

void* ffinput_create(const char* url)
{
	struct ffinput_t* ff;
	ff = (struct ffinput_t*)malloc(sizeof(*ff));
	if (!ff)
		return NULL;

	memset(ff, 0, sizeof(*ff));

	if (0 != ffinput_open(ff, url))
	{
		ffinput_destroy(ff);
		return NULL;
	}

	ffinput_filter_init(ff);
	return ff;
}

void ffinput_destroy(void* p)
{
	struct ffinput_t* ff;
	ff = (struct ffinput_t*)p;

	if (ff->h264bsf)
	{
		av_bsf_free(&ff->h264bsf);
		ff->h264bsf = NULL;
	}

	if (ff->h265bsf)
	{
		av_bsf_free(&ff->h265bsf);
		ff->h265bsf = NULL;
	}

	if (ff->ic)
	{
		avformat_close_input(&ff->ic);
		avformat_free_context(ff->ic);
	}
	free(ff);
}

int ffinput_read(void* p, struct avpacket_t** pkt)
{
	int ret;
	AVPacket ffmpeg;
	struct ffinput_t* ff;
	enum AVCodecID codecid;
	AVRational time_base = { 1, 1000/*ms*/ };

	ff = (struct ffinput_t*)p;
	ret = av_read_frame(ff->ic, &ffmpeg);
	if (ret >= 0)
	{
		assert(0 == ret);
		if ((unsigned int)ffmpeg.stream_index > ff->ic->nb_streams || ffmpeg.stream_index < 0)
		{
			av_packet_unref(&ffmpeg);
			return -1; // invalid stream_index
		}

		ffmpeg.dts = (AV_NOPTS_VALUE == ffmpeg.dts ? ffmpeg.pts : ffmpeg.dts);
		ffmpeg.pts = (AV_NOPTS_VALUE == ffmpeg.pts ? ffmpeg.dts : ffmpeg.pts);
		ffmpeg.dts = av_rescale_q(ffmpeg.dts, ff->ic->streams[ffmpeg.stream_index]->time_base, time_base);
		ffmpeg.pts = av_rescale_q(ffmpeg.pts, ff->ic->streams[ffmpeg.stream_index]->time_base, time_base);

		*pkt = NULL;
		codecid = ff->ic->streams[ffmpeg.stream_index]->codecpar->codec_id;
		switch (codecid)
		{
		case AV_CODEC_ID_H264:
			ret = ffinput_filter_input(ff->h264bsf, &ffmpeg);
			break;

		case AV_CODEC_ID_H265:
			ret = ffinput_filter_input(ff->h265bsf, &ffmpeg);
			break;

		case AV_CODEC_ID_AAC:
		case AV_CODEC_ID_AAC_LATM:
			*pkt = ffmpeg_aac_to_adts(&ff->aac, &ffmpeg);
			ret = 0;
			break;

		default:
			ret = 0;
			break;
		}

		if(ret >= 0 && AV_CODEC_ID_NONE != codecid && AV_CODEC_ID_AAC != codecid && AV_CODEC_ID_AAC_LATM != codecid)
			*pkt = ffmpeg_to_avpacket(&ffmpeg /*, codecid*/ );

		av_packet_unref(&ffmpeg);
		return ret >= 0 ? (*pkt ? 1 : -ENOMEM) : ret;
	}

	return AVERROR_EOF == ret ? 0 : ret;
}
