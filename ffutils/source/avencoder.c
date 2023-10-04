#include "avencoder.h"
#include "ffencoder.h"
#include "ffhelper.h"
#include <string.h>
#include <assert.h>

void* avencoder_create_h264(const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate)
{
	void* ff;
	AVDictionary* opts = NULL;
	AVCodecParameters codecpar;
	memset(&codecpar, 0, sizeof(AVCodecParameters));
	codecpar.codec_type = AVMEDIA_TYPE_VIDEO;
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
	av_dict_set(&opts, "threads", "4", 0);

	ff = ffencoder_create(&codecpar, &opts);
	av_dict_free(&opts);
	return ff;
}
