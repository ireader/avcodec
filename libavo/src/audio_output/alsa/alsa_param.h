#ifndef _alsa_param_h_
#define _alsa_param_h_

#include "alsa_format.h"

static int alsa_hw_param(snd_pcm_t* handle, int format, unsigned int channels, unsigned int *frequency, snd_pcm_uframes_t* frames)
{
	int r, dir = 0;
	snd_pcm_uframes_t period;

	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);

	snd_pcm_hw_params_set_access(handle, params, PCM_SAMPLE_PLANAR(format) ? SND_PCM_ACCESS_RW_NONINTERLEAVED : SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, alsa_format(format));
	snd_pcm_hw_params_set_channels(handle, params, channels);
	snd_pcm_hw_params_set_rate_near(handle, params, frequency, &dir);
	
	// set alsa buffer size
	r = snd_pcm_hw_params_set_buffer_size_near(handle, params, frames);

	// set alsa peroid(latency)
	period = *frequency / 100; // buffer time 10ms
	r = snd_pcm_hw_params_set_period_size_near(handle, params, &period, &dir);

	return r < 0 ? r : snd_pcm_hw_params(handle, params);
}

#endif /* !_alsa_param_h_ */
