#ifndef _avstatistic_h_
#define _avstatistic_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct avbitrate_t
{
	uint32_t i; // internal use only
	uint32_t interval; // bucket interval(ms)
	uint64_t buckets[6]; // N + 1
	
	uint64_t clock; // last clock
	uint64_t total; // total bytes
};

struct avstatistic_t
{
	int64_t clock;			// start time(UTC+0 ms)

	struct
	{
		int64_t pts; // last packet pts
		int64_t dts; // last packet dts
		uint64_t packets;
		struct avbitrate_t bitrate;
	} streams[8];

//#define s_audio streams[0]
//#define s_video streams[1]
};

/// avbitrate_clear clear all data
void avbitrate_clear(struct avbitrate_t* rate);

/// avbitrate_input [10 | 10 | 10 | 10 | 10 | 10] bytes bucket
void avbitrate_input(struct avbitrate_t* rate, uint64_t clock, uint64_t bytes);

/// avbitrate_get bytes per second
uint64_t avbitrate_get(const struct avbitrate_t* rate);

#ifdef __cplusplus
}
#endif
#endif /* !_avstatistic_h_ */
