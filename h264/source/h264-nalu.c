#include "h264-nalu.h"
#include "h264-util.h"
#include <assert.h>

void h264_nalu(const unsigned char* h264, unsigned int bytes, h264_nalu_handler handler, void* param)
{
	unsigned int n;
	const unsigned char* p, *next, *end;

	end = h264 + bytes;
	p = h264_startcode(h264, bytes);

	while(p)
	{
		next = h264_startcode(p, end - p);
		if (next)
		{
			n = next - p - 3;
			if (n > 0 && 0 == p[n - 1]) n--; // filter tailing zero
		}
		else
		{
			n = end - p;
		}

		assert(n > 0);
		if (n > 0)
		{
			handler(param, p, n);
		}

		p = next;
	}
}
