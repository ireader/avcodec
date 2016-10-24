#include "ffinput.h"
#include <errno.h>
#include <memory.h>
#include <assert.h>
#include "libavformat/avformat.h"

struct ffinput_t
{
	AVFormatContext* ic;
};

static int ffinput_open(struct ffinput_t* ff, const char* url)
{
	int r;
	AVDictionary* opt = NULL;
	ff->ic = avformat_alloc_context();
	if (NULL == ff->ic)
	{
		printf("%s(%s): avformat_alloc_context failed.\n", __FUNCTION__, url);
		return ENOMEM;
	}

	//if (!av_dict_get(ff->opt, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
	//	av_dict_set(&ff->opt, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
	//	scan_all_pmts_set = 1;
	//}

	r = avformat_open_input(&ff->ic, url, NULL, &opt);
	if (0 != r)
	{
		printf("%s: avformat_open_input(%s) => %d\n", __FUNCTION__, url, r);
		return r;
	}

	//if (scan_all_pmts_set)
	//	av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

	//ff->ic->probesize = 100 * 1024;
	//ff->ic->max_analyze_duration = 5 * AV_TIME_BASE;

	/* If not enough info to get the stream parameters, we decode the
	first frames to get it. (used in mpeg case for example) */
	r = avformat_find_stream_info(ff->ic, &opt);
	if (r < 0) {
		printf("%s(%s): could not find codec parameters\n", __FUNCTION__, url);
		return r;
	}

	av_dict_free(&opt);
	return 0;
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

	return ff;
}

void ffinput_destroy(void* p)
{
	struct ffinput_t* ff;
	ff = (struct ffinput_t*)p;
	if (ff->ic)
	{
		avformat_close_input(&ff->ic);
		avformat_free_context(ff->ic);
	}
	free(ff);
}

int ffinput_read(void* p, AVPacket* pkt)
{
	struct ffinput_t* ff;
	ff = (struct ffinput_t*)p;

	return av_read_frame(ff->ic, pkt);
}

int ffinput_getstream(void* p, AVStream*** streams)
{
	struct ffinput_t* ff;
	ff = (struct ffinput_t*)p;

	if (!ff->ic)
		return -1;
	*streams = ff->ic->streams;
	return ff->ic->nb_streams;
}
