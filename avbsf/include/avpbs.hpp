#ifndef _avpbs_hpp_
#define _avpbs_hpp_

#include "avpbs.h"

// AVPacket Bitstream
class AVPBitStream
{
public:
	static AVPBitStream* Create(int stream, AVPACKET_CODEC_ID codec, const void* extra, int bytes, avpbs_onpacket onpacket, void* param)
	{
		avpbs_t* bs = avpbs_find(codec);
		if (!bs)
			return NULL;

		void* ptr = bs->create(stream, codec, (const uint8_t*)extra, bytes, onpacket, param);
		if (!ptr)
			return NULL;

		return new AVPBitStream(bs, ptr);
	}
	
public:
	~AVPBitStream()
	{
		if (m_bs && m_stream)
		{
			m_bs->destroy(&m_stream);
			m_bs = NULL;
		}
	}

	int Input(int64_t pts, int64_t dts, const void* data, int bytes, int flags)
	{
		return m_bs->input(m_stream, pts, dts, (const uint8_t*)data, bytes, flags);
	}

private:
	AVPBitStream(avpbs_t* bs, void* stream) : m_bs(bs), m_stream(stream){}
	AVPBitStream(AVPBitStream&) {}
	AVPBitStream& operator= (AVPBitStream&) { return *this; }

private:
	avpbs_t* m_bs;
	void* m_stream;
};

#endif /* !_avpbs_hpp_ */
