#include "AVLivePlayer.h"
#include "avdecoder.h"
#include "avplayer.h"
#include "VOFilter.h"
#include "ctypedef.h"
#include "app-log.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define VIDEO_INTERVAL_MS 40
#define VIDEO_DECODED_FRAMES 2
#define AUDIO_DECODED_FRAMES 2

#define AV_BUFFER_MIN		0
#define AV_BUFFER_MAX		20000
#define AV_BUFFER_DEFAULT	300

AVLivePlayer::AVLivePlayer(void* window)
	: m_window(window)
	, m_play_video(NULL)
	, m_present_video(NULL)
	, m_running(false)
	, m_buffering(true), m_delay(AV_BUFFER_DEFAULT), m_lowlatency(0)
	, m_videos(0), m_audios(0)
	, m_h264_idr(NULL)
	, m_audio_delay("A-delay")
	, m_video_delay("V-delay")
	, m_audioout(new audio_output())
	, m_adecoder(new AudioDecoder())
	, m_vdecoder(new VideoDecoder())
	, m_vfilter(new VOFilter(window))
{
	m_player = avplayer_create(OnAVRender, this);

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

	if (m_present_video)
	{
		avframe_release(m_present_video);
		m_present_video = NULL;
	}

	if (m_play_video)
	{
		avframe_release(m_play_video);
		m_play_video = NULL;
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

int AVLivePlayer::Process(uint64_t clock)
{
	m_clock = clock;
	return avplayer_process(m_player, clock);
}

int AVLivePlayer::Input(struct avpacket_t* pkt)
{
	avpacket_addref(pkt);
	{
		AutoThreadLocker locker(m_locker);
		if (pkt->stream->codecid < AVCODEC_AUDIO_PCM)
		{
			if (pkt->flags & AVPACKET_FLAG_KEY)
				m_h264_idr = pkt->data;
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
	avframe_t* previous = NULL;
	{
		AutoThreadLocker locker(m_locker);
		if (m_play_video)
		{
			previous = m_present_video;
			m_present_video = m_play_video;
			m_play_video = NULL;
		}
	}
	if (previous)
	{
		avframe_release(previous);
	}

	if (m_present_video)
	{
		Present(m_present_video);
	}
}

void AVLivePlayer::Present(struct avframe_t* yuv)
{
	// open and play video in same thread
	int r = m_vfilter.get() ? m_vfilter->Process(yuv) : 0;
	if (0 != r)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] video_output_write(%ld, %ld) => %d\n", __FUNCTION__, yuv->pts, yuv->dts, r);
	}

	m_video_delay.Tick(m_clock, yuv->pts);
}

int STDCALL AVLivePlayer::OnThread(void* param)
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

		if (m_buffering && (GetAudioBuffering() >= m_delay || GetVideoBuffering() > m_delay))
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

	avframe_t *pcm = NULL;
	if (m_adecoder->Decode(pkt, &pcm) >= 0)
	{
		uint64_t duration = pcm->samples * 1000 / pcm->sample_rate;
		avplayer_input_audio(m_player, pcm, pcm->pts, duration, 1);
		atomic_increment32(&m_audios);
		AudioDiscard();
	}
	avpacket_release(pkt);
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

	avframe_t *yuv = NULL;
	if (m_vdecoder->Decode(pkt, &yuv) >= 0)
	{
		if (0 == m_video_delay.m_pts)
			m_video_delay.m_pts = yuv->pts;
		avplayer_input_video(m_player, yuv, yuv->pts, 1);
		atomic_increment32(&m_videos);
		VideoDiscard();
	}
	avpacket_release(pkt);
}

void AVLivePlayer::AudioDiscard()
{
	if (0 == m_lowlatency || m_audios < AUDIO_DECODED_FRAMES || avplayer_get_audio_duration(m_player) < m_lowlatency)
		return;

	int n = 0;
	{
		AutoThreadLocker locker(m_locker);
		for (n = 0; !m_audioQ.empty(); ++n)
		{
			avpacket_release(m_audioQ.front());
			m_audioQ.pop_front();
		}
	}

	if(n > 0) app_log(LOG_WARNING, "Audio discard: %d, v: %d(%d), a: %d(%d)\n", n, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);
}

void AVLivePlayer::VideoDiscard()
{
	if (0 == m_lowlatency || m_videos < VIDEO_DECODED_FRAMES || ( /*m_videos +*/ m_videoQ.size()) * VIDEO_INTERVAL_MS < m_lowlatency)
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

			avpacket_release(pkt);
			m_videoQ.pop_front();
		}

		// check again
		assert(!m_videoQ.empty() && m_videoQ.front()->data == m_h264_idr);
	}
	
	if(n > 0) app_log(LOG_WARNING, "Video discard: %d, v: %d(%d), a: %d(%d)\n", n, m_videoQ.size(), m_videos, m_audioQ.size(), m_audios);
}

uint64_t AVLivePlayer::GetVideoBuffering() const
{
	if (m_videoQ.empty())
		return 0;
	
	AutoThreadLocker locker(m_locker);
	const struct avpacket_t* front = m_videoQ.front();
	if(m_video_delay.m_pts)
		return front->pts - m_video_delay.m_pts;
	else
		return front->pts - m_videoQ.back()->pts;
}

uint64_t AVLivePlayer::GetAudioBuffering() const
{
	int64_t duration = avplayer_get_audio_duration(m_player);
	if (!m_audioQ.empty())
	{
		AutoThreadLocker locker(m_locker);
		const struct avpacket_t* front = m_audioQ.front();
		const struct avpacket_t* back = m_audioQ.back();
		duration += front->pts - back->pts;
	}
	return duration;
}

uint64_t AVLivePlayer::OnAVRender(void* param, int type, const void* frame, int discard)
{
	uint64_t ret;
	AVLivePlayer* player = (AVLivePlayer*)param;

	switch (type)
	{
	case avplayer_render_buffering:
		player->OnBuffering(true);
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

void AVLivePlayer::OnBuffering(bool buffering)
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

uint64_t AVLivePlayer::OnPlayVideo(avframe_t* yuv, int discard)
{
	atomic_decrement32(&m_videos);
	m_event.Signal(); // notify video decode

	if (discard)
		return 0;

	if (m_window)
	{
		Present(yuv);
	}
	else
	{
		avframe_addref(yuv);
		avframe_t* previous = NULL;
		{
			AutoThreadLocker locker(m_locker);
			previous = m_play_video;
			m_play_video = yuv;
		}

		if (previous) 
			avframe_release(previous);
	}
	return 0;
}

uint64_t AVLivePlayer::OnPlayAudio(avframe_t* pcm, int discard)
{
	atomic_decrement32(&m_audios);
	m_event.Signal(); // notify audio decode

	if (discard)
		return 0;

	// Windows 10 Audio Open: ~= 200ms
	if (!m_audioout->isopened() || !m_audioout->check(PCM_SAMPLE_PLANAR(pcm->format) ? 1 : pcm->channels, pcm->sample_rate, pcm->format))
	{
		if (!m_audioout->open(PCM_SAMPLE_PLANAR(pcm->format) ? 1 : pcm->channels, pcm->sample_rate, pcm->format, pcm->sample_rate / 2))
			return 0;
		m_audioout->play();
	}

	if (m_afilter.get()) m_afilter->Process(pcm);

	int r = m_audioout->write(pcm->data[0], pcm->samples);
	if (r != pcm->samples)
	{
		assert(0);
		app_log(LOG_ERROR, "[%s] audio_output_write(%d, %d, %d, %d, %d) => %d\n", __FUNCTION__, pcm->channels, pcm->sample_rate, pcm->sample_bits, pcm->samples, pcm->linesize[0], r);
	}

	// calculate audio buffer sample duration (ms)
	int samples = m_audioout->getframes();
	int duration = (int)((uint64_t)samples * 1000 / pcm->sample_rate);
	m_audio_delay.Tick(m_clock, pcm->pts);
	//app_log(LOG_DEBUG, "[%s] audio_output_getavailablesamples(%d/%dms)\n", __FUNCTION__, samples, duration);

	return samples >= 0 ? duration : 0;
}

int AVLivePlayer::GetAudioSamples() const 
{ 
	return m_audioout->isopened() ? m_audioout->getframes() : 0; 
}
