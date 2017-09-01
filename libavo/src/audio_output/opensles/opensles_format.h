#ifndef _opensles_format_h_
#define _opensles_format_h_

#include <assert.h>

static int opensles_pcm_bits(int format)
{
	switch (format)
	{
	case PCM_SAMPLE_FMT_U8:	return SL_PCMSAMPLEFORMAT_FIXED_8;
	case PCM_SAMPLE_FMT_S16:return SL_PCMSAMPLEFORMAT_FIXED_16;
	case PCM_SAMPLE_FMT_S32:return SL_PCMSAMPLEFORMAT_FIXED_32;
	case PCM_SAMPLE_FMT_FLOAT:return SL_PCMSAMPLEFORMAT_FIXED_32;
	default: return PCM_SAMPLE_BITS(format);
	}
}

static int opensles_pcm_freq(int samples_per_seconds)
{
	switch (samples_per_seconds)
	{
	case 8000:	return SL_SAMPLINGRATE_8;
	case 11025:	return SL_SAMPLINGRATE_11_025;
	case 12000: return SL_SAMPLINGRATE_12;
	case 16000:	return SL_SAMPLINGRATE_16;
	case 22050: return SL_SAMPLINGRATE_22_05;
	case 24000: return SL_SAMPLINGRATE_24;
	case 32000: return SL_SAMPLINGRATE_32;
	case 44100: return SL_SAMPLINGRATE_44_1;
	case 48000: return SL_SAMPLINGRATE_48;
	case 64000: return SL_SAMPLINGRATE_64;
	case 88200: return SL_SAMPLINGRATE_88_2;
	case 96000: return SL_SAMPLINGRATE_96;
	case 192000:return SL_SAMPLINGRATE_192;
	default: return samples_per_seconds * 1000;
	}
}

static int opensles_channel_mask(int channels)
{
	switch (channels)
	{
	case 1/*MONO*/: return SL_SPEAKER_FRONT_CENTER;
	case 2/*STEREO*/: return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	case 3/*SURROUND*/: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	case 4: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_FRONT_RIGHT_OF_CENTER | SL_SPEAKER_BACK_CENTER;
	case 5: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT;
	case 6: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT | SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT | SL_SPEAKER_LOW_FREQUENCY;
	case 8: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_FRONT_RIGHT_OF_CENTER | SL_SPEAKER_TOP_FRONT_LEFT | SL_SPEAKER_TOP_FRONT_RIGHT | SL_SPEAKER_LOW_FREQUENCY | SL_SPEAKER_BACK_LEFT | SL_SPEAKER_BACK_RIGHT;
	default: assert(0); return SL_SPEAKER_FRONT_CENTER;
	}
}

static int opensles_format(SLAndroidDataFormat_PCM_EX *pcm, int channels, int rate, int format)
{
	pcm->formatType = PCM_SAMPLE_FLOAT(format) ? SL_ANDROID_DATAFORMAT_PCM_EX : SL_DATAFORMAT_PCM;
	pcm->numChannels = channels;
	pcm->sampleRate = opensles_pcm_freq(rate); // samples_per_seconds * 1000
	pcm->bitsPerSample = opensles_pcm_bits(format); // 32
	pcm->containerSize = PCM_SAMPLE_BITS(format);
	pcm->channelMask = opensles_channel_mask(channels);
	pcm->endianness = SL_BYTEORDER_LITTLEENDIAN;
	pcm->representation = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
	return 0;
}

#endif /* !_opensles_format_h_ */
