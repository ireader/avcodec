#include "sockutil.h"
#include "avplayer-live.h"
#include "rtmp-client.h"
#include "flv-demuxer.h"
#include "flv-proto.h"
#include "h264-util.h"
#include "app-log.h"
#include <assert.h>

static flv_demuxer_t* s_flv;

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
	return flv_demuxer_input(s_flv, FLV_TYPE_AUDIO, data, bytes, timestamp);
}

static int rtmp_client_onvideo(void* /*param*/, const void* data, size_t bytes, uint32_t timestamp)
{
	return flv_demuxer_input(s_flv, FLV_TYPE_VIDEO, data, bytes, timestamp);
}

static int rtmp_client_onmeta(void* /*param*/, const void* /*data*/, size_t /*bytes*/)
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
static void rtmp_play_test(void* player, const char* host, const char* app, const char* stream)
{
	static char packet[8 * 1024 * 1024];
	snprintf(packet, sizeof(packet), "rtmp://%s/%s", host, app); // tcurl

	socket_init();
	socket_t socket = socket_connect_host(host, 1935, 2000);
	socket_setnonblock(socket, 0);

	struct rtmp_client_handler_t handler;
	handler.send = rtmp_client_send;
	handler.onmeta = rtmp_client_onmeta;
	handler.onaudio = rtmp_client_onaudio;
	handler.onvideo = rtmp_client_onvideo;
	rtmp_client_t* rtmp = rtmp_client_create(app, stream, packet/*tcurl*/, &socket, &handler);
	s_flv = flv_demuxer_create(rtmp_client_ondata, player);

	int r = rtmp_client_start(rtmp, 1);

	while ((r = socket_recv(socket, packet, sizeof(packet), 0)) > 0)
	{
		r = rtmp_client_input(rtmp, packet, r);
	}

	rtmp_client_stop(rtmp);
	flv_demuxer_destroy(s_flv);
	rtmp_client_destroy(rtmp);
	socket_close(socket);
	socket_cleanup();
}

int live_player_test(void* window, const char* host, const char* app, const char* stream)
{
	void* player = avplayer_live_create(window);
	rtmp_play_test(player, host, app, stream);
	avplayer_live_destroy(player);
	return 0;
}
