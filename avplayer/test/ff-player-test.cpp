#include "avplayer-file.h"
#include "ffhelper.h"
#include "ffinput.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <windows.h>

static inline const uint8_t* h264_startcode(const uint8_t *data, size_t bytes)
{
	size_t i;
	for (i = 2; i + 1 < bytes; i++)
	{
		if (0x01 == data[i] && 0x00 == data[i - 1] && 0x00 == data[i - 2])
			return data + i + 1;
	}

	return NULL;
}

static inline int h264_nal_type(const uint8_t *data, size_t bytes)
{
	uint8_t naltype;
	const uint8_t *p;

	do
	{
		p = h264_startcode(data, bytes);
		if (p)
		{
			naltype = p[0] & 0x1f;
			// 1: no-IDR slice
			// 2: A-slice
			// 3: B-slice
			// 4: C-slice
			// 5: IDR frame
			if (naltype > 0 && naltype < 6)
			{
				return naltype;
			}

			bytes -= p - data;
			data = p;
		}
	} while (p);

	return 0;
}

static struct avpacket_t* ff_read_frame(void* ff)
{
	char msg[1024];

	avpacket_t* pkt;
	int r = ffinput_read(ff, &pkt);

	if (r > 0)
	{
		if (pkt->codecid < AVCODEC_AUDIO_PCM)
		{
			if (pkt->pts < 0)
				return NULL;
			int n = h264_nal_type(pkt->data, pkt->size);
			if (n < 1 || n > 5)
			{
				return NULL;
			}
		}

		return pkt;
	}
	return NULL;
}

int ff_player_test(void* window, const char* file)
{
	char msg[1024];
	void* ff = ffinput_create(file);
	while (1)
	{
		avpacket_t* pkt;
		int r = ffinput_read(ff, &pkt);
		if (r < 0)
		{
			r = -11;
		}

		sprintf(msg, "pkt pts: %lld, %lld\n", pkt->pts, pkt->dts);
		OutputDebugString(msg);

		avpacket_release(pkt);
	}

	void* player = avplayer_file_create(window, ff_read_frame, ff);
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
