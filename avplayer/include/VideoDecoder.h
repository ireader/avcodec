#pragma once

#include "avdecoder.h"
#include "avstream.h"
#include <assert.h>

class VideoDecoder
{
public:
	VideoDecoder()
	{
		m_decoder = NULL;
	}

	~VideoDecoder()
	{
		DestroyDecoder();
	}

public:
	int Decode(const avpacket_t* pkt, avframe_t** yuv)
	{
		if (!CreateDecoder(pkt))
			return -1;

		int r = avdecoder_input(m_decoder, pkt);
		if(r >= 0)
			return avdecoder_getframe(m_decoder, yuv);
		return -1;
	}

private:
	bool CreateDecoder(const avpacket_t* pkt)
	{
		assert(pkt->stream->codecid == AVCODEC_VIDEO_H264 || pkt->stream->codecid == AVCODEC_VIDEO_H265);
		if(NULL == m_decoder)
			m_decoder = pkt->stream->codecid == AVCODEC_VIDEO_H264 ? avdecoder_create_h264(pkt->stream->extra, pkt->stream->bytes) : avdecoder_create_h265(pkt->stream->extra, pkt->stream->bytes);
		return !!m_decoder;
	}

	void DestroyDecoder()
	{
		if (m_decoder)
		{
			avdecoder_destroy(m_decoder);
			m_decoder = NULL;
		}
	}

private:
	void* m_decoder;
};
