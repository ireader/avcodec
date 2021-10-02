#ifndef _AVInterval_h_
#define _AVInterval_h_

#include "avstatistic.h"
#include "app-log.h"
#include <stdint.h>
#include <string>

class AVInterval
{
	friend class AVLivePlayer;

public:
	AVInterval(const std::string& tag, int interval=5000) : m_tag(tag), m_interval(interval)
	{
		m_clock = 0;
		m_last = 0;
		m_pts = 0;
		Reset();
	}

public:
	void Tick(uint64_t clock, int64_t pts)
	{
		if (0 != m_clock) {
			avjitter_input(&m_fps, (int)(clock - m_clock));
			avjitter_input(&m_jitter, (int)((clock - m_clock) - (pts - m_pts)));
		}
		m_pts = pts;
		m_clock = clock;
		Print();
	}

private:
	void Reset()
	{
		avjitter_clear(&m_fps);
		avjitter_clear(&m_jitter);
	}

	void Print()
	{
		if (0 == m_last)
			m_last = m_clock;

		if ((int)(m_clock - m_last) > m_interval)
		{
			char fps[64];
			char jitter[64];
			avjitter_format(&m_fps, fps, sizeof(fps));
			avjitter_format(&m_jitter, jitter, sizeof(jitter));
			app_log(LOG_DEBUG, "%s|fps: %s, jitter: %s\n", m_tag.c_str(), fps, jitter);

			m_last = m_clock;
			Reset();
		}
	}

private:
	std::string m_tag;
	uint64_t m_clock;
	uint64_t m_last;
	int64_t m_pts;
	int m_interval;
	struct avjitter_t m_fps; // 1 / fps
	struct avjitter_t m_jitter;
};

#endif /* !_AVInterval_h_ */
