#include "avplayer-file.h"
#include "flv-demuxer.h"
#include "flv-reader.h"
#include <stdlib.h>
#include <string.h>

static void* s_reader;
static void* s_demuxer;
static struct
{
	struct avpacket_t** pkt;
	int* type;
} s_param;

static int file_player_test_read(void* p, struct avpacket_t** pkt, int* type)
{
	int tagtype = 0;
	uint32_t timestamp = 0;
	static uint8_t s_buffer[512 * 1024];

	s_param.pkt = pkt;
	s_param.type = type;
	*s_param.type = -1;
	do
	{
		int r = flv_reader_read(s_reader, &tagtype, &timestamp, s_buffer, sizeof(s_buffer));
		if (r < 0)
			return r;

		flv_demuxer_input(s_demuxer, tagtype, s_buffer, r, timestamp);
	} while (-1 == *s_param.type);

	return 1;
}

void file_player_test_onflv(void* /*param*/, int type, const void* data, size_t bytes, uint32_t pts, uint32_t dts)
{
	if (FLV_AVC == type)
	{
		*s_param.type = 1;
	}
	else if (FLV_AAC == type || FLV_MP3 == type)
	{
		*s_param.type = 0;
	}
	else
	{
		return;
	}

	struct avpacket_t* pkt = avpacket_alloc(bytes);
	memcpy(pkt->data, data, bytes);
	pkt->bytes = bytes;
	pkt->pts = pts;
	pkt->dts = dts;
	*s_param.pkt = pkt;
}

int file_player_test(void* window, const char* flv)
{
	s_reader = flv_reader_create(flv);
	s_demuxer = flv_demuxer_create(file_player_test_onflv, &s_param);

	void* player = avplayer_file_create(window, file_player_test_read, NULL);
	avplayer_file_play(player);

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
