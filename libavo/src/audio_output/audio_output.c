#include "audio_output.h"
#include "av_register.h"
#include <stdlib.h>

struct ao_context_t
{
	const audio_output_t* ao;
	void* id;
};

void* audio_output_open(int channels, int samples_per_second, int format, int samples)
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

	h->id = h->ao->open(channels, samples_per_second, format, samples);
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
	if(h && h->ao && h->ao->close && h->id)
		h->ao->close(h->id);
	free(h);
	return 0;
}

int audio_output_write(void* ao, const void* pcm, int samples)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return (h && h->ao && h->ao->write && h->id) ? h->ao->write(h->id, pcm, samples) : -1;
}

int audio_output_play(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return (h && h->ao && h->ao->play && h->id) ? h->ao->play(h->id) : -1;
}

int audio_output_pause(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return (h && h->ao && h->ao->pause && h->id) ? h->ao->pause(h->id) : -1;
}

int audio_output_reset(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return (h && h->ao && h->ao->reset && h->id) ? h->ao->reset(h->id) : -1;
}

int audio_output_getsamples(void* ao)
{
	struct ao_context_t* h = (struct ao_context_t*)ao;
	return (h && h->ao && h->ao->get_samples && h->id) ? h->ao->get_samples(h->id) : -1;
}
