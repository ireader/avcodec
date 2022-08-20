#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
//#include "libavfilter/avfilter.h"

static void ffavcodec_init(void)
{
	//avcodec_register_all();
}

static void ffavformat_init(void)
{
	//av_register_all();
	//avformat_network_init();
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
	//avformat_network_deinit();
}

void ffutils_list(void)
{
	void* it;
	const AVCodec* codec;
	const AVInputFormat* ifmt;
	const AVOutputFormat* ofmt;

	it = NULL;
	for(ofmt = av_muxer_iterate(&it); ofmt; ofmt = av_muxer_iterate(&it))
	{
		printf("AV Output Format: %s\n", ofmt->long_name);
	}
		
	it = NULL;
	for(ifmt = av_demuxer_iterate(&it); ifmt; ifmt = av_demuxer_iterate(&it))
	{
		printf("AV Input Format: %s\n", ifmt->long_name);
	}

	it = NULL;
	for(codec = av_codec_iterate(&it); codec; codec = av_codec_iterate(&it))
	{
#if LIBAVCODEC_VERSION_MAJOR < 59
		printf("AV Codec(%c): %s(%s)\n", codec->decode ? 'D' : 'E', codec->name, codec->long_name);
#else
		printf("AV Codec(%c): %s(%s)\n", av_codec_is_decoder(codec) ? 'D' : 'E', codec->name, codec->long_name);
#endif
	}
}
