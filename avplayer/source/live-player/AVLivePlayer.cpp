#include "ctypedef.h"
#include "AVLivePlayer.h"
#include "audio_output.h"
#include "sys/system.h"
#include "h264-util.h"
#include "avdecoder.h"
#include "avplayer.h"
#include "VOFilter.h"
#include "app-log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define VIDEO_INTERVAL_MS 40
#define VIDEO_DECODED_FRAMES 2
#define AUDIO_DECODED_FRAMES 2

#define avpacket_free(pkt) free(pkt)

AVLivePlayer::AVLivePlayer(void* window)
	: m_window(window)
	, m_arender(NULL)
	, m_play_video(NULL), m_present_video(NULL)
	, m_running(false)
	, m_buffering(true), m_delay(100)
	, m_videos(0), m_audios(0)
	, m_h264_idr(NULL)
	, m_audio_delay("A-delay")
	, m_video_delay("V-delay")
	, m_vfilter(new VOFilter(window))
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

	if (m_present_video && m_present_video != m_play_video)
		avdecoder_freeframe(m_vdecoder, m_present_video);
	m_present_video = NULL;

	if (m_play_video)
	{
		avdecoder_freeframe(m_vdecoder, m_play_video);
		m_play_video = NULL;
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

int AVLivePlayer::Input(struct avpacket_t* pkt, bool video)
{
	{
		AutoThreadLocker locker(m_locker);
		if (video)
		{
			// TODO: check frame number
			if (h264_idr(pkt->data, pkt->bytes))
			{
				m_h264_idr = pkt->data;
			}
			m_videoQ.push_back(pkt);
		}
		else
		{
			m_audioQ.push_back(pkt);
		}

		//app_log(LOG_DEBUG, "[%s] input%s(pts: %" PRId64 ", dts: %" PRId64 ") => v: %d(%d), a: %d(%d)\n", __FUNCTION__, video ? "video" : "audio", pkt->pts, pkt->dts, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);
	}

	m_event.Signal();
	return 0;
}

void AVLivePlayer::Present()
{
	{
		AutoThreadLocker locker(m_locker);
		if(m_present_video && m_present_video != m_play_video)
			avdecoder_freeframe(m_vdecoder, m_present_video);
		m_present_video = m_play_video;
	}

	if (m_present_video)
	{
		Present(m_present_video);
	}
}

void AVLivePlayer::Present(void* video)
{
	struct avframe_t frame;
	avdecoder_frame_to(video, &frame);

	//uint8_t* u = frame.data[1];
	//frame.data[1] = frame.data[2];
	//frame.data[2] = u;

	// open and play video in same thread
	int r = m_vfilter.get() ? m_vfilter->Process(&frame) : 0;
	if (0 != r)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] video_output_write(%ld, %ld) => %d\n", __FUNCTION__, frame.pts, frame.dts, r);
	}

	m_video_delay.Tick((int)(system_time() - frame.pts));

	//avdecoder_freeframe(m_vdecoder, (void*)video);
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
	struct avpacket_t* pkt;
	{
		AutoThreadLocker locker(m_locker);
		pkt = m_audioQ.front();
		m_audioQ.pop_front();
		//app_log(LOG_DEBUG, "[%s] pts: %" PRId64 ", dts: %" PRId64 "\n", __FUNCTION__, pkt->pts, pkt->dts);
	}

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
			AudioDiscard();
		}
	}
}

void AVLivePlayer::DecodeVideo()
{
	struct avpacket_t* pkt;
	{
		AutoThreadLocker locker(m_locker);
		pkt = m_videoQ.front();
		m_videoQ.pop_front();
		if (m_h264_idr == pkt->data)
			m_h264_idr = NULL; // clear last video IDR frame flags
		//app_log(LOG_DEBUG, "[%s] pts: %" PRId64 ", dts: %" PRId64 "\n", __FUNCTION__, pkt->pts, pkt->dts);
	}

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
			VideoDiscard();
		}
	}
}

void AVLivePlayer::AudioDiscard()
{
	if (m_audios < AUDIO_DECODED_FRAMES || avplayer_get_audio_duration(m_player) < m_delay)
		return;

	int n = 0;
	{
		AutoThreadLocker locker(m_locker);
		for (n = 0; !m_audioQ.empty(); ++n)
		{
			struct avpacket_t* pkt = m_audioQ.front();
			avpacket_free(pkt);
			m_audioQ.pop_front();
		}
	}

	if(n > 0) app_log(LOG_WARNING, "Audio discard: %d, v: %d(%d), a: %d(%d)\n", n, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);
}

void AVLivePlayer::VideoDiscard()
{
	if (m_videos < VIDEO_DECODED_FRAMES || (m_videos + m_videoQ.size()) * VIDEO_INTERVAL_MS < m_delay)
		return;

	int n = 0;
	{
		AutoThreadLocker locker(m_locker);
		if (!m_h264_idr)
			return; // don't have h264 IDR packet

		for (n = 0; !m_videoQ.empty(); ++n)
		{
			struct avpacket_t* pkt = m_videoQ.front();
			if (pkt->data == m_h264_idr)
				break;

			avpacket_free(pkt);
			m_videoQ.pop_front();
		}

		// check again
		assert(!m_videoQ.empty() && m_videoQ.front()->data == m_h264_idr);
	}
	
	if(n > 0) app_log(LOG_WARNING, "Video discard: %d, v: %d(%d), a: %d(%d)\n", n, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);
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

	if (m_window)
	{
		Present((void*)video);
		avdecoder_freeframe(m_vdecoder, (void*)video);
	}
	else
	{
		AutoThreadLocker locker(m_locker);
		if (m_play_video && m_present_video != m_play_video)
		{
			avdecoder_freeframe(m_vdecoder, m_play_video);
		}
		m_play_video = (void*)video;
	}
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
	if (m_afilter.get()) m_afilter->Process(&frame);

	// Windows 10 Audio Open: ~= 200ms
	if (NULL == m_arender)
	{
		m_arender = audio_output_open(1/*frame.channels*/, frame.sample_bits, frame.sample_rate, frame.sample_rate/5);
		if (NULL == m_arender) return 0;
		audio_output_play(m_arender);
	}

	int r = audio_output_write(m_arender, frame.data[0], frame.samples);
	if (r <= 0)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] audio_output_write(%d, %ld, %ld) => %d\n", __FUNCTION__, frame.linesize[0], frame.pts, frame.dts, r);
	}

	// calculate audio buffer sample duration (ms)
	int samples = audio_output_getsamples(m_arender);
	int duration = (uint64_t)samples * 1000 / frame.sample_rate;
	m_audio_delay.Tick((int)(system_time() - frame.pts) + duration);
	//app_log(LOG_DEBUG, "[%s] audio_output_getavailablesamples(%d/%dms)\n", __FUNCTION__, samples, duration);

	avdecoder_freeframe(m_adecoder, (void*)audio);
	return samples >= 0 ? duration : 0;
}
