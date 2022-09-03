#include <Windows.h>
#include <iostream>
#include "ffinput.h"
#include "ffoutput.h"
#include "fftranscode.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
}

struct audio_transcode_test_t
{
	AVFormatContext* ic;
	int audio;
	AVStream* s;

	void* transcode;
	void* output;
};

static int audio_transcode_open(audio_transcode_test_t* ctx, const char* url)
{
	int r;
	AVDictionary* opt = NULL;
	ctx->ic = avformat_alloc_context();
	if (NULL == ctx->ic)
	{
		printf("%s(%s): avformat_alloc_context failed.\n", __FUNCTION__, url);
		return -ENOMEM;
	}

	//if (!av_dict_get(ff->opt, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
	//	av_dict_set(&ff->opt, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
	//	scan_all_pmts_set = 1;
	//}

	av_dict_set(&opt, "timeout", "5000000" /*microsecond*/, 0);
	r = avformat_open_input(&ctx->ic, url, NULL, &opt);
	if (0 != r)
	{
		//printf("%s: avformat_open_input(%s) => %s\n", __FUNCTION__, url, av_err2str(r));
		return r;
	}

	//if (scan_all_pmts_set)
	//	av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

	//ff->ic->probesize = 100 * 1024;
	//ff->ic->max_analyze_duration = 5 * AV_TIME_BASE;

	/* If not enough info to get the stream parameters, we decode the
	first frames to get it. (used in mpeg case for example) */
	r = avformat_find_stream_info(ctx->ic, NULL/*&opt*/);
	if (r < 0) {
		//printf("%s(%s): could not find codec parameters: %s\n", __FUNCTION__, url, av_err2str(r));
		return r;
	}

	av_dict_free(&opt);

	for (int i = 0; i < ctx->ic->nb_streams; i++)
	{
		if (ctx->ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			ctx->audio = ctx->ic->streams[i]->index;
			AVCodecParameters* srcpar = ctx->ic->streams[i]->codecpar;
			ctx->transcode = fftranscode_create_opus(srcpar, 48000, 2, 128000);
			break;
		}
	}

	AVDictionary* fmtOpts = NULL;
	AVCodecParameters *tgtpar = fftranscode_getcodecpar(ctx->transcode);
	ctx->output = ffoutput_create("1.mkv", "matroska", &fmtOpts);
	ctx->s = ffoutput_addstream(ctx->output, NULL, tgtpar);
	ctx->s->time_base = av_make_q(1, tgtpar->sample_rate);
	avcodec_parameters_free(&tgtpar);
	av_dict_free(&fmtOpts);

	if (!ctx->transcode || !ctx->output)
		return -1;

	return 0;
}

static int audio_transcode_close(audio_transcode_test_t* ctx)
{
	if (ctx->ic)
	{
		if (ctx->ic->pb)
			avio_closep(&ctx->ic->pb);
		avformat_free_context(ctx->ic);
	}

	if (ctx->transcode)
		fftranscode_destroy(ctx->transcode);

	if (ctx->output)
		ffoutput_destroy(ctx->output);

	return 0;
}

static int audio_transcode_process(audio_transcode_test_t* ctx, AVPacket* pkt)
{
	int r = fftranscode_input(ctx->transcode, pkt);
	if (r < 0)
		return r;
	
	AVPacket out;
	memset(&out, 0, sizeof(out));
	r = fftranscode_getpacket(ctx->transcode, &out);
	while (r >= 0)
	{
		out.stream_index = ctx->s->index;
		r = ffoutput_write(ctx->output, &out);
		av_packet_unref(&out);
		if (r < 0)
			return r;

		memset(&out, 0, sizeof(out));
		r = fftranscode_getpacket(ctx->transcode, &out);
	}

	return 0;
}

void audio_transcode_test(const char* url)
{
	struct audio_transcode_test_t ctx;
	int r = audio_transcode_open(&ctx, url);

	AVPacket pkt;
	r = av_read_frame(ctx.ic, &pkt);
	while (r >= 0)
	{
		if(ctx.audio == pkt.stream_index)
		{
			pkt.time_base = ctx.ic->streams[pkt.stream_index]->time_base;
			r = audio_transcode_process(&ctx, &pkt);
		}

		av_packet_unref(&pkt);

		r = av_read_frame(ctx.ic, &pkt);
	}

	audio_transcode_close(&ctx);
}
