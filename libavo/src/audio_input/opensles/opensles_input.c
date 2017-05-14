#include "audio_input.h"
#include "opensles_input.h"
#include "opensles_engine.h"
#include "opensles_format.h"
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

static void* opensles_open(int channels, int rate, int format, int samples, audio_input_callback cb, void* param)
{
	int r;
	SLAndroidDataFormat_PCM_EX pcm;
	struct opensles_recorder_t* recorder;
	recorder = (struct opensles_recorder_t*)malloc(sizeof(*recorder));
	if (NULL == recorder)
		return NULL;
	memset(recorder, 0, sizeof(struct opensles_recorder_t));
	opensles_format(&pcm, channels, rate, format);

	recorder->cb = cb;
	recorder->param = param;
	recorder->bytes_per_sample = channels * PCM_SAMPLE_BITS(format) / 8;
	recorder->samples_per_buffer = samples;
	recorder->ptr = (sl_uint8_t*)malloc(OPENSLES_BUFFERS * recorder->samples_per_buffer * recorder->bytes_per_sample);
	
	if (NULL == recorder->ptr
		|| 0 != opensles_engine_create(&recorder->engineObject, &recorder->engine)
		|| 0 != opensles_recorder_create(recorder, &pcm)
		|| 0 != opensles_recorder_register_callback(recorder))
	{
		opensles_close(recorder);
		return NULL;
	}

	return recorder;
}

static int opensles_start(void* p)
{
	struct opensles_recorder_t* recorder;
	recorder = (struct opensles_recorder_t*)p;
	return (*recorder->record)->SetRecordState(recorder->record, SL_RECORDSTATE_RECORDING);
}

static int opensles_stop(void* p)
{
	struct opensles_recorder_t* recorder;
	recorder = (struct opensles_recorder_t*)p;
	return (*recorder->record)->SetRecordState(recorder->record, SL_RECORDSTATE_STOPPED);
}

int opensles_recorder_register()
{
	static audio_input_t ai;
	memset(&ai, 0, sizeof(ai));
	ai.open = opensles_open;
	ai.close = opensles_close;
	ai.start = opensles_start;
	ai.stop = opensles_stop;
	return av_set_class(AV_AUDIO_RECORDER, "opensles", &ai);
}
