#ifndef _alsa_format_h_
#define _alsa_format_h_

static snd_pcm_format_t alsa_format(int format)
{
	switch (format)
	{
	case PCM_SAMPLE_FMT_U8: return SND_PCM_FORMAT_U8;
	case PCM_SAMPLE_FMT_S16: return SND_PCM_FORMAT_S16_LE;
	case PCM_SAMPLE_FMT_FLOAT: return SND_PCM_FORMAT_FLOAT_LE;
	default: return SND_PCM_FORMAT_UNKNOWN;
	}
}

#endif /* !_alsa_format_h_ */
