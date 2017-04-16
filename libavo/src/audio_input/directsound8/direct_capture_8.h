#ifndef _direct_capture_8_h_
#define _direct_capture_8_h_

#include <dsound.h>
#include "audio_input.h"

#define MAX_EVENTS 25

class DxSound8In
{
public:
	DxSound8In(audio_input_callback cb, void* param);
	~DxSound8In();

public:
	int Open(int channels, int bitsPerSamples, int samplesPerSec);
	int Close();
	int Stop();
	int Start();
	bool IsOpened() const;

private:
	static DWORD WINAPI OnWorker(LPVOID lpParameter);
	DWORD OnWorker();

private:
	int m_channels;
	int m_bitsPerSample;
	int m_samplesPerSec;

	audio_input_callback m_cb;
	void* m_param;

	HANDLE m_thread;
	HANDLE m_events[MAX_EVENTS+1];
	LPDIRECTSOUNDCAPTURE8 m_sound;
	LPDIRECTSOUNDCAPTUREBUFFER8 m_dsb;
};

#endif /* !_direct_capture_8_h_ */
