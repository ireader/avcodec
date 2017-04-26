#include "audio_output.h"
#include "av_register.h"
#include <stdlib.h>

struct ao_context_t
{
	const audio_output_t* ao;
	void* id;
};

void* audio_output_open(int channels, int bits_per_sample, int samples_per_second)
{
	struct ao_context_t* h;
	h = (struct ao_context_t*)malloc(sizeof(struct ao_context_t));
	if (NULL == h)
		return NULL;
	
	h->ao = (const audio_output_t*)av_get_class(AV_AUDIO_PLAYER);
	if (NULL == h->ao)
	{
		audio_output_close(h);
		return NULL;
	}

	h->id = h->ao->open(channels, bits_per_sample, samples_per_second);
	if (NULL == h->id)
	{
		audio_output_close(h);
		return NULL;
	}

	return h;
}

int audio_output_close(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	if(h->ao && h->id)
		h->ao->close(h->id);
	free(h);
	return 0;
}

int audio_output_write(void* ao, const void* samples, int count)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return h->ao->write(h->id, samples, count);
}

int audio_output_play(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return h->ao->play(h->id);
}

int audio_output_pause(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return h->ao->pause(h->id);
}

int audio_output_reset(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return h->ao->reset(h->id);
}

int audio_output_getbuffersize(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return h->ao->get_buffer_size(h->id);
}

int audio_output_getavailablesamples(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return h->ao->get_available_sample(h->id);
}

int audio_output_getinfo(void* ao, int *channel, int *bits_per_sample, int *samples_per_second)
{
	struct ao_context_t* h;
	h = (struct ao_context_t*)ao;
	return h->ao->get_info(h->id, channel, bits_per_sample, samples_per_second);
}

int audio_output_getvolume(void* ao, int* v)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return (h->ao&&h->ao->get_volume)?h->ao->get_volume(h->id, v):-1;
}

int audio_output_setvolume(void* ao, int v)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return (h->ao&&h->ao->set_volume)?h->ao->set_volume(h->id, v):-1;
}
