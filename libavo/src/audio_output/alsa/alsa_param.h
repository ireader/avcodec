#ifndef _alsa_param_h_
#define _alsa_param_h_

#include "alsa_format.h"

static int alsa_hw_param(snd_pcm_t* handle, int format, unsigned int channels, unsigned int *frequency, snd_pcm_uframes_t* frames)
{
	//snd_pcm_uframes_t size;
	snd_pcm_uframes_t period;
	//snd_pcm_uframes_t period20ms;

	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);

	snd_pcm_hw_params_set_access(handle, params, PCM_SAMPLE_PLANAR(format) ? SND_PCM_ACCESS_RW_NONINTERLEAVED : SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, alsa_format(format));
	snd_pcm_hw_params_set_channels(handle, params, channels);
	snd_pcm_hw_params_set_rate_near(handle, params, frequency, 0);
	
	// set alsa buffer size(maximum latency)
	//if(0 == snd_pcm_hw_params_get_buffer_size_max(params, &size))
	//	*frames = *frames > size ? size : *frames;
	snd_pcm_hw_params_set_buffer_size_near(handle, params, frames);
	
	// set alsa peroid
	// http://users.suse.com/~mana/alsa090_howto.html
	// http://0pointer.de/blog/projects/all-about-periods.html
	//period20ms = *frequency / 50; // period time 20ms
	//if(0 == snd_pcm_hw_params_get_period_size_min(params, &period, 0))
	//	period = period > period20ms ? period : period20ms;
	snd_pcm_hw_params_set_period_size_near(handle, params, &period, 0);

	return snd_pcm_hw_params(handle, params);
}

#endif /* !_alsa_param_h_ */
