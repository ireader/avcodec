#include "audio_input.h"
#include "opensles_input.h"
#include "opensles_engine.h"
#include "opensles_recorder.h"
#include "opensles_callback.h"
#include "av_register.h"
#include <string.h>
#include <math.h>

static int opensles_close(void* p)
{
	struct opensles_recorder_t* recorder;
	recorder = (struct opensles_recorder_t*)p;
	opensles_recorder_destroy(recorder);
	opensles_engine_destroy(&recorder->engineObject, &recorder->engine);

	if (recorder->ptr)
	{
		free(recorder->ptr);
		recorder->ptr = NULL;
	}
	free(recorder);
}

static void* opensles_open(int channels, int bits_per_samples, int samples_per_seconds, audio_input_callback cb, void* param)
{
	int r;
	struct opensles_recorder_t* recorder;
	recorder = (struct opensles_recorder_t*)malloc(sizeof(*recorder));
	if (NULL == recorder)
		return NULL;
	memset(recorder, 0, sizeof(struct opensles_recorder_t));
	recorder->cb = cb;
	recorder->param = param;
	recorder->channels = channels;
	recorder->sample_bits = bits_per_samples;
	recorder->sample_rate = samples_per_seconds;
	recorder->bytes_per_sample = recorder->channels * recorder->sample_bits / 8;
	recorder->samples_per_buffer = recorder->sample_rate * OPENSLES_TIME / 1000;
	recorder->ptr = (sl_uint8_t*)malloc(OPENSLES_BUFFERS * recorder->samples_per_buffer * recorder->bytes_per_sample);
	
	if (NULL == recorder->ptr
		|| 0 != opensles_engine_create(&recorder->engineObject, &recorder->engine)
		|| 0 != opensles_recorder_create(recorder, channels, bits_per_samples, samples_per_seconds)
		|| 0 != opensles_recorder_start(recorder))
	{
		opensles_close(recorder);
		return NULL;
	}

	return recorder;
}

static int opensles_pause(void* p)
{
	struct opensles_recorder_t* recorder;
	recorder = (struct opensles_recorder_t*)p;

	if (recorder->record)
	{
		return (*recorder->record)->SetRecordState(recorder->record, SL_RECORDSTATE_PAUSED);
	}
	return -1;
}

static int opensles_flush(void* p)
{
	struct opensles_recorder_t* recorder;
	recorder = (struct opensles_recorder_t*)p;

	if (recorder->bufferQ)
	{
		return (*recorder->bufferQ)->Clear(recorder->bufferQ);
	}
	return -1;
}

int opensles_recorder_register()
{
	static audio_input_t ai;
	memset(&ai, 0, sizeof(ai));
	ai.open = opensles_open;
	ai.close = opensles_close;
	ai.pause = opensles_pause;
	ai.reset = opensles_flush;
	return av_set_class(AV_AUDIO_RECORDER, "opensles", &ai);
}
