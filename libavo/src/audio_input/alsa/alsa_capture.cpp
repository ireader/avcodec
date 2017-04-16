#include "audio_input.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include "npr.h"

#define DEVICE_NAME "default"

struct alsa_audio_in_t
{
	snd_pcm_t* handle;
	int channels;
	int bits_per_sample;
	int samples_per_seconds;
	audio_input_callback cb;
	void* param;

	OSThreadID tid;
	bool running;
};

static int Worker(void* param)
{
	alsa_audio_in_t* ai = (alsa_audio_in_t*)param;

	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_current(ai->handle, params);

	snd_pcm_uframes_t frames = 0;
	snd_pcm_hw_params_get_period_size(params, &frames, NULL);

	int total = frames * ai->bits_per_sample * ai->channels / 8;
	char* buffer = (char*)malloc(total);
	while(ai->running)
	{
		int r = snd_pcm_readi(ai->handle, buffer, frames);
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
			ai->cb(ai->param, buffer, r*ai->bits_per_sample*ai->channels / 8);
		}
	}

	free(buffer);
	return 0;
}

static int IsOpened(void* object)
{
	alsa_audio_in_t* ai = (alsa_audio_in_t*)object;
	return NULL==ai->handle?0:1;
}

static int Close(void* object)
{
	alsa_audio_in_t* ai = (alsa_audio_in_t*)object;
	if(IsOpened(ai))
	{
		ai->running = false;
		OSCloseThread(ai->tid);

		snd_pcm_drain(ai->handle);

		snd_pcm_close(ai->handle);
	}

	free(ai);
	return 0;
}

static void* Open(int channels, int bits_per_samples, int samples_per_seconds, audio_input_callback cb, void* param)
{
	alsa_audio_in_t* ai = (alsa_audio_in_t*)malloc(sizeof(alsa_audio_in_t));
	if(NULL == ai)
		return NULL;
	memset(ai, 0, sizeof(alsa_audio_in_t));

	snd_pcm_format_t format;
	if(bits_per_samples == 8)
		format = SND_PCM_FORMAT_U8;
	else if(bits_per_samples == 16)
		format = SND_PCM_FORMAT_S16_LE;
	else
		return NULL;

	int r = snd_pcm_open(&ai->handle, DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
	if(r < 0)
	{
		printf("alas open capture error:%s\r\n", snd_strerror(r));
		Close(ai);		
		return NULL;
	}

	snd_pcm_hw_params_t* params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(ai->handle, params);

	snd_pcm_hw_params_set_access(ai->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(ai->handle, params, format);
	snd_pcm_hw_params_set_channels(ai->handle, params, channels);

	unsigned int val = samples_per_seconds;
	snd_pcm_hw_params_set_rate_near(ai->handle, params, &val, 0);

	snd_pcm_uframes_t frames = samples_per_seconds*bits_per_samples/8/25; // 25 fps
	snd_pcm_hw_params_set_period_size_near(ai->handle, params, &frames, 0);

	r = snd_pcm_hw_params(ai->handle, params);
	if(r < 0)
	{
		printf("snd_pcm_hw_params: %s\r\n", snd_strerror(r));
		Close(ai);
		return NULL;
	}

	ai->running = true;
	OSCreateThread(&ai->tid, NULL, Worker, ai, 0);

	ai->cb = cb;
	ai->param = param;
	ai->channels = channels;
	ai->bits_per_sample = bits_per_samples;
	ai->samples_per_seconds = samples_per_seconds;
	return ai;
}

static int Pause(void* object)
{
	alsa_audio_in_t* ai = (alsa_audio_in_t*)object;
	return snd_pcm_pause(ai->handle, 1);
}

static int Reset(void* object)
{
	alsa_audio_in_t* ai = (alsa_audio_in_t*)object;
	snd_pcm_reset(ai->handle);
	return 0;
}

static int SetVolume(void* /*object*/, int v)
{
	snd_mixer_t* mixer = NULL;
	int r = snd_mixer_open(&mixer, 0);
	if(r < 0)
	{
		printf("setvolume snd_mixer_open error: %d, %s\n", r, snd_strerror(r));
		return r;
	}

	r = snd_mixer_attach(mixer, DEVICE_NAME);
	if(r < 0)
	{
		printf("setvolume snd_mixer_attach error: %d, %s\n", r, snd_strerror(r));
		snd_mixer_close(mixer);
		return r;
	}

	r = snd_mixer_selem_register(mixer, NULL, NULL);
	if(r < 0)
	{
		printf("setvolume snd_mixer_selem_register error: %d, %s\n", r, snd_strerror(r));
		snd_mixer_close(mixer);
		return r;
	}

	r = snd_mixer_load(mixer);
	if(r < 0)
	{
		printf("setvolume snd_mixer_load error: %d, %s\n", r, snd_strerror(r));
		snd_mixer_close(mixer);
		return r;
	}

	for(snd_mixer_elem_t* e=snd_mixer_first_elem(mixer); e; e=snd_mixer_elem_next(e))
	{
		if(SND_MIXER_ELEM_SIMPLE==snd_mixer_elem_get_type(e) 
			&& snd_mixer_selem_is_active(e)
			&& 0!=snd_mixer_selem_has_capture_volume(e))
		{
			long min = 0;
			long max = 0;
			snd_mixer_selem_get_capture_volume_range(e, &min, &max);
			
			v = ((v&0xFFFF) * (max-min) + 0xFFFF/2)/0xFFFF; // map (0-0xFFFF) to volume range
			printf("mixer capture volume range: %ld-%ld, set: %d\n", min, max, v);

			snd_mixer_selem_set_capture_volume_all(e, v);
			snd_mixer_close(mixer);
			return 0;
		}
	}

	snd_mixer_close(mixer);
	return -1;
}

static int GetVolume(void* /*object*/)
{
	snd_mixer_t* mixer = NULL;
	int r = snd_mixer_open(&mixer, 0);
	if(r < 0)
	{
		printf("getvolume snd_mixer_open error: %d, %s\n", r, snd_strerror(r));
		return r;
	}

	r = snd_mixer_attach(mixer, DEVICE_NAME);
	if(r < 0)
	{
		printf("getvolume snd_mixer_attach error: %d, %s\n", r, snd_strerror(r));
		snd_mixer_close(mixer);
		return r;
	}

	r = snd_mixer_selem_register(mixer, NULL, NULL);
	if(r < 0)
	{
		printf("getvolume snd_mixer_selem_register error: %d, %s\n", r, snd_strerror(r));
		snd_mixer_close(mixer);
		return r;
	}

	r = snd_mixer_load(mixer);
	if(r < 0)
	{
		printf("getvolume snd_mixer_load error: %d, %s\n", r, snd_strerror(r));
		snd_mixer_close(mixer);
		return r;
	}

	for(snd_mixer_elem_t* e=snd_mixer_first_elem(mixer); e; e=snd_mixer_elem_next(e))
	{
		if(SND_MIXER_ELEM_SIMPLE==snd_mixer_elem_get_type(e) 
			&& snd_mixer_selem_is_active(e)
			&& 0!=snd_mixer_selem_has_capture_volume(e))
		{
			long min = 0;
			long max = 0;
			snd_mixer_selem_get_capture_volume_range(e, &min, &max);
			
			long v = 0;
			snd_mixer_selem_get_capture_volume(e, SND_MIXER_SCHN_FRONT_LEFT, &v);
			printf("mixer capture volume range[%ld-%ld], current: %ld", min, max, v);

			v = (v*0xFFFF+(max-min)/2)/(max-min); // map volume range to (0-0xFFFF)
			printf(" map: %ld\n", v);

			snd_mixer_close(mixer);
			return v;
		}
	}
	snd_mixer_close(mixer);
	return -1;
}

extern "C" int alsa_capture_register()
{
	static audio_input_t ai;
	memset(&ai, 0, sizeof(ai));
	ai.open = Open;
	ai.close = Close;
	ai.isopened = IsOpened;
	ai.getvolume = GetVolume;
	ai.setvolume = SetVolume;
	ai.pause = Pause;
	ai.reset = Reset;
	return av_set_class(AV_AUDIO_CAPTURE, "alsa", &ai);
}
