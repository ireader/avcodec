#pragma once

#include "sys/sync.hpp"
#include <stdint.h>
#include <queue>

struct AVFrame
{
	int64_t pts;
	uint64_t duration; // MS
	const void* frame;
	int serial;
};

class AVFrameQ
{
public:
	AVFrameQ()
	{
		m_underrun = 0;
		m_overrun = 0;
		m_serial = -1;
		m_duration = 0;
	}

	~AVFrameQ()
	{
		while (!m_frames.empty())
		{
			//AVFrame& frame = m_frames.front();
			//avpacket_release(frame);
			m_frames.pop();
		}
	}

	bool Read(AVFrame& frame)
	{
		AutoThreadLocker locker(m_locker);
		if (m_frames.empty())
		{
			++m_underrun;
			return false;
		}

		frame = m_frames.front();
		m_frames.pop();

		m_duration -= frame.duration;
		return true;
	}

	bool Next(AVFrame& frame)
	{
		AutoThreadLocker locker(m_locker);
		if (m_frames.empty())
		{
			++m_underrun;
			return false;
		}

		frame = m_frames.front();
		return true;
	}

	bool Write(const AVFrame& frame)
	{
		//if (m_frames.size() >= m_max_packets)
		//	app_log(LOG_WARNING, "[AVPacketQueue] overrun: %lu\n", (unsigned long)m_overrun++);

		//avpacket_addref(pkt);

		AutoThreadLocker locker(m_locker);
		m_duration += frame.duration;
		m_serial = frame.serial;
		m_frames.push(frame);
		return true;
	}

	size_t Size() const
	{
		return m_frames.size();
	}

	int GetSerial() const
	{
		return m_serial;
	}

	uint64_t GetDuration() const
	{
		return m_duration;
	}

private:
	int m_serial;
	size_t m_overrun; // full
	size_t m_underrun;
	uint64_t m_duration;
	mutable ThreadLocker m_locker;
	std::queue<AVFrame> m_frames;
};
