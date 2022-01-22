// test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include "ffencoder.h"
#include "ffoutput.h"
#include "ffresample.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

static void video_test()
{
	AVCodecParameters* codecpar = avcodec_parameters_alloc();
	codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	codecpar->codec_id = AV_CODEC_ID_H264;
	codecpar->format = AV_PIX_FMT_YUV420P;
	codecpar->width = 1920;
	codecpar->height = 1080;
	codecpar->bit_rate = 2000000;

	AVDictionary* opts = NULL;
	av_dict_set(&opts, "preset", "fast", 0);
	av_dict_set(&opts, "crt", "23", 0);
	void* ff = ffencoder_create(codecpar, opts);
	av_dict_free(&opts);

	AVCodecParameters* par = avcodec_parameters_alloc();
	ffencoder_getcodecpar(ff, par);

	void* w = ffoutput_create("1.mp4", "mp4");
	AVStream* s = ffoutput_addstream(w, par);
	s->time_base = av_make_q(1, 900000);
	avcodec_parameters_free(&par);

	for (int64_t dts = 0; dts < 90 * 1000; dts += 40*90)
	{
		static const uint8_t yuv[1920 * 1080 * 3 / 2] = { 0 };
		AVFrame* frame = av_frame_alloc();
		frame->format = AV_PIX_FMT_YUV420P;
		//frame->pict_type = AV_PICTURE_TYPE_I;
		//frame->flags = 0;
		frame->width = 1920;
		frame->height = 1080;
		av_frame_get_buffer(frame, 0);
		//frame->linesize[0] = 1920;
		//frame->linesize[1] = 1920 /2;
		//frame->linesize[2] = 1920 /2;
		//frame->data[0] = (uint8_t*)yuv;
		//frame->data[1] = (uint8_t*)(yuv + 1920 * 1080);
		//frame->data[2] = (uint8_t*)(yuv + 1920 * 1080 + 1920 * 1080 / 4);
		frame->pkt_dts = dts;
		//frame->pkt_pts = dts;
		frame->pts = dts;
		av_frame_is_writable(frame);
		av_frame_make_writable(frame);

		int r = ffencoder_input(ff, frame);
		if (r >= 0)
		{
			AVPacket pkt;
			memset(&pkt, 0, sizeof(pkt));
			r = ffencoder_getpacket(ff, &pkt);
			if (r >= 0)
			{
				pkt.stream_index = s->index;
				ffoutput_write(w, &pkt);
				av_packet_unref(&pkt);
			}
		}

		av_frame_free(&frame);

		Sleep(10);
	}

	ffencoder_destroy(ff);
	ffoutput_destroy(w);
}

static void audio_test()
{
	AVCodecParameters* codecpar = avcodec_parameters_alloc();
	codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	codecpar->codec_id = AV_CODEC_ID_AAC;
	codecpar->format = AV_SAMPLE_FMT_FLTP;
	codecpar->channels = 2;
	codecpar->sample_rate = 48000;
	codecpar->channel_layout = av_get_default_channel_layout(codecpar->channels);
	codecpar->bit_rate = 128000;

	AVDictionary* opts = NULL;
	void* ff = ffencoder_create(codecpar, opts);
	void* swr = ffresample_create(AV_SAMPLE_FMT_S16, 48000, 2, AV_SAMPLE_FMT_FLTP, 48000, 2);
	AVCodecParameters* par = avcodec_parameters_alloc();
	ffencoder_getcodecpar(ff, par);

	void* w = ffoutput_create("1.mp4", "mp4");
	AVStream* s = ffoutput_addstream(w, par);
	s->time_base = av_make_q(1, par->sample_rate);
	avcodec_parameters_free(&par);

	FILE* fp = fopen("C:\\Users\\Administrator\\Downloads\\rc_dev\\rc_dev\\RDP_Win\\000001946F7704A0.pcm", "rb");
	for (int64_t dts = 48000; true || dts < 10 * 1000; dts += 480)
	{
		static uint8_t pcm[1024 * 2 * 4];

		AVFrame* frame = av_frame_alloc();
		frame->format = AV_SAMPLE_FMT_FLTP;
		frame->channels = 2;
		frame->channel_layout = av_get_default_channel_layout(frame->channels);
		frame->sample_rate = 48000;
		frame->nb_samples = 480;
		frame->pkt_dts = dts;
		frame->pts = dts;
		av_frame_get_buffer(frame, 0);
		//av_frame_make_writable(frame);

		int r = 0;
		if (480 * 4 != fread(pcm, 1, 480 * 4, fp))
			break;

		const uint8_t* in[2] = { pcm, pcm + 480 * 2};
		r = ffresample_convert(swr, (const uint8_t**)in, 480, frame->data, 480);
		//frame->nb_samples = 1024;

		r = ffencoder_input(ff, frame);
		if (r >= 0)
		{
			AVPacket pkt;
			memset(&pkt, 0, sizeof(pkt));
			r = ffencoder_getpacket(ff, &pkt);
			if (r >= 0)
			{
				pkt.stream_index = s->index;
				ffoutput_write(w, &pkt);
				av_packet_unref(&pkt);
			}
		}

		av_frame_free(&frame);

		//Sleep(10);
	}

	ffencoder_destroy(ff);
	ffoutput_destroy(w);
}

int main()
{
	//av_register_all();
	//avcodec_register_all();
	avformat_network_init();

	audio_test();

    std::cout << "Hello World!\n";
}
