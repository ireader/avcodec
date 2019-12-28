#include "sockutil.h"
#include "sys/thread.h"
#include "sys/system.h"
#include "avplayer-live.h"
#include "rtmp-client.h"
#include "flv-demuxer.h"
#include "flv-proto.h"
#include "h264-util.h"
#include "app-log.h"
#include "time64.h"
#include <assert.h>

struct player_statistic_t
{
	time64_t start;
	time64_t stop;
	time64_t dns;
	time64_t connected;
	time64_t first_recv; // rtmp handshake
	time64_t first_packet; // first audio/video/data chunk/packet
	time64_t first_audio_frame;
	time64_t first_video_frame;
};

static bool s_running;
static flv_demuxer_t* s_flv;
static struct player_statistic_t s_stats;

static int rtmp_client_send(void* param, const void* header, size_t len, const void* data, size_t bytes)
{
	socket_t* socket = (socket_t*)param;
	socket_bufvec_t vec[2];
	socket_setbufvec(vec, 0, (void*)header, len);
	socket_setbufvec(vec, 1, (void*)data, bytes);
	return socket_send_v_all_by_time(*socket, vec, bytes ? 2 : 1, 0, 2000);
}

static int rtmp_client_onaudio(void* /*param*/, const void* data, size_t bytes, uint32_t timestamp)
{
	if (0 == s_stats.first_audio_frame)
		s_stats.first_audio_frame = time64_now();

	//char msg[1024];
	//sprintf(msg, "Audio %u\n", timestamp);
	//OutputDebugStringA(msg);
	return flv_demuxer_input(s_flv, FLV_TYPE_AUDIO, data, bytes, timestamp);
}

static int rtmp_client_onvideo(void* /*param*/, const void* data, size_t bytes, uint32_t timestamp)
{
	if (0 == s_stats.first_video_frame)
	{
		s_stats.first_video_frame = time64_now();
		
		char msg[1024];
		sprintf(msg, "Video %.1f, Audio: %.1f\n", (unsigned int)(s_stats.first_video_frame-s_stats.start)/1000.0, 0==s_stats.first_audio_frame ? 0.0 : (unsigned int)(s_stats.first_audio_frame-s_stats.start)/1000.0);
		OutputDebugStringA(msg);
	}

	//char msg[1024];
	//sprintf(msg, "Video %u\n", timestamp);
	//OutputDebugStringA(msg);
	return flv_demuxer_input(s_flv, FLV_TYPE_VIDEO, data, bytes, timestamp);
}

static int rtmp_client_onscript(void* /*param*/, const void* /*data*/, size_t /*bytes*/, uint32_t /*timestamp*/)
{
	return 0;
}

static int rtmp_client_ondata(void* player, int avtype, const void* data, size_t bytes, uint32_t pts, uint32_t dts, int flags)
{
	avpacket_t* pkt = avpacket_alloc(bytes);
	memcpy(pkt->data, data, bytes);
	pkt->size = bytes;
	pkt->pts = pts;
	pkt->dts = dts;

	if (FLV_VIDEO_H264 == avtype)
	{
		pkt->codecid = AVCODEC_VIDEO_H264;
		if ((0x01 & flags) || h264_idr(pkt->data, pkt->size))
			pkt->flags |= AVPACKET_FLAG_KEY;
		avplayer_live_input(player, pkt);
	}
	else if (FLV_VIDEO_H265 == avtype)
	{
		pkt->codecid = AVCODEC_VIDEO_H265;
		//if ((0x01 & flags) || h265_idr(pkt->data, pkt->size))
		//	pkt->flags |= AVPACKET_FLAG_KEY;
		avplayer_live_input(player, pkt);
	}
	else if (FLV_AUDIO_AAC == avtype)
	{
		pkt->codecid = AVCODEC_AUDIO_AAC;
		avplayer_live_input(player, pkt);
	}
	else if (FLV_AUDIO_MP3 == avtype)
	{
		pkt->codecid = AVCODEC_AUDIO_MP3;
		avplayer_live_input(player, pkt);
	}
	avpacket_release(pkt);
	return 0;
}

// rtmp_play_test("rtmp://strtmpplay.cdn.suicam.com/carousel/51632");
static void rtmp_play_test(void* player, const char* host, int port, const char* app, const char* stream)
{
	static char packet[8 * 1024 * 1024];
	snprintf(packet, sizeof(packet), "rtmp://%s/%s", host, app); // tcurl

	s_stats.start = time64_now();
	socket_init();
	socket_t socket = socket_connect_host(host, port, 2000);
	s_stats.connected = time64_now();
	socket_setnonblock(socket, 0);
	socket_setnondelay(socket, 1);
	size_t recvbuf = 256 * 1024;
	socket_getrecvbuf(socket, &recvbuf);
	socket_setrecvbuf(socket, recvbuf * 2);

	struct rtmp_client_handler_t handler;
	handler.send = rtmp_client_send;
	handler.onaudio = rtmp_client_onaudio;
	handler.onvideo = rtmp_client_onvideo;
	handler.onscript = rtmp_client_onscript;
	rtmp_client_t* rtmp = rtmp_client_create(app, stream, packet/*tcurl*/, &socket, &handler);
	s_flv = flv_demuxer_create(rtmp_client_ondata, player);

	int r = rtmp_client_start(rtmp, 1);

	while ((r = socket_recv(socket, packet, sizeof(packet), 0)) > 0)
	{
		if(0 == s_stats.first_recv)
			s_stats.first_recv = time64_now();
		r = rtmp_client_input(rtmp, packet, r);
	}

	rtmp_client_stop(rtmp);
	flv_demuxer_destroy(s_flv);
	rtmp_client_destroy(rtmp);
	socket_close(socket);
	socket_cleanup();
}

static int STDCALL live_player_thread(void* player)
{
	while (s_running)
	{
		int r = avplayer_live_process(player, system_clock());
		if (r > 0)
			system_sleep(r);
	}
	return 0;
}

int live_player_test(void* window, const char* host, int port, const char* app, const char* stream)
{
	pthread_t thread;
	void* player = avplayer_live_create(window);

	s_running = true;
	thread_create(&thread, live_player_thread, player);
	rtmp_play_test(player, host, port, app, stream);

	s_running = false;
	thread_destroy(thread);
	avplayer_live_destroy(player);
	return 0;
}
