#include "audio_input.h"
#include "av_register.h"
#include <stdlib.h>

struct ai_context_t
{
	const audio_input_t* ai;
	void* id;
};

void* audio_input_open(int channels, int bits_per_sample, int samples_per_second, audio_input_callback cb, void* param)
{
	struct ai_context_t* h;
	h = (struct ai_context_t*)malloc(sizeof(struct ai_context_t));
	if (NULL == h)
		return NULL;

	h->ai = (const audio_input_t*)av_get_class(AV_AUDIO_CAPTURE);
	if (NULL == h->ai)
	{
		audio_input_close(h);
		return NULL;
	}

	h->id = h->ai->open(channels, bits_per_sample, samples_per_second, cb, param);
	if (NULL == h->id)
	{
		audio_input_close(h);
		return NULL;
	}

	return h;
}

int audio_input_close(void* ai)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	if (h->ai && h->id)
		h->ai->close(h->id);
	free(h);
	return 0;
}

int audio_input_isopened(void* ai)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	return (h->ai&&h->ai->isopened)?h->ai->isopened(h->id):-1;
}

int audio_input_pause(void* ai)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	return (h->ai&&h->ai->pause)?h->ai->pause(h->id):-1;
}

int audio_input_reset(void* ai)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	return (h->ai&&h->ai->reset)?h->ai->reset(h->id):-1;
}

int audio_input_setvolume(void* ai, int v)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	return (h->ai&&h->ai->setvolume)?h->ai->setvolume(h->id, v):-1;
}

int audio_input_getvolume(void* ai)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	return (h->ai&&h->ai->getvolume)?h->ai->getvolume(h->id):-1;
}