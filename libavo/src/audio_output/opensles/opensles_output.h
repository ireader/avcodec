#ifndef _opensles_output_h_
#define _opensles_output_h_

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>
#include <stdint.h>
#include <stdlib.h>

#define OPENSLES_TIME 10
#define OPENSLES_BUFFERS 200

struct opensles_player_t
{
	SLObjectItf engineObject;
	SLObjectItf outputObject; // output mix object
	SLObjectItf playerObject; // player object

	SLEngineItf engine; // engine interface
	SLPlayItf player; // player interface
	SLAndroidSimpleBufferQueueItf bufferQ; // buffer queue interface
	SLVolumeItf volume; // volume interface

	int channels;
	int sample_bits;
	int sample_rate;
	int bytes_per_sample;
	int samples_per_buffer;

	sl_uint8_t* ptr;
	size_t offset;
};

#define CHECK_OPENSL_ERROR(ret__, ...) \
    do { \
    	if ((ret__) != SL_RESULT_SUCCESS) \
    	{ \
    		__android_log_print(ANDROID_LOG_ERROR, "SLES", __VA_ARGS__); \
    		return ret; \
    	} \
    } while (0)

#endif /* !_opensles_output_h_ */
