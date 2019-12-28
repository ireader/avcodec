#ifndef _avjitter_h_
#define _avjitter_h_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct avjitter_t {
	int duration;
	int count;
	int max;
	int min;
};

static inline void av_jitter_reset(struct avjitter_t* jitter)
{
	memset(jitter, 0, sizeof(*jitter));
}

static inline int av_jitter_format(const struct avjitter_t* jitter, char* buf, int len)
{
	return snprintf(buf, len, "%0.2f(%d~%d)", jitter->count > 0 ? jitter->duration * 1.0f / jitter->count : 0.0, jitter->min, jitter->max);
}

static inline void av_jitter_input(struct avjitter_t* jitter, int value)
{
	jitter->duration += value;
	if (0 == jitter->count++)
	{
		jitter->max = value;
		jitter->min = value;
	}
	if (value > jitter->max)
		jitter->max = value;
	if (value < jitter->min)
		jitter->min = value;
}

#endif /* !_avjitter_h_ */
