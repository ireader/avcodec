#ifndef _avstatistic_h_
#define _avstatistic_h_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct avjitter_t {
	int duration;
	int count;
	int max;
	int min;
};

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
		struct avjitter_t jitter;
		struct avbitrate_t bitrate;

		int64_t first_recv; // rtmp handshake
		int64_t first_packet; // first audio/video/data chunk/packet
		int64_t first_audio_frame;
		int64_t first_video_frame;
	} streams[8];

//#define s_audio streams[0]
//#define s_video streams[1]
};

void avstatistic_init(struct avstatistic_t* stats, int64_t clock, int interval);

/// avjitter_clear clear all data
void avjitter_clear(struct avjitter_t* jitter);

/// avjitter_input update jitter min/max
void avjitter_input(struct avjitter_t* jitter, int value);

/// avjitter_format output format jitter string
int avjitter_format(const struct avjitter_t* jitter, char* buf, int len);


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
