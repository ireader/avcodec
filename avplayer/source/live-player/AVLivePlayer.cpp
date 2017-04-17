#include "ctypedef.h"
#include "AVLivePlayer.h"
#include "video_output.h"
#include "audio_output.h"
#include "h264-util.h"
#include "avdecoder.h"
#include "avplayer.h"
#include "app-log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define VIDEO_INTERVAL_MS 40
#define VIDEO_DECODED_FRAMES 2
#define AUDIO_DECODED_FRAMES 2

AVLivePlayer::AVLivePlayer(void* window)
	: m_window(window)
	, m_vrender(NULL), m_arender(NULL)
	, m_last_video(NULL)
	, m_running(false)
	, m_buffering(true), m_delay(100)
	, m_videos(0), m_audios(0)
{
	m_player = avplayer_create(OnAVRender, this);
	m_vdecoder = avdecoder_create_h264();
	m_adecoder = avdecoder_create_aac();

	m_running = true;
	thread_create(&m_thread, OnThread, this);
}

AVLivePlayer::~AVLivePlayer()
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
}

int AVLivePlayer::Input(struct avpacket_t* pkt, bool video)
{
	{
		AutoThreadLocker locker(m_locker);
		video ? m_videoQ.push_back(*pkt) : m_audioQ.push_back(*pkt);
		//app_log(LOG_DEBUG, "[%s] input%s(pts: %" PRId64 ", dts: %" PRId64 ") => v: %d(%d), a: %d(%d)\n", __FUNCTION__, video ? "video" : "audio", pkt->pts, pkt->dts, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);
	}

	m_event.Signal();
	return 0;
}

void AVLivePlayer::Present()
{
	void* video = NULL;
	{
		AutoThreadLocker locker(m_locker);
		video = m_last_video;
		m_last_video = NULL;
	}

	if (video)
	{
		Present(video);
	}
}

void AVLivePlayer::Present(void* video)
{
	struct avframe_t frame;
	avdecoder_frame_to(video, &frame);

	// open and play video in same thread
	if (NULL == m_vrender)
	{
		// don't create EGL context
		m_vrender = video_output_open(m_window, frame.format, frame.width, frame.height);
		if (NULL == m_vrender) return;
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
}

int STDCALL AVLivePlayer::OnThread(void*  param)
{
	AVLivePlayer* player = (AVLivePlayer*)param;
	return player->OnThread();
}

int AVLivePlayer::OnThread()
{
	bool needmoreframe = false;
	while (m_running)
	{
		assert(m_videos >= 0 && m_audios >= 0);
		if ((m_buffering || m_videos < VIDEO_DECODED_FRAMES) && !m_videoQ.empty())
		{
			DecodeVideo();
			needmoreframe = true;
		}

		if ((m_buffering || m_audios < VIDEO_DECODED_FRAMES) && !m_audioQ.empty())
		{
			DecodeAudio();
			needmoreframe = true;
		}

		if (m_buffering && ((uint32_t)m_videos * VIDEO_INTERVAL_MS > m_delay || avplayer_get_audio_duration(m_player) >= m_delay))
		{
			OnBuffering(false);
		}

		if (!needmoreframe)
			m_event.TimeWait(100);
	}

	return 0;
}

void AVLivePlayer::DecodeAudio()
{
	struct avpacket_t pkt;
	{
		AutoThreadLocker locker(m_locker);
		pkt = m_audioQ.front();
		m_audioQ.pop_front();
		//app_log(LOG_DEBUG, "[%s] pts: %" PRId64 ", dts: %" PRId64 "\n", __FUNCTION__, pkt.pts, pkt.dts);
	}

	int r = avdecoder_input(m_adecoder, &pkt);
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
			AudioDiscard();
		}
	}

	free(pkt.data);
}

void AVLivePlayer::DecodeVideo()
{
	struct avpacket_t pkt;
	{
		AutoThreadLocker locker(m_locker);
		pkt = m_videoQ.front();
		m_videoQ.pop_front();
		//app_log(LOG_DEBUG, "[%s] pts: %" PRId64 ", dts: %" PRId64 "\n", __FUNCTION__, pkt.pts, pkt.dts);
	}

	int r = avdecoder_input(m_vdecoder, &pkt);
	if (r >= 0)
	{
		void* frame = avdecoder_getframe(m_vdecoder);
		if (NULL != frame)
		{
			struct avframe_t yuv;
			avdecoder_frame_to(frame, &yuv);
			avplayer_input_video(m_player, frame, yuv.pts, 1);
			atomic_increment32(&m_videos);
			VideoDiscard();
		}
	}

	free(pkt.data);
}

void AVLivePlayer::AudioDiscard()
{
	if (m_audios < AUDIO_DECODED_FRAMES || avplayer_get_audio_duration(m_player) < m_delay)
		return;

	AutoThreadLocker locker(m_locker);
	while (m_audioQ.size() > 1)
	{
		struct avpacket_t& pkt = m_audioQ.back();
		free(pkt.data);
		app_log(LOG_WARNING, "[%s] discard pts: %" PRId64 ", dts: %" PRId64 ", v: %d(%d), a: %d(%d)\n", __FUNCTION__, pkt.pts, pkt.dts, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);
		m_audioQ.pop_back();
	}
}

void AVLivePlayer::VideoDiscard()
{
	if (m_videos < VIDEO_DECODED_FRAMES || (m_videos + m_videoQ.size()) * VIDEO_INTERVAL_MS < m_delay)
		return;

	AVPacketQ::reverse_iterator it;
	AutoThreadLocker locker(m_locker);
	for (it = m_videoQ.rbegin(); it != m_videoQ.rend(); ++it)
	{
		struct avpacket_t& idr = *it;
		if (h264_idr(idr.data, idr.bytes))
		{
			AVPacketQ::reverse_iterator pkt = m_videoQ.rbegin();
			while (pkt->data != idr.data)
			{
				free(pkt->data);
				app_log(LOG_WARNING, "[%s] discard pts: %" PRId64 ", dts: %" PRId64 ", v: %d(%d), a: %d(%d)\n", __FUNCTION__, pkt->pts, pkt->dts, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);

				m_videoQ.pop_back();
				pkt = m_videoQ.rbegin();
			}

			it = m_videoQ.rbegin();
		}
	}
}

uint64_t AVLivePlayer::OnAVRender(void* param, int type, const void* frame, int discard)
{
	AVLivePlayer* player = (AVLivePlayer*)param;

	switch (type)
	{
	case avplayer_render_buffering:
		player->OnBuffering(true);
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

void AVLivePlayer::OnBuffering(bool buffering)
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

uint64_t AVLivePlayer::OnPlayVideo(const void* video, int discard)
{
	atomic_decrement32(&m_videos);
	m_event.Signal(); // notify video decode

	if (discard)
	{
		avdecoder_freeframe(m_vdecoder, (void*)video);
		return 0;
	}

#if 1
	Present((void*)video);
#else
	AutoThreadLocker locker(m_locker);
	if (m_last_video)
	{
		avdecoder_freeframe(m_vdecoder, m_last_video);
	}
	m_last_video = (void*)video;
#endif
	return 0;
}

uint64_t AVLivePlayer::OnPlayAudio(const void* audio, int discard)
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

	// Windows 10 Audio Open: ~= 200ms
	if (NULL == m_arender)
	{
		m_arender = audio_output_open(1/*frame.channels*/, frame.sample_bits, frame.sample_rate);
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
	//app_log(LOG_DEBUG, "[%s] audio_output_getavailablesamples(%d/%dms)\n", __FUNCTION__, samples, (int)((uint64_t)samples * 1000 / sample_rate));
	return samples >= 0 ? (uint64_t)samples * 1000 / sample_rate : 0;
}
