#include "audio_input.h"
#include "av_register.h"
#include <stdlib.h>

struct ai_context_t
{
	const audio_input_t* ai;
	void* id;
};

void* audio_input_open(int channels, int frequency, int format, int frames, audio_input_callback cb, void* param)
{
	struct ai_context_t* h;
	h = (struct ai_context_t*)malloc(sizeof(struct ai_context_t));
	if (NULL == h)
		return NULL;

	h->ai = (const audio_input_t*)av_get_class(AV_AUDIO_RECORDER);
	if (NULL == h->ai)
	{
		audio_input_close(h);
		return NULL;
	}

	h->id = h->ai->open(channels, frequency, format, frames, cb, param);
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
	if (h && h->ai && h->ai->close && h->id)
		h->ai->close(h->id);
	free(h);
	return 0;
}

int audio_input_start(void* ai)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	return (h && h->ai && h->ai->start && h->id) ? h->ai->start(h->id) : -1;
}

int audio_input_stop(void* ai)
{
	struct ai_context_t* h = (struct ai_context_t*)ai;
	return (h && h->ai && h->ai->stop && h->id) ? h->ai->stop(h->id) : -1;
}
