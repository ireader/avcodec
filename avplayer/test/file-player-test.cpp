#include "sys/thread.h"
#include "sys/system.h"
#include "avplayer-file.h"
#include "flv-demuxer.h"
#include "flv-reader.h"
#include "flv-proto.h"
#include <stdlib.h>
#include <string.h>

static bool s_running;
static void* s_reader;
static flv_demuxer_t* s_demuxer;
static struct
{
	struct avpacket_t* pkt;
} s_param;

static struct avpacket_t* file_player_test_read(void* p)
{
	int tagtype = 0;
	uint32_t timestamp = 0;
	static uint8_t s_buffer[512 * 1024];

	s_param.pkt = NULL;
	do
	{
		int r = flv_reader_read(s_reader, &tagtype, &timestamp, s_buffer, sizeof(s_buffer));
		if (r < 0)
			return NULL;

		flv_demuxer_input(s_demuxer, tagtype, s_buffer, r, timestamp);
	} while (!s_param.pkt);

	return s_param.pkt;
}

int file_player_test_onflv(void* /*param*/, int avtype, const void* data, size_t bytes, uint32_t pts, uint32_t dts, int flags)
{
	enum AVPACKET_CODEC_ID codecid = AVCODEC_NONE;
	if (FLV_VIDEO_H264 == avtype)
	{
		codecid = AVCODEC_VIDEO_H264;
	}
	else if (FLV_VIDEO_H265 == avtype)
	{
		codecid = AVCODEC_VIDEO_H265;
	}
	else if (FLV_AUDIO_AAC == avtype)
	{
		codecid = AVCODEC_AUDIO_AAC;
	}
	else if (FLV_AUDIO_MP3 == avtype)
	{
		codecid = AVCODEC_AUDIO_MP3;
	}
	else
	{
		return -1;
	}

	struct avpacket_t* pkt = avpacket_alloc(bytes);
	memcpy(pkt->data, data, bytes);
	pkt->size = bytes;
	pkt->pts = pts;
	pkt->dts = dts;
	pkt->codecid = codecid;
	s_param.pkt = pkt;
	return 0;
}

static int STDCALL file_player_thread(void* player)
{
	while (s_running)
	{
		int r = avplayer_file_process(player, system_clock());
		if (r > 0)
			system_sleep(r);
	}
	return 0;
}

int file_player_test(void* window, const char* flv)
{
	pthread_t thread;
	s_reader = flv_reader_create(flv);
	s_demuxer = flv_demuxer_create(file_player_test_onflv, &s_param);

	void* player = avplayer_file_create(window, file_player_test_read, NULL);
	s_running = true;
	thread_create(&thread, file_player_thread, player);

	avplayer_file_play(player);

	//s_running = false;
	//thread_destroy(thread);
	//if (s_demuxer)
	//{
	//	flv_demuxer_destroy(s_demuxer);
	//	s_demuxer = NULL;
	//}

	//if (s_reader)
	//{
	//	flv_reader_destroy(s_reader);
	//	s_reader = NULL;
	//}
	return 0;
}
