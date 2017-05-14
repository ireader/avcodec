#ifndef _opensles_input_h_
#define _opensles_input_h_

#include "avframe.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <android/log.h>
#include <stdint.h>
#include <stdlib.h>

#define OPENSLES_BUFFERS 10

#ifndef SL_ANDROID_DATAFORMAT_PCM_EX
/* The following pcm representations and data formats map to those in OpenSLES 1.1 */
#define SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT       ((SLuint32) 0x00000001)
#define SL_ANDROID_PCM_REPRESENTATION_UNSIGNED_INT     ((SLuint32) 0x00000002)
#define SL_ANDROID_PCM_REPRESENTATION_FLOAT            ((SLuint32) 0x00000003)

#define SL_ANDROID_DATAFORMAT_PCM_EX    ((SLuint32) 0x00000004)

typedef struct SLAndroidDataFormat_PCM_EX_ {
	SLuint32         formatType;
	SLuint32         numChannels;
	SLuint32         sampleRate;
	SLuint32         bitsPerSample;
	SLuint32         containerSize;
	SLuint32         channelMask;
	SLuint32         endianness;
	SLuint32         representation;
} SLAndroidDataFormat_PCM_EX;
#endif

struct opensles_recorder_t
{
	SLObjectItf engineObject;
	SLObjectItf recorderObject; // recorder object

	SLEngineItf engine; // engine interface
	SLRecordItf record; // record interface
	SLAndroidSimpleBufferQueueItf bufferQ; // buffer queue interface

	int bytes_per_sample;
	int samples_per_buffer;

	sl_uint8_t* ptr;
	size_t offset;

	audio_input_callback cb;
	void* param;
};

#define CHECK_OPENSL_ERROR(ret__, ...) \
    do { \
    	if ((ret__) != SL_RESULT_SUCCESS) \
    	{ \
    		__android_log_print(ANDROID_LOG_ERROR, "SLES", __VA_ARGS__); \
    		return ret; \
    	} \
    } while (0)

#endif /* !_opensles_input_h_ */
