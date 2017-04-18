#ifndef _AVFilePlayer_h_
#define _AVFilePlayer_h_

#include "avplayer-file.h"
#include "avpacket.h"
#include "sys/event.h"
#include "sys/atomic.h"
#include "sys/thread.h"
#include "sys/sync.hpp"
#include <list>

class AVFilePlayer
{
public:
	AVFilePlayer(void* window, avplayer_file_read read, void* param);
	~AVFilePlayer();

public:
	void Play();
	void Pause();
	void Reset();

private:
	static uint64_t OnAVRender(void* param, int video, const void* frame, int discard);
	uint64_t OnPlayVideo(const void* video, int discard);
	uint64_t OnPlayAudio(const void* audio, int discard);

	static int STDCALL OnThread(void* param);
	int OnThread();

	void DecodeAudio();
	void DecodeVideo();
	void OnBuffering(bool buffering);

private:
	void* m_window;

	void* m_player;
	void* m_vrender;
	void* m_arender;
	void* m_vdecoder;
	void* m_adecoder;

	bool m_running;
	pthread_t m_thread;
	ThreadEvent m_event;

	bool m_buffering;
	int32_t m_videos, m_audios; // frames in avplayer buffer queue
	typedef std::list<struct avpacket_t*> AVPacketQ;
	AVPacketQ m_audioQ, m_videoQ;

	avplayer_file_read m_reader;
	void* m_param;
};

#endif /* !_AVFilePlayer_h_ */
