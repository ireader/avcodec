#ifndef _audio_input_h
#define _audio_input_h

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*audio_input_callback)(void* param, const void* samples, int bytes);

typedef struct
{
	void* (*open)(int channels, int bits_per_samples, int samples_per_seconds, audio_input_callback cb, void* param);
	int (*close)(void* ai);
	int (*isopened)(void* ai);

	int (*pause)(void* ai);
	int (*reset)(void* ai);

	int (*setvolume)(void* ai, int v);
	int (*getvolume)(void* ai);
} audio_input_t;

void* audio_input_open(int channels, int bits_per_samples, int samples_per_seconds, audio_input_callback cb, void* param);
int audio_input_close(void* ai);
int audio_input_isopened(void* ai);

int audio_input_pause(void* ai);
int audio_input_reset(void* ai);

int audio_input_setvolume(void* ai, int v);
int audio_input_getvolume(void* ai);

#ifdef __cplusplus
}

#include <stdlib.h>
#include <string.h>

class audio_input
{
public:
	audio_input():m_ai(0), m_channels(0), m_bits_per_sampels(0), m_samples_per_seconds(0), m_callback(NULL), m_param(NULL), m_size(0), m_p(NULL){};
	~audio_input(){ close(); if(m_p) free(m_p); }

public:
	bool open(int channels, int bits_per_samples, int samples_per_seconds, audio_input_callback cb, void* param, int buffer_size=0)
	{
		if(isopened())
			return true;

		m_ai = audio_input_open(channels, bits_per_samples, samples_per_seconds, OnSamples, this);
		if(!m_ai)
			return false;

		m_param = param;
		m_callback = cb;
		m_size = buffer_size;
		if(m_size > 0)
		{
			m_p = (char*)malloc(m_size);
			m_offset = 0;
		}

		m_channels = channels;
		m_bits_per_sampels = bits_per_samples;
		m_samples_per_seconds = samples_per_seconds;
		return true;
	}

	bool isopened() const { return 1==(m_ai?audio_input_isopened(m_ai) : 0); }
	int close(){ if(m_ai) audio_input_close(m_ai); m_ai = 0; return 0; }

	int pause() { return isopened()?audio_input_pause(m_ai) : -1; }
	int reset()	{ return isopened()?audio_input_reset(m_ai) : -1; }

	int getvolume() const { return isopened()?audio_input_getvolume(m_ai) : -1; }
	int setvolume(int v)  { return isopened()?audio_input_setvolume(m_ai, v) : -1; }

private:
	static void OnSamples(void* param, const void* samples, int bytes)
	{
		audio_input* self = (audio_input*)param;
		self->OnSamples(samples, bytes);
	}

	void OnSamples(const void* samples, int bytes)
	{
		if(0 == m_size)
		{
			m_callback(m_param, samples, bytes);
			return;
		}

		int offset = 0;
		while(m_offset + (bytes-offset) >= m_size)
		{
			memcpy(m_p+m_offset, (const char*)samples+offset, m_size-m_offset);
			m_callback(m_param, m_p, m_size);

			offset += m_size-m_offset;
			m_offset = 0;			
		}

		if(bytes-offset > 0)
		{
			memcpy(m_p+m_offset, (const char*)samples+offset, bytes-offset);
			m_offset += bytes-offset;
		}
	}

private:
	audio_input(const audio_input&){}
	audio_input& operator=(const audio_input&){ return *this; }

private:
	void* m_ai;
	int m_channels;
	int m_bits_per_sampels;
	int m_samples_per_seconds;
	audio_input_callback m_callback;
	void* m_param;
	int m_size;
	char* m_p;
	int m_offset;
};

#endif

#endif /* !_audio_input_h */
