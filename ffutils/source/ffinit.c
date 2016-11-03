#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
//#include "libavfilter/avfilter.h"

static void ffavcodec_init(void)
{
	avcodec_register_all();
}

static void ffavformat_init(void)
{
	av_register_all();
	avformat_network_init();
}

static void ffavfilter_init(void)
{
	//avfilter_register_all();
}

void ffutils_init(void)
{
	ffavcodec_init();
	ffavformat_init();
	ffavfilter_init();
}

void ffutils_deinit(void)
{
	avformat_network_deinit();
}
