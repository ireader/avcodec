#include "avstatistic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void avbitrate_clear(struct avbitrate_t* rate)
{
	rate->i = 0;
	rate->clock = 0;
	rate->total = 0;
	rate->interval = 1;
	memset(rate->buckets, 0, sizeof(rate->buckets));
}

void avbitrate_input(struct avbitrate_t* rate, uint64_t clock, uint64_t bytes)
{
	uint32_t N;
	uint32_t i, j;

	N = sizeof(rate->buckets) / sizeof(rate->buckets[0]);
	i = (clock / rate->interval) % N;

	if (rate->i != i)
	{
		for (j = (rate->i + 1) % N; j != i; j = (j + 1) % N)
			rate->buckets[j] = 0;

		rate->i = i;
		rate->buckets[i] = 0;
	}

	rate->clock = clock;
	rate->total += bytes;
	rate->buckets[i] += bytes; // update bucket bytes
}

uint64_t avbitrate_get(const struct avbitrate_t* rate)
{
	uint32_t i;
	uint32_t N;
	uint64_t total;

	N = sizeof(rate->buckets) / sizeof(rate->buckets[0]);

	total = 0;
	for (i = (rate->i + 1) % N; i != rate->i; i = (i + 1) % N)
	{
		total += rate->buckets[i];
	}

	return total * 1000 / ((N - 1) * (uint64_t)rate->interval);
}

/// format_size byte
const char* format_size(uint64_t size, char buf[16])
{
	int i;
	uint64_t unit = 1024;
	static const char* format[] = { "%.1fKB", "%.1fMB", "%.1fGB", "%.1fTB", "%.1fPB", "%.1fPB" };
	
	if (size < unit)
	{
		snprintf(buf, 15, "%u", (unsigned int)size);
		return buf;
	}

	for (i = 0; i < sizeof(format) / sizeof(format[0]) - 1; i++)
	{
		if (size < unit * unit)
			break;

		unit *= unit;
	}

	snprintf(buf, 15, format[i], size * 1.0 / unit);
	return buf;
}

/// format_bitrate byte per second => bit per second
const char* format_bitrate(uint64_t bitrate, char buf[16])
{
	int i;
	uint64_t unit = 1000;
	static const char* format[] = { "%.1fkbps", "%.1fMbps", "%.1fGbps", "%.1fTbps", "%.1fPbps", "%.1fPbps" };
//	static const char* format[] = { "%.1fKB/s", "%.1fMB/s", "%.1fGB/s", "%.1fTB/s", "%.1fPB/s", "%.1fPB/s" };

	bitrate *= 8; // byte to bits
	for (i = 0; i < sizeof(format)/sizeof(format[0]) - 1; i++)
	{
		if (bitrate < unit * unit)
			break;

		unit *= unit;
	}
	
	snprintf(buf, 15, format[i], bitrate * 1.0 / unit);
	return buf;
}

/// format_duration
const char* format_duration(uint64_t duration, char buf[16])
{
	if (duration >= (uint64_t)365 * 24 * 60 * 60 * 1000)
	{
		snprintf(buf, 15, "%.1fyear", (duration / (24 * 60 * 60 * 1000)) / 365.0);
		return buf;
	}

	if (duration >= 24 * 60 * 60 * 1000)
	{
		snprintf(buf, 15, "%.1fdays", (duration / (60 * 60 * 1000)) / 24.0);
		return buf;
	}

	if (duration >= 60 * 60 * 1000)
	{
		snprintf(buf, 15, "%dh%dm", (int)duration / (60 * 60 * 1000), (int)(duration / (60 * 1000)) % 60);
		return buf;
	}

	if (duration >= 60 * 1000)
	{
		snprintf(buf, 15, "%dm%ds", (int)duration / (60 * 1000), (int)(duration / 1000) % 60);
		return buf;
	}

	snprintf(buf, 15, "%ds", (int)duration / 1000);
	return buf;
}
