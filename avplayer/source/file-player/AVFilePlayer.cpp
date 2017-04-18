#include "AVFilePlayer.h"
#include "video_output.h"
#include "audio_output.h"
#include "avdecoder.h"
#include "avplayer.h"
#include "app-log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define avpacket_free(pkt) free(pkt)

AVFilePlayer::AVFilePlayer(void* window, avplayer_file_read reader, void* param)
	:m_window(window)
	,m_vrender(NULL), m_arender(NULL)
	,m_running(false)
	,m_videos(0), m_audios(0)
	,m_reader(reader), m_param(param)
{
	m_player = avplayer_create(OnAVRender, this);
	m_vdecoder = avdecoder_create_h264();
	m_adecoder = avdecoder_create_aac();

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

	if (m_arender)
	{
		audio_output_close(m_arender);
		m_arender = NULL;
	}

	if (m_vrender)
	{
		video_output_close(m_vrender);
		m_vrender = NULL;
	}

	if (m_adecoder)
	{
		avdecoder_destroy(m_adecoder);
		m_adecoder = NULL;
	}

	if (m_vdecoder)
	{
		avdecoder_destroy(m_vdecoder);
		m_vdecoder = NULL;
	}

	while (!m_audioQ.empty())
	{
		avpacket_free(m_audioQ.front());
		m_audioQ.pop_front();
	}

	while (!m_videoQ.empty())
	{
		avpacket_free(m_videoQ.front());
		m_videoQ.pop_front();
	}
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
		int type = 0;
		struct avpacket_t* pkt;
		while (m_videoQ.size() + m_audioQ.size() < 20
			&& m_reader && m_reader(m_param, &pkt, &type) > 0)
		{
			0 == type ? m_audioQ.push_back(pkt) : m_videoQ.push_back(pkt);
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

	int r = avdecoder_input(m_adecoder, pkt);
	avpacket_free(pkt);
	if (r >= 0)
	{
		void* frame = avdecoder_getframe(m_adecoder);
		if (NULL != frame)
		{
			struct avframe_t pcm;
			avdecoder_frame_to(frame, &pcm);
			uint64_t duration = pcm.samples * 1000 / pcm.sample_rate;
			avplayer_input_audio(m_player, frame, pcm.pts, duration, 1);
			atomic_increment32(&m_audios);
		}
	}
}

void AVFilePlayer::DecodeVideo()
{
	struct avpacket_t* pkt = m_videoQ.front();
	m_videoQ.pop_front();

	int r = avdecoder_input(m_vdecoder, pkt);
	avpacket_free(pkt);
	if (r >= 0)
	{
		void* frame = avdecoder_getframe(m_vdecoder);
		if (NULL != frame)
		{
			struct avframe_t yuv;
			avdecoder_frame_to(frame, &yuv);
			avplayer_input_video(m_player, frame, yuv.pts, 1);
			atomic_increment32(&m_videos);
		}
	}
}

uint64_t AVFilePlayer::OnAVRender(void* param, int type, const void* frame, int discard)
{
	AVFilePlayer* player = (AVFilePlayer*)param;

	switch (type)
	{
	case avplayer_render_buffering:
		//player->OnBuffering(true);
		break;

	case avplayer_render_audio:
		return player->OnPlayAudio(frame, discard);

	case avplayer_render_video:
		return player->OnPlayVideo(frame, discard);

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
		if (m_arender) audio_output_pause(m_arender);
	}
	else
	{
		avplayer_play(m_player);
		if (m_arender) audio_output_play(m_arender);
	}

	m_buffering = buffering;
	app_log(LOG_INFO, "[%s] %s\n", __FUNCTION__, buffering ? "true" : "false");
}

uint64_t AVFilePlayer::OnPlayVideo(const void* video, int discard)
{
	atomic_decrement32(&m_videos);
	m_event.Signal(); // notify video decode

	if (discard)
	{
		avdecoder_freeframe(m_vdecoder, (void*)video);
		return 0;
	}

	struct avframe_t frame;
	avdecoder_frame_to(video, &frame);

	// open and play video in same thread
	if (NULL == m_vrender)
	{
		m_vrender = video_output_open(m_window, frame.format, frame.width, frame.height);
		if (NULL == m_vrender) return 0;
	}

	uint8_t* u = frame.data[1];
	frame.data[1] = frame.data[2];
	frame.data[2] = u;
	int r = video_output_write(m_vrender, &frame, 0, 0, 0, 0, 0, 0, 0, 0);
	if (0 != r)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] video_output_write(%ld, %ld) => %d\n", __FUNCTION__, frame.pts, frame.dts, r);
	}

	avdecoder_freeframe(m_vdecoder, (void*)video);
	return 0;
}

uint64_t AVFilePlayer::OnPlayAudio(const void* audio, int discard)
{
	atomic_decrement32(&m_audios);
	m_event.Signal(); // notify audio decode

	if (discard)
	{
		avdecoder_freeframe(m_adecoder, (void*)audio);
		return 0;
	}

	struct avframe_t frame;
	avdecoder_frame_to(audio, &frame);

	if (NULL == m_arender)
	{
		m_arender = audio_output_open(1/*frame.channel*/, frame.sample_bits, frame.sample_rate);
		if (NULL == m_arender) return 0;
		audio_output_play(m_arender);
	}

	int r = audio_output_write(m_arender, frame.data[0], frame.samples);
	if (r <= 0)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] audio_output_write(%d, %ld, %ld) => %d\n", __FUNCTION__, frame.linesize[0], frame.pts, frame.dts, r);
	}
	
	int sample_rate = frame.sample_rate;
	avdecoder_freeframe(m_adecoder, (void*)audio);

	// calculate audio buffer sample duration (ms)
	int samples = audio_output_getavailablesamples(m_arender);
	//app_log(LOG_DEBUG, "[%s] audio_output_getavailablesamples(%d/%d)\n", __FUNCTION__, samples, (int)((uint64_t)samples * 1000 / sample_rate));
	return samples >= 0 ? (uint64_t)samples * 1000 / sample_rate : 0;
}
