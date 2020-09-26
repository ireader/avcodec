#include "sockutil.h"
#include "sys/thread.h"
#include "sys/system.h"
#include "avplayer-live.h"
#include "rtmp-client.h"
#include "flv-demuxer.h"
#include "flv-proto.h"
#include "app-log.h"
#include "time64.h"
#include <assert.h>

struct live_player_test_t
{
    bool running;
    socket_t socket;
    
    flv_demuxer_t* flv;
    rtmp_client_t* rtmp;
    
    void* player;
    struct avstream_t* video;
    struct avstream_t* audio;
};

static int rtmp_client_send(void* param, const void* header, size_t len, const void* data, size_t bytes)
{
    struct live_player_test_t* player = (struct live_player_test_t*)param;
	socket_bufvec_t vec[2];
	socket_setbufvec(vec, 0, (void*)header, len);
	socket_setbufvec(vec, 1, (void*)data, bytes);
	return socket_send_v_all_by_time(player->socket, vec, bytes ? 2 : 1, 0, 2000);
}

static int rtmp_client_onaudio(void* param, const void* data, size_t bytes, uint32_t timestamp)
{
    struct live_player_test_t* player = (struct live_player_test_t*)param;
	return flv_demuxer_input(player->flv, FLV_TYPE_AUDIO, data, bytes, timestamp);
}

static int rtmp_client_onvideo(void* param, const void* data, size_t bytes, uint32_t timestamp)
{
    struct live_player_test_t* player = (struct live_player_test_t*)param;
	return flv_demuxer_input(player->flv, FLV_TYPE_VIDEO, data, bytes, timestamp);
}

static int rtmp_client_onscript(void* /*param*/, const void* /*data*/, size_t /*bytes*/, uint32_t /*timestamp*/)
{
	return 0;
}

static int rtmp_client_ondata(void* param, int avtype, const void* data, size_t bytes, uint32_t pts, uint32_t dts, int flags)
{
    struct live_player_test_t* player = (struct live_player_test_t*)param;
	avpacket_t* pkt = avpacket_alloc((int)bytes);
	memcpy(pkt->data, data, bytes);
	pkt->size = (int)bytes;
	pkt->pts = pts;
	pkt->dts = dts;

	if (FLV_VIDEO_H264 == avtype)
	{
        avstream_addref(player->video);
        pkt->stream = player->video;
		pkt->stream->codecid = AVCODEC_VIDEO_H264;
		if (0x01 & flags)
			pkt->flags |= AVPACKET_FLAG_KEY;
		avplayer_live_input(player->player, pkt);
	}
	else if (FLV_VIDEO_H265 == avtype)
	{
        avstream_addref(player->video);
        pkt->stream = player->video;
		pkt->stream->codecid = AVCODEC_VIDEO_H265;
		if (0x01 & flags)
		    pkt->flags |= AVPACKET_FLAG_KEY;
		avplayer_live_input(player->player, pkt);
	}
	else if (FLV_AUDIO_AAC == avtype)
	{
        avstream_addref(player->audio);
        pkt->stream = player->audio;
		pkt->stream->codecid = AVCODEC_AUDIO_AAC;
		avplayer_live_input(player->player, pkt);
	}
	else if (FLV_AUDIO_MP3 == avtype)
	{
        avstream_addref(player->audio);
        pkt->stream = player->audio;
		pkt->stream->codecid = AVCODEC_AUDIO_MP3;
		avplayer_live_input(player->player, pkt);
	}
	avpacket_release(pkt);
	return 0;
}

// rtmp_play_test("rtmp://strtmpplay.cdn.suicam.com/carousel/51632");
static void rtmp_play_test(struct live_player_test_t* player, const char* host, int port, const char* app, const char* stream)
{
	static char packet[8 * 1024 * 1024];
	snprintf(packet, sizeof(packet), "rtmp://%s/%s", host, app); // tcurl

	socket_init();
	player->socket = socket_connect_host(host, port, 2000);
	socket_setnonblock(player->socket, 0);
	socket_setnondelay(player->socket, 1);
	size_t recvbuf = 256 * 1024;
	socket_getrecvbuf(player->socket, &recvbuf);
	socket_setrecvbuf(player->socket, recvbuf * 2);

	struct rtmp_client_handler_t handler;
	handler.send = rtmp_client_send;
	handler.onaudio = rtmp_client_onaudio;
	handler.onvideo = rtmp_client_onvideo;
	handler.onscript = rtmp_client_onscript;
	player->rtmp = rtmp_client_create(app, stream, packet/*tcurl*/, player, &handler);
	player->flv = flv_demuxer_create(rtmp_client_ondata, player);
    player->video = avstream_alloc(0);
    player->audio = avstream_alloc(0);

	int r = rtmp_client_start(player->rtmp, 1);

	while ((r = socket_recv(player->socket, packet, sizeof(packet), 0)) > 0)
	{
		r = rtmp_client_input(player->rtmp, packet, r);
	}

    avstream_release(player->video);
    avstream_release(player->audio);
	rtmp_client_stop(player->rtmp);
	flv_demuxer_destroy(player->flv);
	rtmp_client_destroy(player->rtmp);
	socket_close(player->socket);
	socket_cleanup();
}

static int STDCALL live_player_thread(void* param)
{
    struct live_player_test_t* player = (struct live_player_test_t*)param;
	while (player->running)
	{
		int r = avplayer_live_process(player->player, system_clock());
		if (r > 0)
			system_sleep(r);
	}
	return 0;
}

int live_player_test(void* window, const char* host, int port, const char* app, const char* stream)
{
	pthread_t thread;
    struct live_player_test_t player;
	player.player = avplayer_live_create(window);

	player.running = true;
	thread_create(&thread, live_player_thread, &player);
	rtmp_play_test(&player, host, port, app, stream);

	player.running = false;
	thread_destroy(thread);
	avplayer_live_destroy(player.player);
	return 0;
}
