#ifndef _VOFilter_h_
#define _VOFilter_h_

#include "AVFilter.h"
#include "video_output.h"

class VOFilter : public IVideoFilter
{
public:
	VOFilter(void* window)
	{
		m_vo = NULL;
		m_window = window;
	}

	virtual ~VOFilter()
	{
		if (m_vo)
		{
			video_output_close(m_vo);
			m_vo = NULL;
		}
	}

public:
	virtual int Process(const struct avframe_t* frame)
	{
		if (NULL == m_vo)
		{
			m_vo = video_output_open(m_window, frame->format, frame->width, frame->height);
			if (NULL == m_vo)
				return -1;
		}
		
		return video_output_write(m_vo, frame, 0, 0, 0, 0, 0, 0, 0, 0);
	}

private:
	void* m_vo;
	void* m_window;
};

#endif /* !_VOFilter_h_ */
