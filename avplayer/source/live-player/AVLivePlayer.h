#ifndef _AVLivePlayer_h_
#define _AVLivePlayer_h_

#include "avpacket.h"
#include "sys/event.h"
#include "sys/atomic.h"
#include "sys/thread.h"
#include "sys/sync.hpp"
#include <list>
#include "cpm/shared_ptr.h"
#include "../AVInterval.h"
#include "AVFilter.h"

class AVLivePlayer
{
public:
	AVLivePlayer(void* window);
	~AVLivePlayer();

public:
	int Input(struct avpacket_t* pkt, bool video);

	void Present();

public:
	void SetAudioFilter(std::shared_ptr<IAudioFilter> filter) { m_afilter = filter; }
	void SetVideoFilter(std::shared_ptr<IVideoFilter> filter) { m_vfilter = filter; }

private:
	static uint64_t OnAVRender(void* param, int type, const void* frame, int discard);
	uint64_t OnPlayVideo(const void* frame, int discard);
	uint64_t OnPlayAudio(const void* pcm, int discard);

	static int STDCALL OnThread(void* param);
	int OnThread();

	void OnBuffering(bool buffering);
	void VideoDiscard();
	void AudioDiscard();

	void Present(void* video);
	void DecodeAudio();
	void DecodeVideo();

private:
	void* m_window;

	void* m_player;
	void* m_arender;
	void* m_vdecoder;
	void* m_adecoder;
	void* m_play_video;
	void* m_present_video;

	bool m_running;
	pthread_t m_thread;
	ThreadEvent m_event;
	ThreadLocker m_locker; // audio/video Queue locker

	bool m_buffering;
	uint32_t m_delay;
	int32_t m_videos, m_audios; // frames in avplayer buffer queue
	typedef std::list<struct avpacket_t*> AVPacketQ;
	AVPacketQ m_audioQ, m_videoQ;
	const uint8_t* m_h264_idr;

	AVInterval m_audio_delay;
	AVInterval m_video_delay;
	
	std::shared_ptr<IAudioFilter> m_afilter;
	std::shared_ptr<IVideoFilter> m_vfilter;
};

#endif /* !_AVLivePlayer_h_ */
