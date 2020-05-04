#ifndef _AVLivePlayer_h_
#define _AVLivePlayer_h_

#include <list>
#include <atomic>
#include "sys/event.h"
#include "sys/atomic.h"
#include "sys/thread.h"
#include "sys/sync.hpp"
#include "cpm/shared_ptr.h"
#include "../AVInterval.h"
#include "audio_output.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "AVFilter.h"

class AVLivePlayer
{
public:
	AVLivePlayer(void* window);
	~AVLivePlayer();

public:
	int Process(uint64_t clock);

	int Input(struct avpacket_t* pkt);

	void Present();

	int GetAudioSamples() const;

public:
	void SetAudioFilter(std::shared_ptr<IAudioFilter> filter) { m_afilter = filter; }
	void SetVideoFilter(std::shared_ptr<IVideoFilter> filter) { m_vfilter = filter; }

private:
	static uint64_t OnAVRender(void* param, int type, const void* frame, int discard);
	uint64_t OnPlayVideo(avframe_t* video, int discard);
	uint64_t OnPlayAudio(avframe_t* audio, int discard);

	static int STDCALL OnThread(void* param);
	int OnThread();

	void OnBuffering(bool buffering);
	void VideoDiscard();
	void AudioDiscard();
	uint64_t GetVideoBuffering() const;
	uint64_t GetAudioBuffering() const;

	void Present(struct avframe_t* video);
	void DecodeAudio();
	void DecodeVideo();

private:
	void* m_window;
	void* m_player;
	avframe_t* m_play_video;
	avframe_t* m_present_video;

	bool m_running;
	pthread_t m_thread;
	ThreadEvent m_event;
	mutable ThreadLocker m_locker; // audio/video Queue locker

	bool m_buffering;
	uint32_t m_delay;
	uint32_t m_lowlatency; // enable/disable low latency mode(discard audio/video frames)
	int32_t m_videos, m_audios; // frames in avplayer buffer queue
	typedef std::list<struct avpacket_t*> AVPacketQ;
	AVPacketQ m_audioQ, m_videoQ;
	const uint8_t* m_h264_idr;

	uint64_t m_clock;
	AVInterval m_audio_delay;
	AVInterval m_video_delay;
	
	std::shared_ptr<audio_output> m_audioout;
	std::shared_ptr<AudioDecoder> m_adecoder;
	std::shared_ptr<VideoDecoder> m_vdecoder;
	std::shared_ptr<IAudioFilter> m_afilter;
	std::shared_ptr<IVideoFilter> m_vfilter;
};

#endif /* !_AVLivePlayer_h_ */
