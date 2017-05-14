#ifndef _opensles_recorder_h_
#define _opensles_recorder_h_

#include "opensles_input.h"

static int opensles_recorder_create(struct opensles_recorder_t* recorder, SLAndroidDataFormat_PCM_EX* format)
{
	int ret;
	const SLboolean requireds[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	const SLInterfaceID interfaces[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION };

	SLDataLocator_IODevice device = { SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT, SL_DEFAULTDEVICEID_AUDIOINPUT, NULL };
	SLDataSource inputSource = { &device, NULL };

	SLDataLocator_AndroidSimpleBufferQueue locator = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, OPENSLES_BUFFERS };
	SLDataSink inputSink = { &locator, format };

	SLAndroidConfigurationItf config;
	SLint32 audio_device = SL_ANDROID_RECORDING_PRESET_GENERIC; // SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION

	ret = (*recorder->engine)->CreateAudioRecorder(recorder->engine, &recorder->recorderObject, &inputSource, &inputSink, sizeof(interfaces) / sizeof(interfaces[0]), interfaces, requireds);
	CHECK_OPENSL_ERROR(ret, "%s: SLEngine->CreateAudioRecorder() failed", __FUNCTION__);

	// android configuration
	ret = (*recorder->recorderObject)->GetInterface(recorder->recorderObject, SL_IID_ANDROIDCONFIGURATION, &config);
	CHECK_OPENSL_ERROR(ret, "%s: SLPlayerObject->GetInterface(SL_IID_ANDROIDCONFIGURATION) failed", __FUNCTION__);

	// set record device
	ret = (*config)->SetConfiguration(config, SL_ANDROID_KEY_RECORDING_PRESET, &audio_device, sizeof(SLint32));
	CHECK_OPENSL_ERROR(ret, "%s: SLAndroidConfigurationItf->SetConfiguration(SL_ANDROID_KEY_RECORDING_PRESET: %d) failed", __FUNCTION__, (int)audio_device);

	// initialize
	ret = (*recorder->recorderObject)->Realize(recorder->recorderObject, SL_BOOLEAN_FALSE);
	CHECK_OPENSL_ERROR(ret, "%s: SLRecorderObject->Realize() failed", __FUNCTION__);

	// record
	ret = (*recorder->recorderObject)->GetInterface(recorder->recorderObject, SL_IID_RECORD, &recorder->record);
	CHECK_OPENSL_ERROR(ret, "%s: SLRecorderObject->GetInterface(SL_IID_RECORD) failed", __FUNCTION__);

	// android buffer queue
	ret = (*recorder->recorderObject)->GetInterface(recorder->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &recorder->bufferQ);
	CHECK_OPENSL_ERROR(ret, "%s: SLRecorderObject->GetInterface(SL_IID_ANDROIDSIMPLEBUFFERQUEUE) failed", __FUNCTION__);

	return 0;
}

static int opensles_recorder_destroy(struct opensles_recorder_t* recoder)
{
	if (recoder->record)
	{
		(*recoder->record)->SetRecordState(recoder->record, SL_RECORDSTATE_STOPPED);
		recoder->record = NULL;
	}

	if (recoder->bufferQ)
	{
		(*recoder->bufferQ)->Clear(recoder->bufferQ);
		recoder->bufferQ = NULL;
	}

	if (recoder->recorderObject) {
		(*recoder->recorderObject)->Destroy(recoder->recorderObject);
		recoder->recorderObject = NULL;
	}

	return 0;
}

#endif /* !_opensles_recorder_h_ */
