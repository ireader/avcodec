#include "audio_input.h"
#include "av_register.h"
#include "avframe.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEVICE_NAME "default"

struct alsa_recorder_t
{
	snd_pcm_t* handle;
	snd_pcm_uframes_t samples;
	int bytes_per_sample;
	unsigned int rate;
	
	audio_input_callback cb;
	void* param;

	pthread_t thread;
	int running;
};

static snd_pcm_format_t alsa_format(int format)
{
	switch (format)
	{
	case PCM_SAMPLE_FMT_U8: return SND_PCM_FORMAT_U8;
	case PCM_SAMPLE_FMT_S16: return SND_PCM_FORMAT_S16_LE;
	case PCM_SAMPLE_FMT_FLOAT: return SND_PCM_FORMAT_FLOAT_LE;
	default: return SND_PCM_FORMAT_UNKNOWN;
	}
}

static int alsa_setparam(struct alsa_recorder_t* ai, int format, unsigned int channels, unsigned int *frequency, snd_pcm_uframes_t* frames)
{
	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(ai->handle, params);

	snd_pcm_hw_params_set_access(ai->handle, params, PCM_SAMPLE_PLANAR(format) ? SND_PCM_ACCESS_RW_NONINTERLEAVED : SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(ai->handle, params, alsa_format(format));
	snd_pcm_hw_params_set_channels(ai->handle, params, channels);
	snd_pcm_hw_params_set_rate_near(ai->handle, params, frequency, 0);
	snd_pcm_hw_params_set_period_size_near(ai->handle, params, frames, 0);

	return snd_pcm_hw_params(ai->handle, params);
}

static void* Worker(void* param)
{
	int r;
	char* buffer;
	struct alsa_recorder_t* ai;
	ai = (struct alsa_recorder_t*)param;

	buffer = (char*)malloc(ai->samples * ai->bytes_per_sample);
	while(ai->running)
	{
		r = snd_pcm_readi(ai->handle, buffer, ai->samples);
		if(-EPIPE == r)
		{
			printf("snd_pcm_readi overrun occurred\n");
			snd_pcm_prepare(ai->handle);
		}
		else if(r < 0)
		{
			printf("snd_pcm_readi %d, %s\n", r, snd_strerror(r));
			break;
		}

		if(r > 0)
		{
			ai->cb(ai->param, buffer, r);
		}
	}

	free(buffer);
	return NULL;
}

static int alsa_close(void* object)
{
	struct alsa_recorder_t* ai;
	ai = (struct alsa_recorder_t*)object;
	
	if (0 != ai->thread)
	{
		ai->running = 0;
		pthread_join(ai->thread, NULL);
	}

	if(ai->handle)
	{
		snd_pcm_drain(ai->handle);
		snd_pcm_close(ai->handle);
	}

	free(ai);
	return 0;
}

static void* alsa_open(int channels, int rate, int format, int samples, audio_input_callback cb, void* param)
{
	int r;
	struct alsa_recorder_t* ai;
	ai = (struct alsa_recorder_t*)malloc(sizeof(*ai));
	if(NULL == ai)
		return NULL;

	memset(ai, 0, sizeof(*ai));
	ai->rate = rate;
	ai->samples = samples;
	ai->bytes_per_sample = channels * PCM_SAMPLE_BITS(format) / 8;

	r = snd_pcm_open(&ai->handle, DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
	if(r < 0)
	{
		printf("alas open capture error:%s\r\n", snd_strerror(r));
		alsa_close(ai);		
		return NULL;
	}

	r = alsa_setparam(ai, format, channels, &ai->rate, &ai->samples);
	if(r < 0)
	{
		printf("snd_pcm_hw_params: %s\r\n", snd_strerror(r));
		alsa_close(ai);
		return NULL;
	}

	ai->running = 1;
	pthread_create(&ai->thread, NULL, Worker, ai);

	ai->cb = cb;
	ai->param = param;
	return ai;
}

static int alsa_start(void* object)
{
	struct alsa_recorder_t* ai;
	ai = (struct alsa_recorder_t*)object;
	return snd_pcm_start(ai->handle);
}

static int alsa_stop(void* object)
{
	struct alsa_recorder_t* ai;
	ai = (struct alsa_recorder_t*)object;
	return snd_pcm_pause(ai->handle, 1);
}

int alsa_recorder_register()
{
	static audio_input_t ai;
	memset(&ai, 0, sizeof(ai));
	ai.open = alsa_open;
	ai.close = alsa_close;
	ai.start = alsa_start;
	ai.stop = alsa_stop;
	return av_set_class(AV_AUDIO_RECORDER, "alsa", &ai);
}
