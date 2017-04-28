#ifndef _AVFilter_h_
#define _AVFilter_h_

#include "avframe.h"

struct IAudioFilter
{
	virtual ~IAudioFilter() {}

	virtual int AudioFilter(const struct avframe_t* frame) = 0;
};

struct IVideoFilter
{
	virtual ~IVideoFilter() {}

	virtual int VideoFilter(const struct avframe_t* frame) = 0;
};

#endif /* !_AVFilter_h_ */
