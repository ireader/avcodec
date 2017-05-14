#ifndef _audio_input_h
#define _audio_input_h

#ifdef __cplusplus
extern "C" {
#endif

///@param[in] pcm sample buffer
///@param[in] samples number of samples per channel
typedef void (*audio_input_callback)(void* param, const void* pcm, int samples);

typedef struct
{
	///@see audio_input_open
	void* (*open)(int channels, int samples_per_second, int format, int samples, audio_input_callback cb, void* param);
	int (*close)(void* ai);

	int (*start)(void* ai);
	int (*stop)(void* ai);
} audio_input_t;

///@param[in] channels audio channel number
///@param[in] samples_per_second clock rate in Hz
///@param[in] format audio format, PCM_SAMPLE_FMT_XXX(avframe.h)
///@param[in] samples capture buffer size
void* audio_input_open(int channels, int samples_per_second, int format, int samples, audio_input_callback cb, void* param);
int audio_input_close(void* ai);

int audio_input_start(void* ai);
int audio_input_stop(void* ai);

#ifdef __cplusplus
}

class audio_input
{
public:
	audio_input():m_ai(0), m_channels(0), m_rate(0), m_format(0) {};
	~audio_input(){ close(); }

public:
	bool open(int channels, int samples_per_second, int format, int samples, audio_input_callback cb, void* param)
	{
		if(isopened())
			return true;

		m_ai = audio_input_open(channels, samples_per_second, format, samples, cb, param);
		if(!m_ai)
			return false;

		m_rate = samples_per_second;
		m_format = format;
		m_channels = channels;
		return true;
	}

	bool isopened() const { return !!m_ai; }
	int close(){ if(m_ai) audio_input_close(m_ai); m_ai = 0; return 0; }

	int start() { return isopened()?audio_input_start(m_ai) : -1; }
	int stop()	{ return isopened()?audio_input_stop(m_ai) : -1; }

private:
	audio_input(const audio_input&){}
	audio_input& operator=(const audio_input&){ return *this; }

private:
	void* m_ai;
	int m_rate;
	int m_format;
	int m_channels;
};

#endif

#endif /* !_audio_input_h */
