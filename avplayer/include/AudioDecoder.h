#pragma once

#include "audio-decoder.h"
#include "avdecoder.h"
#include "avstream.h"

class AudioDecoder
{
public:
	AudioDecoder()
	{
		m_codecid = AVCODEC_NONE;
		m_audio_class = NULL;
		m_decoder = NULL;
	}

	~AudioDecoder()
	{
		DestroyDecoder();
	}

public:
	int Decode(avpacket_t* pkt, avframe_t** pcm)
	{
		if (!CreateDecoder(pkt))
			return -1;

		int r = m_audio_class->decode(m_decoder, pkt);
		if (r >= 0)
			return m_audio_class->getframe(m_decoder, pcm);
		return -1;
	}

private:
	bool CreateDecoder(const avpacket_t* pkt)
	{
		if (m_codecid != pkt->stream->codecid)
			DestroyDecoder();

		if (NULL == m_decoder)
		{
			m_codecid = pkt->stream->codecid;

			if (AVCODEC_AUDIO_AAC == pkt->stream->codecid)
			{
				m_audio_class = aac_decoder();
				m_decoder = m_audio_class->create(PCM_SAMPLE_FMT_S16, pkt->stream->channels, pkt->stream->sample_rate, pkt->stream->extra, pkt->stream->bytes);
			}
			else if (AVCODEC_AUDIO_OPUS == pkt->stream->codecid)
			{
				m_audio_class = opus_decoder();
				m_decoder = m_audio_class->create(PCM_SAMPLE_FMT_S16, pkt->stream->channels, pkt->stream->sample_rate, pkt->stream->extra, pkt->stream->bytes);
			}
			else if (AVCODEC_AUDIO_MP3 == pkt->stream->codecid)
			{
				m_audio_class = mp3_decoder();
				m_decoder = m_audio_class->create(PCM_SAMPLE_FMT_S16, pkt->stream->channels, pkt->stream->sample_rate, pkt->stream->extra, pkt->stream->bytes);
			}
			else if (AVCODEC_AUDIO_G711A == pkt->stream->codecid || AVCODEC_AUDIO_G711U == pkt->stream->codecid)
			{
				m_audio_class = AVCODEC_AUDIO_G711A == pkt->stream->codecid ? g711a_decoder() : g711u_decoder();
				m_decoder = m_audio_class->create(PCM_SAMPLE_FMT_S16, pkt->stream->channels, pkt->stream->sample_rate, pkt->stream->extra, pkt->stream->bytes);
			}
		}

		return !!m_decoder;
	}

	void DestroyDecoder()
	{
		if (m_audio_class && m_decoder)
			m_audio_class->destroy(m_decoder);
		m_audio_class = NULL;
		m_decoder = NULL;
		m_codecid = AVCODEC_NONE;
	}

private:
	void* m_decoder;
	enum AVPACKET_CODEC_ID m_codecid;
	struct audio_decoder_t* m_audio_class;
};
