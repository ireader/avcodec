#ifndef _direct_capture_8_h_
#define _direct_capture_8_h_

#include <dsound.h>
#include "audio_input.h"

#define MAX_EVENTS 5

struct direct_capture_t
{
	int samples;
	int bytes_per_sample;

	audio_input_callback cb;
	void* param;

	HANDLE thread;
	HANDLE events[MAX_EVENTS+1];
	LPDIRECTSOUNDCAPTURE8 sound;
	LPDIRECTSOUNDCAPTUREBUFFER8 dsb;
};

#endif /* !_direct_capture_8_h_ */
