#include "avframe.h"

int avframe_get_pcm_sample_bits(int format)
{
	static int bits[] = { 8, 16, 32, 32, 64, 8, 16, 32, 32, 64, 64, 64 };
	return bits[format % (sizeof(bits)/sizeof(bits[0]))];
}
