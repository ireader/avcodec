#ifndef _pcm_h_
#define _pcm_h_

#include <stdint.h>

enum pcm_sample_format
{
	PCM_SAMPLE_FMT_NONE		= -1,

	PCM_SAMPLE_FMT_U8		= 0,		///< unsigned 8 bits
	PCM_SAMPLE_FMT_S16		= 1,		///< signed 16 bits
	PCM_SAMPLE_FMT_S32		= 2,		///< signed 32 bits
	PCM_SAMPLE_FMT_FLOAT	= 3,		///< float
	PCM_SAMPLE_FMT_DOUBLE	= 4,		///< double

	PCM_SAMPLE_FMT_U8P		= 5,		///< unsigned 8 bits, planar
	PCM_SAMPLE_FMT_S16P		= 6,		///< signed 16 bits, planar
	PCM_SAMPLE_FMT_S32P		= 7,		///< signed 32 bits, planar
	PCM_SAMPLE_FMT_FLTP		= 8,		///< float, planar
	PCM_SAMPLE_FMT_DBLP		= 9,		///< double, planar

	PCM_SAMPLE_FMT_S64		= 10,		///< signed 64 bits
	PCM_SAMPLE_FMT_S64P		= 11,		///< signed 64 bits, planar
};

struct pcm_t
{
	int format;		 ///< PCM_SAMPLE_FMT_XXX
	int channel;	 ///< number of audio channels
	int sample_bits; ///< bits per sample
	int sample_rate; ///< samples per second
	int samples;	 ///< number of audio samples (per channel)

#define PCM_PLANAR_NUM 8
	uint8_t* data[PCM_PLANAR_NUM];
	int linesize[PCM_PLANAR_NUM];

	int64_t pts;
	int64_t dts;	///< reserved, always 0
};

#endif /* !_pcm_h_ */
