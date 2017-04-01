#ifndef _alsa_param_h_
#define _alsa_param_h_

static int alsa_hw_param(snd_pcm_t* handle, snd_pcm_format_t format, int channels, int frequency)
{
	int r;
	unsigned int val = frequency;
	snd_pcm_uframes_t frames = frequency;
	snd_pcm_uframes_t period = frequency / 50; // buffer time 20ms

	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);

	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, format);
	snd_pcm_hw_params_set_channels(handle, params, channels);
	snd_pcm_hw_params_set_rate_near(handle, params, &val, 0);

	// set alsa buffer
	r = snd_pcm_hw_params_set_buffer_size_near(handle, params, &frames);

	// set alsa peroid(latency)
	r = snd_pcm_hw_params_set_period_size_near(handle, params, &period, 0);

	return snd_pcm_hw_params(handle, params);
}

#endif /* !_alsa_param_h_ */
