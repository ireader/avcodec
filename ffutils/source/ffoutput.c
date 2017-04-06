#include "ffoutput.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "libavformat/avformat.h"

struct ffoutput_t
{
	AVFormatContext* oc;
	struct AVIOInterruptCB interrupt_callback;
};

static int ffoutput_interrupt(void* p)
{
	struct ffoutput_t* ff;
	ff = (struct ffoutput_t*)p;
	printf("ffoutput_interrupt: %s\n", ff->oc->filename);
	return 0;
}

static int ffoutput_video_stream(struct ffoutput_t* ff, enum AVCodecID codecId)
{
	AVStream* st = NULL;
	st = avformat_new_stream(ff->oc, NULL);
	st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	st->codecpar->codec_id = codecId;
	st->codecpar->bit_rate = 0;
	st->codecpar->width = 0;
	st->codecpar->height = 0;
	st->time_base.den = 0;
	st->time_base.num = 0;

	if (ff->oc->oformat->flags & AVFMT_GLOBALHEADER) {
		st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	return 0;
}

static int ffoutput_audio_stream(struct ffoutput_t* ff, enum AVCodecID codecId)
{
	AVStream* st = NULL;
	st = avformat_new_stream(ff->oc, NULL);
	st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	st->codecpar->codec_id = codecId;
	st->codecpar->channels = 1;
	st->codecpar->sample_rate = 44000;
	st->codecpar->frame_size = 0;

	if (ff->oc->oformat->flags & AVFMT_GLOBALHEADER) {
		st->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	return 0;
}

static int ffoutput_open(struct ffoutput_t* ff, const char* url, AVStream* streams[], int count)
{
	int i, r;
	AVStream* st;
	AVDictionary* opts = NULL;

	r = avformat_alloc_output_context2(&ff->oc, NULL, "flv", url);
	if (NULL == ff->oc)
	{
		printf("%s(%s): avformat_alloc_context failed.\n", __FUNCTION__, url);
		return ENOMEM;
	}

	/* open the file */
	r = avio_open2(&ff->oc->pb, url, AVIO_FLAG_WRITE, &ff->oc->interrupt_callback, &opts);
	if (r < 0) {
		printf("%s(%s): avio_open2 failed: %d\n", __FUNCTION__, url, r);
		return r;
	}

	for (i = 0; i < count; i++)
	{
		st = avformat_new_stream(ff->oc, NULL);
		avcodec_parameters_copy(st->codecpar, streams[i]->codecpar);
	}

//	avformat_write_header(ff->oc, NULL);

	av_dict_free(&opts);
	return 0;
}

void* ffoutput_create(const char* url, AVStream* streams[], int count)
{
	struct ffoutput_t* ff;
	ff = (struct ffoutput_t*)malloc(sizeof(*ff));
	if (!ff)
		return NULL;

	memset(ff, 0, sizeof(*ff));
	ff->interrupt_callback.callback = ffoutput_interrupt;
	ff->interrupt_callback.opaque = ff;

	if (0 != ffoutput_open(ff, url, streams, count))
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

	return av_interleaved_write_frame(ff->oc, pkt);
}
