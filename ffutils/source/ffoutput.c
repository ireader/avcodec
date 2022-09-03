#include "ffoutput.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "libavformat/avformat.h"

struct ffoutput_t
{
	int header;
	AVFormatContext* oc;
	struct AVIOInterruptCB interrupt_callback;
};

static int ffoutput_interrupt(void* p)
{
	struct ffoutput_t* ff;
	ff = (struct ffoutput_t*)p;
	printf("ffoutput_interrupt: %p\n", ff->oc);
	return 0;
}

//int ffoutput_add_video_stream(void* p, enum AVCodecID codecId)
//{
//	AVStream* st ;
//	struct ffoutput_t* ff;
//	ff = (struct ffoutput_t*)p;
//
//	st = avformat_new_stream(ff->oc, NULL);
//	st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
//	st->codecpar->codec_id = codecId;
//	//st->codecpar->bit_rate = 0;
//	//st->codecpar->width = 0;
//	//st->codecpar->height = 0;
//	//st->time_base.den = 0;
//	//st->time_base.num = 0;
//
//	//if (ff->oc->oformat->flags & AVFMT_GLOBALHEADER) {
//	//	st->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//	//}
//	return st->index;
//}
//
//int ffoutput_add_audio_stream(void* p, enum AVCodecID codecId, int channel, int frequency, int bits_per_sample)
//{
//	AVStream* st;
//	struct ffoutput_t* ff;
//	ff = (struct ffoutput_t*)p;
//	
//	st = avformat_new_stream(ff->oc, NULL);
//	st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
//	st->codecpar->codec_id = codecId;
//	st->codecpar->channels = channel;
//	st->codecpar->sample_rate = frequency;
//	st->channel_layout = av_get_default_channel_layout(st->channels);
//
//	//if (ff->oc->oformat->flags & AVFMT_GLOBALHEADER) {
//	//	st->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
//	//}
//	return st->index;
//}

AVStream* ffoutput_addstream(void* p, const AVCodec* codec, AVCodecParameters* codecpar)
{
	AVStream* st;
	struct ffoutput_t* ff;
	ff = (struct ffoutput_t*)p;

	st = avformat_new_stream(ff->oc, codec);
	if (!st)
		return NULL;

	avcodec_parameters_copy(st->codecpar, codecpar);
	return st;
}

static int ffoutput_open(struct ffoutput_t* ff, const char* url, const char* format, AVDictionary** opts)
{
	int r;
	r = avformat_alloc_output_context2(&ff->oc, NULL, format, url);
	if (NULL == ff->oc)
	{
		printf("%s(%s): avformat_alloc_context failed.\n", __FUNCTION__, url);
		return -ENOMEM;
	}

	/* open the file */
	r = avio_open2(&ff->oc->pb, url, AVIO_FLAG_WRITE, &ff->interrupt_callback, opts);
	if (r < 0) {
		printf("%s(%s): avio_open2 failed: %s\n", __FUNCTION__, url, av_err2str(r));
		return r;
	}

	//for (i = 0; i < count; i++)
	//{
	//	AVStream* st;
	//	st = avformat_new_stream(ff->oc, NULL);
	//	avcodec_parameters_copy(st->codecpar, streams[i]->codecpar);
	//}

	//avformat_init_output(ff->oc, opts);

//	avformat_write_header(ff->oc, NULL);

	//av_dict_free(&opts);
	return 0;
}

void* ffoutput_create(const char* url, const char* format, AVDictionary** opts)
{
	struct ffoutput_t* ff;
	ff = (struct ffoutput_t*)malloc(sizeof(*ff));
	if (!ff)
		return NULL;

	memset(ff, 0, sizeof(*ff));
	ff->interrupt_callback.callback = ffoutput_interrupt;
	ff->interrupt_callback.opaque = ff;

	if (0 != ffoutput_open(ff, url, format, opts))
	{
		ffoutput_destroy(ff);
		return NULL;
	}

	return ff;
}

void ffoutput_destroy(void* p)
{
	struct ffoutput_t* ff;
	ff = (struct ffoutput_t*)p;
	if (ff->oc)
	{
		av_write_trailer(ff->oc);
		if (ff->oc->pb)
			avio_closep(&ff->oc->pb);
		avformat_free_context(ff->oc);
	}
	free(ff);
}

int ffoutput_write(void* p, AVPacket* pkt)
{
	struct ffoutput_t* ff;
	ff = (struct ffoutput_t*)p;

	if (!ff->header)
	{
		ff->header = 1;
		//avformat_init_output(ff->oc, NULL);
		avformat_write_header(ff->oc, NULL);
	}

	return av_interleaved_write_frame(ff->oc, pkt);
}
