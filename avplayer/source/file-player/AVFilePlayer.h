#ifndef _AVFilePlayer_h_
#define _AVFilePlayer_h_

#include <list>
#include <atomic>
#include "cpm/shared_ptr.h"
#include "sys/sync.hpp"
#include "avplayer-file.h"
#include "sys/event.h"
#include "sys/atomic.h"
#include "sys/thread.h"
#include "audio_output.h"
#include "AudioDecoder.h"
#include "VideoDecoder.h"
#include "AVFilter.h"

class AVFilePlayer
{
public:
	AVFilePlayer(void* window, avplayer_file_read read, void* param);
	~AVFilePlayer();

public:
	int Process(uint64_t clock);
	uint64_t GetPosition() const;

	void Play();
	void Pause();
	void Reset();

	int SetSpeed(int speed);
	int GetSpeed() const;

public:
	void SetAudioFilter(std::shared_ptr<IAudioFilter> filter) { m_afilter = filter; }
	void SetVideoFilter(std::shared_ptr<IVideoFilter> filter) { m_vfilter = filter; }

private:
	static uint64_t OnAVRender(void* param, int video, const void* frame, int discard);
	uint64_t OnPlayVideo(avframe_t* video, int discard);
	uint64_t OnPlayAudio(avframe_t* audio, int discard);

	static int STDCALL OnThread(void* param);
	int OnThread();

	void DecodeAudio();
	void DecodeVideo();
	void OnBuffering(bool buffering);

private:
	void* m_window;
	void* m_player;

	bool m_running;
	pthread_t m_thread;
	ThreadEvent m_event;

	bool m_buffering;
	int32_t m_videos, m_audios; // frames in avplayer buffer queue
	typedef std::list<struct avpacket_t*> AVPacketQ;
	AVPacketQ m_audioQ, m_videoQ;

	int m_speed;
	int64_t m_pts;	
	avplayer_file_read m_reader;
	void* m_param;

	std::shared_ptr<audio_output> m_audioout;
	std::shared_ptr<AudioDecoder> m_adecoder;
	std::shared_ptr<VideoDecoder> m_vdecoder;
	std::shared_ptr<IAudioFilter> m_afilter;
	std::shared_ptr<IVideoFilter> m_vfilter;
};

#endif /* !_AVFilePlayer_h_ */
