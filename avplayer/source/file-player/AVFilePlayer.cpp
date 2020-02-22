#include "AVFilePlayer.h"
#include "avdecoder.h"
#include "avplayer.h"
#include "VOFilter.h"
#include "app-log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

AVFilePlayer::AVFilePlayer(void* window, avplayer_file_read reader, void* param)
	: m_window(window)
	, m_running(false)
	, m_videos(0), m_audios(0)
	, m_reader(reader), m_param(param)
	, m_audioout(new audio_output())
	, m_adecoder(new AudioDecoder())
	, m_vdecoder(new VideoDecoder())
	, m_vfilter(new VOFilter(window))
{
	m_pts = 0;
	m_player = avplayer_create(OnAVRender, this);

	m_running = true;
	thread_create(&m_thread, OnThread, this);
}

AVFilePlayer::~AVFilePlayer()
{
	if (m_running)
	{
		m_running = false;
		thread_destroy(m_thread);
	}

	if (m_player)
	{
		avplayer_destroy(m_player);
		m_player = NULL;
	}

	while (!m_audioQ.empty())
	{
		avpacket_release(m_audioQ.front());
		m_audioQ.pop_front();
	}

	while (!m_videoQ.empty())
	{
		avpacket_release(m_videoQ.front());
		m_videoQ.pop_front();
	}
}

int AVFilePlayer::Process(uint64_t clock)
{
	return avplayer_process(m_player, clock);
}

void AVFilePlayer::Play()
{
	avplayer_play(m_player);
}

void AVFilePlayer::Pause()
{
	avplayer_pause(m_player);
}

void AVFilePlayer::Reset()
{
	// TODO: reset flags
	// 1. clear audio buffer
	// 2. discard audio frames(pkt sn)
	// 3. discard video frames(pkt sn)
}

uint64_t AVFilePlayer::GetPosition() const
{
	return m_pts;
}

int AVFilePlayer::SetSpeed(int speed)
{
	m_speed = speed;
	return 0;
}

int AVFilePlayer::GetSpeed() const
{
	return m_speed;
}

int STDCALL AVFilePlayer::OnThread(void* param)
{
	AVFilePlayer* player = (AVFilePlayer*)param;
	return player->OnThread();
}

int AVFilePlayer::OnThread()
{
	bool needmoreframe = false;
	while (m_running)
	{
		while (m_videoQ.size() + m_audioQ.size() < 20 && m_reader)
		{
			struct avpacket_t* pkt = m_reader(m_param);
			if (NULL == pkt)
				break;

			if (pkt->stream->codecid < AVCODEC_AUDIO_PCM)
				m_videoQ.push_back(pkt);
			else
				m_audioQ.push_back(pkt);
		}

		if (m_videos < 2 && !m_videoQ.empty())
		{
			DecodeVideo();
			needmoreframe = true;
		}

		if (m_audios < 2 && !m_audioQ.empty())
		{
			DecodeAudio();
			needmoreframe = true;
		}

		if(!needmoreframe)
			m_event.TimeWait(100);
	}

	return 0;
}

void AVFilePlayer::DecodeAudio()
{
	struct avpacket_t* pkt = m_audioQ.front();
	m_audioQ.pop_front();

	avframe_t *pcm = NULL;
	if (m_adecoder->Decode(pkt, &pcm) >= 0)
	{
		uint64_t duration = pcm->samples * 1000 / pcm->sample_rate;
		avplayer_input_audio(m_player, pcm, pcm->pts, duration, 1);
		atomic_increment32(&m_audios);
	}
	avpacket_release(pkt);
}

void AVFilePlayer::DecodeVideo()
{
	struct avpacket_t* pkt = m_videoQ.front();
	m_videoQ.pop_front();

	avframe_t *yuv = NULL;
	if (m_vdecoder->Decode(pkt, &yuv) >= 0)
	{
		avplayer_input_video(m_player, yuv, yuv->pts, 1);
		atomic_increment32(&m_videos);
	}
	avpacket_release(pkt);
}

uint64_t AVFilePlayer::OnAVRender(void* param, int type, const void* frame, int discard)
{
	uint64_t ret;
	AVFilePlayer* player = (AVFilePlayer*)param;

	switch (type)
	{
	case avplayer_render_buffering:
		//player->OnBuffering(true);
		break;

	case avplayer_render_audio:
		ret = player->OnPlayAudio((avframe_t*)frame, discard);
		avframe_release((avframe_t*)frame);
		return ret;

	case avplayer_render_video:
		ret = player->OnPlayVideo((avframe_t*)frame, discard);
		avframe_release((avframe_t*)frame);
		return ret;

	default:
		assert(0);
	}

	return 0;
}

void AVFilePlayer::OnBuffering(bool buffering)
{
	if (buffering)
	{
		avplayer_pause(m_player);
		m_audioout->pause();
	}
	else
	{
		avplayer_play(m_player);
		m_audioout->play();
	}

	m_buffering = buffering;
	app_log(LOG_INFO, "[%s] %s\n", __FUNCTION__, buffering ? "true" : "false");
}

uint64_t AVFilePlayer::OnPlayVideo(avframe_t* yuv, int discard)
{
	atomic_decrement32(&m_videos);
	m_event.Signal(); // notify video decode
	m_pts = yuv->pts;

	if (discard)
		return 0;

	//uint8_t* u = frame.data[1];
	//frame.data[1] = frame.data[2];
	//frame.data[2] = u;

	// open and play video in same thread
	int r = m_vfilter.get() ? m_vfilter->Process(yuv) : 0;
	if (0 != r)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] video_output_write(%u, %u) => %d\n", __FUNCTION__, (unsigned int)yuv->pts, (unsigned int)yuv->dts, r);
	}

	return 0;
}

uint64_t AVFilePlayer::OnPlayAudio(avframe_t* pcm, int discard)
{
	atomic_decrement32(&m_audios);
	m_event.Signal(); // notify audio decode
	if(pcm->pts - m_pts > 500)
		m_pts = pcm->pts;

	if (discard)
		return 0;

	if (m_afilter.get()) m_afilter->Process(pcm);

	if (!m_audioout->isopened() || !m_audioout->check(PCM_SAMPLE_PLANAR(pcm->format) ? 1 : pcm->channels, pcm->sample_rate, pcm->format))
	{
		if (!m_audioout->open(PCM_SAMPLE_PLANAR(pcm->format) ? 1 : pcm->channels, pcm->sample_rate, pcm->format, pcm->sample_rate))
			return 0;
		m_audioout->play();
	}

	int r = m_audioout->write(pcm->data[0], pcm->samples);
	if (r != pcm->samples)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] audio_output_write(%d, %ld, %ld) => %d\n", __FUNCTION__, pcm->linesize[0], pcm->pts, pcm->dts, r);
	}
	
	// calculate audio buffer sample duration (ms)
	int samples = m_audioout->getframes();
	//app_log(LOG_DEBUG, "[%s] audio_output_getavailablesamples(%d/%d)\n", __FUNCTION__, samples, (int)((uint64_t)samples * 1000 / pcm->sample_rate));
	return samples >= 0 ? (uint64_t)samples * 1000 / pcm->sample_rate : 0;
}
