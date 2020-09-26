#include "sys/thread.h"
#include "sys/system.h"
#include "avplayer-file.h"
#include "flv-demuxer.h"
#include "flv-reader.h"
#include "flv-proto.h"
#include "avpacket.h"
#include <stdlib.h>
#include <string.h>

struct file_player_test_t
{
    bool running;
    void* reader;
    flv_demuxer_t* demuxer;
    
    void* player;
    struct avstream_t* video;
    struct avstream_t* audio;
    struct avpacket_t* pkt;
};

static struct avpacket_t* file_player_test_read(void* p)
{
	int tagtype = 0;
    size_t taglen = 0;
	uint32_t timestamp = 0;
    static uint8_t s_buffer[10 * 1024 * 1024];
    struct file_player_test_t* player = (struct file_player_test_t*)p;

	player->pkt = NULL;
	do
	{
		int r = flv_reader_read(player->reader, &tagtype, &timestamp, &taglen, s_buffer, sizeof(s_buffer));
		if (r < 0)
			return NULL;

		flv_demuxer_input(player->demuxer, tagtype, s_buffer, taglen, timestamp);
	} while (!player->pkt);

	return player->pkt;
}

int file_player_test_onflv(void* param, int avtype, const void* data, size_t bytes, uint32_t pts, uint32_t dts, int flags)
{
    struct file_player_test_t* player = (struct file_player_test_t*)param;
    struct avpacket_t* pkt = avpacket_alloc(bytes);
    memcpy(pkt->data, data, bytes);
    pkt->size = bytes;
    pkt->pts = pts;
    pkt->dts = dts;

	if (FLV_VIDEO_H264 == avtype)
	{
        avstream_addref(player->video);
        pkt->stream = player->video;
		pkt->stream->codecid = AVCODEC_VIDEO_H264;
	}
	else if (FLV_VIDEO_H265 == avtype)
	{
        avstream_addref(player->video);
        pkt->stream = player->video;
		pkt->stream->codecid = AVCODEC_VIDEO_H265;
	}
	else if (FLV_AUDIO_AAC == avtype)
	{
        avstream_addref(player->audio);
        pkt->stream = player->audio;
		pkt->stream->codecid = AVCODEC_AUDIO_AAC;
	}
	else if (FLV_AUDIO_MP3 == avtype)
	{
        avstream_addref(player->audio);
        pkt->stream = player->audio;
		pkt->stream->codecid = AVCODEC_AUDIO_MP3;
	}
	else
	{
        avpacket_release(pkt);
		return -1;
	}

	player->pkt = pkt;
	return 0;
}

static int STDCALL file_player_thread(void* param)
{
    int r = 0;
    struct file_player_test_t* player = (struct file_player_test_t*)param;
	while (player->running && r >= 0)
	{
		r = avplayer_file_process(player->player, system_clock());
		if (r > 0)
			system_sleep(r);
	}
	return 0;
}

int file_player_test(void* window, const char* flv)
{
	pthread_t thread;
    struct file_player_test_t player;
	player.reader = flv_reader_create(flv);
	player.demuxer = flv_demuxer_create(file_player_test_onflv, &player);
    player.video = avstream_alloc(0);
    player.audio = avstream_alloc(0);

	player.player = avplayer_file_create(window, file_player_test_read, &player);
	player.running = true;
	thread_create(&thread, file_player_thread, player.player);

	avplayer_file_play(&player);
    
	thread_destroy(thread); // wait for file end
    flv_reader_destroy(player.reader);
    flv_demuxer_destroy(player.demuxer);
	return 0;
}
