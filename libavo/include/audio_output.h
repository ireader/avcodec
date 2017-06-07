#ifndef _audio_output_h_
#define _audio_output_h_

#ifdef __cplusplus
extern "C"{
#endif

typedef struct
{
	///@see audio_output_open
	void* (*open)(int channels, int frequency, int format, int frames);
	int (*close)(void* ao);
	
	///@see audio_output_write
	int (*write)(void* ao, const void* pcm, int frames);

	int (*play)(void* ao);
	int (*pause)(void* ao);
	int (*reset)(void* ao);

	///@see audio_output_getframes
	int (*get_frames)(void* ao);
} audio_output_t;


///@param[in] channels audio channel number
///@param[in] frequency clock rate in Hz
///@param[in] format audio format, PCM_SAMPLE_FMT_XXX(avframe.h)
///@param[in] frames audio output buffer size by samples(per channel)
void* audio_output_open(int channels, int frequency, int format, int frames);
int audio_output_close(void* ao);

///@param[in] pcm sample buffer
///@param[in] frames number of samples per channel
///@return >0-write samples, <0-error
int audio_output_write(void* ao, const void* pcm, int frames);

///@return 0-ok, other-error
int audio_output_play(void* ao);
int audio_output_pause(void* ao);
int audio_output_reset(void* ao);

///@return available sample number(per channel)
int audio_output_getframes(void* ao);

#ifdef __cplusplus
}

class audio_output
{
public:
	audio_output():m_ao(0), m_format(0), m_channels(0), m_frequency(0){}
	~audio_output(){ close(); }

public:
	bool open(int channels, int frequency, int format, int samples=0)
	{
		if(isopened() && check(channels, frequency, format))
			return true;

		close();

		m_ao = audio_output_open(channels, frequency, format, samples?samples: frequency);
		m_format = format;
		m_channels = channels;
		m_frequency = frequency;
		return !!m_ao;
	}

	int write(const void* samples, int count){ return isopened()?audio_output_write(m_ao, samples, count) : -1; }

	bool isopened() const	{ return !!m_ao; }
	int close()				{ if(m_ao) audio_output_close(m_ao); m_ao = 0; return 0; }

	int play()				{ return isopened()?audio_output_play(m_ao)  : -1; }
	int pause()				{ return isopened()?audio_output_pause(m_ao) : -1; }
	int reset()				{ return isopened()?audio_output_reset(m_ao) : -1; }

	int getframes() const	{ return isopened()?audio_output_getframes(m_ao) : -1; }

	bool check(int channels, int frequency, int format)
	{
		if(!m_ao) return false;
		return m_channels == channels && m_frequency == frequency && m_format == format;
	}
	
private:
	audio_output(const audio_output&){}
	audio_output& operator=(const audio_output&){ return *this; }

private:
	void* m_ao;
	int m_format;
	int m_channels;
	int m_frequency;
};

#endif

#endif /* !_audio_output_h_ */
