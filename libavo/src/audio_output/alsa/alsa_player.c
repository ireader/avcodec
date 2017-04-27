#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include "alsa_mixer.h"
#include "alsa_param.h"
#include "alsa_format.h"
#include "alsa_recovery.h"
#include "audio_output.h"
#include "av_register.h"

#define DEVICE_NAME "default"

struct alsa_player_t
{
	snd_pcm_t* handle;
	unsigned int channels;
	unsigned int sample_bits;
	unsigned int sample_rate;
	snd_pcm_uframes_t samples;
};

static int alsa_close(void* object)
{
	struct alsa_player_t* ao = (struct alsa_player_t*)object;
	if(NULL != ao->handle)
	{
		snd_pcm_drain(ao->handle);
		snd_pcm_close(ao->handle);
	}
	free(ao);
	return 0;
}

static void* alsa_open(int channels, int bits_per_sample, int samples_per_second, int samples)
{
	int r;
	snd_pcm_format_t format;
	struct alsa_player_t* ao;
	ao = (struct alsa_player_t*)malloc(sizeof(*ao));
	if(NULL == ao)
		return NULL;
	memset(ao, 0, sizeof(struct alsa_player_t));
	ao->samples = samples;
	ao->channels = channels;
	ao->sample_bits = bits_per_sample;
	ao->sample_rate = samples_per_second;

	format = alsa_format(ao->sample_bits);
	if (SND_PCM_FORMAT_UNKNOWN == format)
	{
		alsa_close(ao);
		return NULL;
	}

	//r = snd_pcm_open(&ao->handle, "plug:dmix", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	//r = snd_pcm_open(&m_handle, "plug:dmix", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	//r = snd_pcm_open(&m_handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_ASYNC);
	r = snd_pcm_open(&ao->handle, DEVICE_NAME, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if(r < 0)
	{
		printf("alsa snd_pcm_open(%s) error:%s\r\n", DEVICE_NAME, snd_strerror(r));
		alsa_close(ao);
		return NULL;
	}

	r = alsa_hw_param(ao->handle, format, ao->channels, &ao->sample_rate, &ao->samples);
	if(r < 0)
	{
		printf("snd_pcm_hw_params: %s\n", snd_strerror(r));
		alsa_close(ao);
		return NULL;
	}

	
	return ao;
}

static int alsa_write(void* object, const void* samples, int count)
{
	struct alsa_player_t* ao = (struct alsa_player_t*)object;
	int r = snd_pcm_writei(ao->handle, samples, count);
	if(r < 0)
	{
		r = alsa_recovery(ao->handle, r);
		if(0 == r)
			r = snd_pcm_writei(ao->handle, samples, count);
	}

	if(r < 0)
	{
		printf("snd_pcm_writei error=%d, %s\r\n", r, snd_strerror(r));
		return r;
	}
	return r;
}

static int alsa_play(void* object)
{
	struct alsa_player_t* ao = (struct alsa_player_t*)object;
	return snd_pcm_pause(ao->handle, 0); // resume
	//return snd_pcm_start(ao->handle);
}

static int alsa_pause(void* object)
{
	struct alsa_player_t* ao = (struct alsa_player_t*)object;
	return snd_pcm_pause(ao->handle, 1);
}

static int alsa_reset(void* object)
{
	struct alsa_player_t* ao = (struct alsa_player_t*)object;
	snd_pcm_reset(ao->handle);
	return 0;
}

static int alsa_get_samples(void* object)
{
	snd_pcm_state_t state;
	snd_pcm_sframes_t frames;
	struct alsa_player_t* ao = (struct alsa_player_t*)object;
	// http://www.alsa-project.org/alsa-doc/alsa-lib/pcm.html
	// The function snd_pcm_avail_update() updates the current available count of samples for writing (playback) 
	// or filled samples for reading (capture). This call is mandatory for updating actual r/w pointer. 
	// Using standalone, it is a light method to obtain current stream position, 
	// because it does not require the user <-> kernel context switch, but the value is less accurate, 
	// because ring buffer pointers are updated in kernel drivers only when an interrupt occurs. 
	// If you want to get accurate stream state, use functions snd_pcm_avail(), snd_pcm_delay() or snd_pcm_avail_delay().
	state = snd_pcm_state(ao->handle);
	if(SND_PCM_STATE_RUNNING  != state)
		return 0;

	frames = snd_pcm_avail_update(ao->handle);
	//snd_pcm_sframes_t frames = snd_pcm_avail(ao->handle); // exact avail value
	if(frames < 0)
	{
		printf("space: samples: %d, frames: %d, err: %s\n", (int)ao->samples, (int)frames, snd_strerror(frames));
		return 0;
	}
	return ao->samples <= frames ? 0 : ao->samples - frames;
}

static int alsa_set_volume(void* object, int v)
{
	snd_mixer_t* mixer = NULL;
	int r = alsa_mixer_load(DEVICE_NAME, &mixer);
	if (0 != r)
	{
		printf("alsa_mixer_load(%s) error: %d, %s\n", DEVICE_NAME, r, snd_strerror(r));
		return r;
	}

	r = alsa_mixer_set_volume(mixer, v);
	snd_mixer_close(mixer);
	return r;
}

static int alsa_get_volume(void* object, int* v)
{
	snd_mixer_t* mixer = NULL;
	int r = alsa_mixer_load(DEVICE_NAME, &mixer);
	if (0 != r)
	{
		printf("alsa_mixer_load(%s) error: %d, %s\n", DEVICE_NAME, r, snd_strerror(r));
		return r;
	}

	r = alsa_mixer_get_volume(mixer, v);
	snd_mixer_close(mixer);
	return r;
}

int alsa_player_register()
{
	static audio_output_t ao;
	memset(&ao, 0, sizeof(ao));
	ao.open = alsa_open;
	ao.close = alsa_close;
	ao.write = alsa_write;
	ao.play = alsa_play;
	ao.pause = alsa_pause;
	ao.reset = alsa_reset;
	ao.get_samples = alsa_get_samples;
	ao.get_volume = alsa_get_volume;
	ao.set_volume = alsa_set_volume;
	return av_set_class(AV_AUDIO_PLAYER, "alsa", &ao);
}
