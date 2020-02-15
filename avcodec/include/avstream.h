#ifndef _avstream_h_
#define _avstream_h_

#include <stdint.h>
#include "avcodecid.h"

struct avstream_t
{
	int stream; // stream index
	enum AVPACKET_CODEC_ID codecid;

	// video only
	int width;
	int height;
	double fps; // optional

	// audio only
	int channels;	 ///< number of audio channels
	int sample_bits; ///< bits per sample
	int sample_rate; ///< samples per second(frequency)

	// stream extra data(codec data)
	void* extra;
	int bytes;
};

#endif /* !_avstream_h_ */
