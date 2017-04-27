#ifndef _alsa_param_h_
#define _alsa_param_h_

static int alsa_hw_param(snd_pcm_t* handle, snd_pcm_format_t format, unsigned int channels, unsigned int *frequency, snd_pcm_uframes_t* frames)
{
	int r;
	snd_pcm_uframes_t period;

	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);

	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, format);
	snd_pcm_hw_params_set_channels(handle, params, channels);
	snd_pcm_hw_params_set_rate_near(handle, params, frequency, 0);

	// set alsa peroid(latency)
	period = *frequency / 100; // buffer time 10ms
	snd_pcm_hw_params_set_period_size_near(handle, params, &period, 0);

	// set alsa buffer size
	r = snd_pcm_hw_params_set_buffer_size_near(handle, params, frames);
	
	return 0 == r ? snd_pcm_hw_params(handle, params) : r;
}

#endif /* !_alsa_param_h_ */
