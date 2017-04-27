#ifndef _alsa_format_h_
#define _alsa_format_h_

static snd_pcm_format_t alsa_format(unsigned int sample_bits)
{
	switch (sample_bits)
	{
	case 8: return SND_PCM_FORMAT_U8;
	case 16:return SND_PCM_FORMAT_S16_LE;
	case 24:return SND_PCM_FORMAT_U24_LE;
	case 32:return SND_PCM_FORMAT_FLOAT_LE;
	case 64:return SND_PCM_FORMAT_FLOAT64_LE;
	default:return SND_PCM_FORMAT_UNKNOWN;
	}
}

#endif /* !_alsa_format_h_ */
