#ifndef _AVFilter_h_
#define _AVFilter_h_

#include "avframe.h"

struct IAudioFilter
{
	virtual ~IAudioFilter() {}

	virtual int Process(const struct avframe_t* frame) = 0;
};

struct IVideoFilter
{
	virtual ~IVideoFilter() {}

	virtual int Process(const struct avframe_t* frame) = 0;
};

#endif /* !_AVFilter_h_ */
