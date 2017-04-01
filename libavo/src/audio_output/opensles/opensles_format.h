#ifndef _opensles_format_h_
#define _opensles_format_h_

#include <assert.h>

static int opensles_pcm_bits(int bits_per_samples)
{
	switch (bits_per_samples)
	{
	case 8:	return SL_PCMSAMPLEFORMAT_FIXED_8;
	case 16:return SL_PCMSAMPLEFORMAT_FIXED_16;
	case 20:return SL_PCMSAMPLEFORMAT_FIXED_20;
	case 24:return SL_PCMSAMPLEFORMAT_FIXED_24;
	case 28:return SL_PCMSAMPLEFORMAT_FIXED_28;
	case 32:return SL_PCMSAMPLEFORMAT_FIXED_32;
	default: return bits_per_samples;
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
	case 1: return SL_SPEAKER_FRONT_CENTER;
	case 2: return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	case 3: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
	case 4: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_TOP_CENTER;
	case 5: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_TOP_FRONT_LEFT | SL_SPEAKER_TOP_FRONT_RIGHT;
	case 6: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_TOP_FRONT_LEFT | SL_SPEAKER_TOP_FRONT_RIGHT | SL_SPEAKER_LOW_FREQUENCY;
	case 8: return SL_SPEAKER_FRONT_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_FRONT_LEFT_OF_CENTER | SL_SPEAKER_TOP_FRONT_LEFT | SL_SPEAKER_TOP_FRONT_RIGHT | SL_SPEAKER_LOW_FREQUENCY | SL_SPEAKER_SIDE_LEFT | SL_SPEAKER_SIDE_RIGHT;
	default: assert(0); return SL_SPEAKER_FRONT_CENTER;
	}
}

static int opensles_format(SLAndroidDataFormat_PCM_EX *pcm, int channels, int bits_per_samples, int samples_per_seconds)
{
	assert(bits_per_samples <= 32);
	pcm->formatType = SL_DATAFORMAT_PCM;
	pcm->numChannels = channels;
	pcm->samplesPerSec = opensles_pcm_freq(samples_per_seconds); // samples_per_seconds * 1000
	pcm->bitsPerSample = opensles_pcm_bits(bits_per_samples); // SL_PCMSAMPLEFORMAT_FIXED_16
	pcm->containerSize = pcm->bitsPerSample;
	pcm->channelMask = opensles_channel_mask(channels);
	pcm->endianness = SL_BYTEORDER_LITTLEENDIAN;
	pcm->representation = SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT;
	return 0;
}

static int opensles_format_float(SLAndroidDataFormat_PCM_EX *pcm, int channels, int bits_per_samples, int samples_per_seconds)
{
	assert(32 == bits_per_samples || 64 == bits_per_samples);
	pcm->formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
	pcm->numChannels = channels;
	pcm->samplesPerSec = opensles_pcm_freq(samples_per_seconds); // samples_per_seconds * 1000
	pcm->bitsPerSample = opensles_pcm_bits(bits_per_samples); // 32
	pcm->containerSize = pcm->bitsPerSample;
	pcm->channelMask = opensles_channel_mask(channels);
	pcm->endianness = SL_BYTEORDER_LITTLEENDIAN;
	pcm->representation = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
	return 0;
}

#endif /* !_opensles_format_h_ */
