#ifndef _opensles_callback_h_
#define _opensles_callback_h_

#define N(recorder) (recorder->bytes_per_sample * recorder->samples_per_buffer)

static void opensles_recorder_callback(SLAndroidSimpleBufferQueueItf bufferQueue, void *pContext)
{
	int i, ret;
	struct opensles_recorder_t* recorder;
	SLAndroidSimpleBufferQueueState state;

	recorder = (struct opensles_recorder_t*)pContext;
	assert(recorder->bufferQ == bufferQueue);

	ret = (*recorder->bufferQ)->GetState(recorder->bufferQ, &state);
	if (SL_RESULT_SUCCESS != ret)
	{
		__android_log_print(ANDROID_LOG_ERROR, "SLES", "SLAndroidSimpleBufferQueueItf->GetState() failed: %d", ret);
		return;
	}

	//__android_log_print(ANDROID_LOG_ERROR, "SLES", "GetState: count: %d, index: %d\n", (int)state.count, (int)state.index);

	assert(state.count <= OPENSLES_BUFFERS);
	state.index = (state.index - 1) % OPENSLES_BUFFERS;
	for (i = 0; i < OPENSLES_BUFFERS - state.count; ++i)
	{
		// capture callback
		recorder->cb(recorder->param, recorder->ptr + (i + state.index) * N(recorder), recorder->samples_per_buffer);

		// fill buffer
		ret = (*recorder->bufferQ)->Enqueue(recorder->bufferQ, recorder->ptr + (i + state.index) * N(recorder), N(recorder));
		if (SL_RESULT_SUCCESS != ret)
			__android_log_print(ANDROID_LOG_ERROR, "SLES", "SLAndroidSimpleBufferQueueItf->Enqueue() failed: %d", ret);
	}
}

static int opensles_recorder_register_callback(struct opensles_recorder_t* recorder)
{
	int i, ret;

	// fill buffer queue
	for (i = 0; i < OPENSLES_BUFFERS; ++i)
	{
		ret = (*recorder->bufferQ)->Enqueue(recorder->bufferQ, recorder->ptr + i * N(recorder), N(recorder));
		CHECK_OPENSL_ERROR(ret, "%s: SLAndroidSimpleBufferQueueItf->Enqueue() failed: %d", __FUNCTION__, ret);
	}

	// callback
	ret = (*recorder->bufferQ)->RegisterCallback(recorder->bufferQ, opensles_recorder_callback, recorder);
	CHECK_OPENSL_ERROR(ret, "%s: SLAndroidSimpleBufferQueueItf->RegisterCallback() failed: %d", __FUNCTION__, ret);

	return 0;
}

#undef N
#endif /* !_opensles_callback_h_ */
