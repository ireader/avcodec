#ifndef _AVInterval_h_
#define _AVInterval_h_

#include "app-log.h"
#include <stdint.h>
#include <string>

class AVInterval
{
public:
	AVInterval(const std::string& tag) : m_tag(tag)
	{
		m_clock = 0;
		Reset();
	}

public:
	void Tick(uint64_t clock)
	{
		if (0 != m_clock) {
			int diff = (int)(clock - m_clock);
			if (diff > m_max)
				m_max = diff;
			if (diff < m_min)
				m_min = diff;
			m_total += diff;
		}

		if (++m_count == 50) {
			app_log(LOG_DEBUG, "%s|AVG: %0.2f, MIN: %d, MAX: %d\n", m_tag.c_str(), m_total * 1.0f / m_count, m_min, m_max);
			Reset();
		}

		m_clock = clock;
	}

private:
	void Reset()
	{
		m_total = 0;
		m_count = 0;
		m_max = -1;
		m_min = 10000;
	}

private:
	std::string m_tag;
	uint64_t m_clock;
	int m_total;
	int m_count;
	int m_max;
	int m_min;
};

#endif /* !_AVInterval_h_ */
