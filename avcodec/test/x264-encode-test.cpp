#include "h264-encoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

void x264_encode_test(const char* input, const char* output)
{
	static unsigned char s_yuv[256 * 1024];
	char filename[128] = { 0 };
	FILE* wfp = fopen(output, "wb");

	h264_parameter_t param;
	param.profile = H264_PROFILE_BASELINE;
	param.level = H264_LEVEL_2_0;
	param.width = 352;
	param.height = 288;
	param.format = PICTURE_YUV420;
	param.frame_rate = 25000;
	param.gop_size = 50;
	param.bitrate = 500000;

	h264_encoder_t* api = x264_encoder();
//	h264_encoder_t* api = openh264_encoder();
	void* h264 = api->create(&param);

	for (int i = 0; i < 300; i++)
	{
		snprintf(filename, sizeof(filename), "%s%03d.yuv", input, i);
		FILE* fp = fopen(filename, "rb");
		int r = fread(s_yuv, 1, sizeof(s_yuv), fp);
		if (r > 0)
		{
			picture_t pic;
			memset(&pic, 0, sizeof(pic));
			pic.format = PICTURE_YUV420;
			pic.width = 352;
			pic.height = 288;
			pic.pts = i * 40;
			pic.dts = i * 40;
			pic.flags = 0;
			pic.data[0] = s_yuv;
			pic.linesize[0] = pic.width;
			pic.data[1] = pic.data[0] + pic.linesize[0] * pic.height;
			pic.linesize[1] = pic.width / 2;
			pic.data[2] = pic.data[1] + pic.linesize[1] * pic.height / 2;
			pic.linesize[2] = pic.width / 2;

			assert(api->input(h264, &pic) > 0);

			avpacket_t pkt;
			memset(&pkt, 0, sizeof(pkt));
			assert(api->getpacket(h264, &pkt) > 0);
			assert(pkt.bytes == fwrite(pkt.data, 1, pkt.bytes, wfp));
		}
		fclose(fp);
	}

	api->destroy(h264);
	fclose(wfp);
}
