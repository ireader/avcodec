#include "avplayer-file.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static uint8_t s_buffer[10 * 1024 * 1024];

static inline const uint8_t* h264_startcode(const uint8_t *data, ptrdiff_t bytes)
{
	ptrdiff_t i;
	for (i = 2; i + 1 < bytes; i++)
	{
		if (0x01 == data[i] && 0x00 == data[i - 1] && 0x00 == data[i - 2])
		{
			if (i >= 3 && 0x00 == data[i - 3])
				return data + (i - 3);
			else
				return data + (i - 2);
		}
	}

	return NULL;
}

static inline const uint8_t* h264_frame(const uint8_t *data, ptrdiff_t bytes)
{
	uint8_t naltype;
	const uint8_t *p;

	while (bytes > 4)
	{
		p = h264_startcode(data, bytes);
		if (!p)
			return data + bytes;

		naltype = p[0x01 == p[2] ? 3 : 4] & 0x1f;
		// 1: no-IDR slice
		// 2: A-slice
		// 3: B-slice
		// 4: C-slice
		// 5: IDR frame
		if (naltype > 0 && naltype < 6)
		{
			return p;
		}

		p += 3;
		bytes -= p - data;
		data = p;
	}

	return data + bytes;
}

struct avpacket_t* h264_packet(uint8_t* data, size_t bytes, int64_t pts, int64_t dts)
{
    static avstream_t* stream;
    if(!stream)
        stream = avstream_alloc(0);
    avstream_addref(stream);
    
	struct avpacket_t* pkt = avpacket_alloc(bytes);
	memcpy(pkt->data, data, bytes);
    pkt->stream = stream;
	pkt->stream->codecid = AVCODEC_VIDEO_H264;
	pkt->size = bytes;
	pkt->pts = pts;
	pkt->dts = dts;
	return pkt;
}

static struct avpacket_t* h264_file_reader(void* end)
{
	static int64_t pts = 0;
	static const uint8_t* p = NULL;
	struct avpacket_t* pkt = NULL;

	if (NULL == p)
	{
		p = h264_startcode(s_buffer, (uint8_t*)end - s_buffer);
	}

	if (p >= end)
		return NULL;

	const uint8_t* next = h264_frame(p, (uint8_t*)end - p);
	next = h264_startcode(next + 4, (uint8_t*)end - next - 4);

	pkt = h264_packet((uint8_t*)p, next - p, pts, pts);

	p = next;
	pts += 40; // 40ms
	return pkt;
}

int h264_player_test(void* window, const char* h264)
{
	FILE* fp = fopen(h264, "rb");
	int r = fread(s_buffer, 1, sizeof(s_buffer), fp);
	fclose(fp);

	uint8_t *end = s_buffer + r;
	void* player = avplayer_file_create(window, h264_file_reader, end);
	avplayer_file_play(player);
	return 0;
}
