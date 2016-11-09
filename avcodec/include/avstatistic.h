#ifndef _avstatistic_h_
#define _avstatistic_h_

#include <stdint.h>

struct avstatistic_t
{
	int64_t clock;			// start time(UTC+0 ms)

	int64_t audio_clock;	// last audio recv/send time(UTC+0 ms)
	int64_t audio_pts;		// audio pts
	uint64_t audio_bytes;
	uint64_t audio_packets;

	int64_t video_clock;	// last video recv/send time(UTC+0 ms)
	int64_t video_pts;		// video pts/dts
	uint64_t video_bytes;
	uint64_t video_packets;
};

#endif /* !_avstatistic_h_ */
