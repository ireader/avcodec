#include <AudioToolbox/AudioToolbox.h>
#include "audio_input.h"
#include "av_register.h"
#include "avframe.h"
#include <assert.h>
#include <stdio.h>

struct audio_queue_recorder_t
{
    AudioQueueRef queue;
    AudioQueueBufferRef buffers[ 4 ];
    AudioStreamBasicDescription format;

    UInt32 running;
    audio_input_callback cb;
    void* param;
};

static void audio_queue_input_callback(void * __nullable inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer, const AudioTimeStamp *inStartTime, UInt32 inNumberPacketDescriptions, const AudioStreamPacketDescription * __nullable inPacketDescs)
{
    OSStatus r;
    struct audio_queue_recorder_t* recorder;
    recorder = (struct audio_queue_recorder_t*)inUserData;
    
    //this is called by the audio queue when it has finished decoding our data.
    //The buffer is now free to be reused.

    //printf("audio_queue_input_callback[%d] samples: %d\n", (int)((AudioQueueBufferRef*)inBuffer->mUserData - recorder->buffers), inBuffer->mAudioDataByteSize/recorder->format.mBytesPerFrame);
    if(recorder->cb)
        recorder->cb(recorder->param, inBuffer->mAudioData, inBuffer->mAudioDataByteSize/recorder->format.mBytesPerFrame);
    
    if(recorder->running)
    {
        r = AudioQueueEnqueueBuffer(recorder->queue, inBuffer, 0, 0 );
        if(0 != r)
            printf("AudioQueueEnqueueBuffer[%d] error: %d\n", (int)((AudioQueueBufferRef*)inBuffer->mUserData - recorder->buffers), r);
    }
}

static void audio_queue_property_listener(void * __nullable inUserData, AudioQueueRef inAQ, AudioQueuePropertyID inID)
{
    UInt32 size;
    OSStatus err;
    struct audio_queue_recorder_t* recorder;
    recorder = (struct audio_queue_recorder_t*)inUserData;
    
    size = sizeof(recorder->running);
    err = AudioQueueGetProperty(recorder->queue, kAudioQueueProperty_IsRunning, &recorder->running, &size);
    if (err)
    {
        printf("get kAudioQueueProperty_IsRunning");
        return;
    }

    //printf("get kAudioQueueProperty_IsRunning = %d\n", recorder->running);
}

static int audio_queue_record_close(void* object)
{
    int i;
    OSStatus ret;
    struct audio_queue_recorder_t* recorder;
    recorder = (struct audio_queue_recorder_t*)object;
    
    if(!recorder)
        return 0;
    
    ret = 0;
    recorder->running = 0;
    if(recorder->queue)
    {
        // Flush data, to make sure we play to the end.
        //AudioQueueFlush(player->queue);
        // Stop immediately/asynchronously.
        AudioQueueStop(recorder->queue, 1);
    }
    
    for(i = 0; i < sizeof(recorder->buffers)/sizeof(recorder->buffers[0]); i++)
    {
        if(recorder->buffers[i])
        {
            AudioQueueFreeBuffer(recorder->queue, recorder->buffers[i]);
            recorder->buffers[i] = NULL;
        }
    }
    
    if(recorder->queue)
    {
        ret = AudioQueueDispose(recorder->queue, 1);
        recorder->queue = NULL;
    }

    free(recorder);
    return ret;
}

static void* audio_queue_record_open(int channels, int samples_per_second, int fmt, int samples, audio_input_callback cb, void* param)
{
    // For linear PCM, only interleaved formats are supported. Compressed formats are supported.
    
    int i;
    OSStatus status;
    UInt32 dataFormatSize;
    AudioStreamBasicDescription* format;
    struct audio_queue_recorder_t* recorder;
    recorder = (struct audio_queue_recorder_t*)calloc(1, sizeof(*recorder));
    if(!recorder)
        return NULL;
    
    recorder->cb = cb;
    recorder->param = param;
    // The audio queue format specification.  This structure must be set
    // to values congruent to the ones we use with ov_read().
    format = &recorder->format;

    // First, the constants.  The format is quite hard coded, both here
    // and in the calls to ov_read().
    format->mFormatID = kAudioFormatLinearPCM;
    format->mFormatFlags = (PCM_SAMPLE_FLOAT(fmt) ? kAudioFormatFlagIsFloat : ((PCM_SAMPLE_FMT_U8==fmt || PCM_SAMPLE_FMT_U8P==fmt) ? 0 : kAudioFormatFlagIsSignedInteger)) | (PCM_SAMPLE_PLANAR(fmt) ? kAudioFormatFlagIsNonInterleaved : 0);
    //format->mFormatFlags |= kLinearPCMFormatFlagIsPacked;
    format->mFramesPerPacket = 1;
    format->mBitsPerChannel = PCM_SAMPLE_BITS(fmt);
    // Load the sample rate and channel number from the vorbis file.
    format->mSampleRate = samples_per_second;
    format->mChannelsPerFrame = channels;
    // The number of bytes depends on the channel number.
    format->mBytesPerPacket = format->mBytesPerFrame = channels * PCM_SAMPLE_BITS(fmt) / 8; // times two, for 16bit

    // Create the audio queue with the desired format.  Notice that we
    // use the OggVorbis_File pointer as the data far the callback.
    status = AudioQueueNewInput(format, audio_queue_input_callback, recorder, NULL, NULL, 0, &recorder->queue );
    if (0 != status) {
        printf( "AudioQueueNewInput status = %d\n", status );
        audio_queue_record_close(recorder);
        return NULL;
    }
    
    dataFormatSize = sizeof (recorder->format);
    status = AudioQueueGetProperty(recorder->queue, kAudioQueueProperty_StreamDescription, &recorder->format, &dataFormatSize);
    if (0 != status) {
        printf( "AudioQueueGetProperty status = %d\n", status );
        audio_queue_record_close(recorder);
        return NULL;
    }

    // For me distortions happen with 3 buffers; hence the magic number 5.
    for ( i = 0; i < sizeof(recorder->buffers) / sizeof (recorder->buffers[0]); i++ )
    {
        // For each buffer...

        // The size of the buffer is a magic number.  4096 is good enough, too.
        status = AudioQueueAllocateBuffer( recorder->queue, samples * format->mBytesPerFrame, &recorder->buffers[ i ] );
        if (0 != status) {
            printf( "AudioQueueAllocateBuffer status = %d\n", status );
            audio_queue_record_close(recorder);
            return NULL;
        }
        
        recorder->buffers[i]->mUserData = &recorder->buffers[i];

        // Enqueue buffers, before play.  According to the process outlined
        // in the Audio Queue Services Programming Guide, we must do this
        // before calling AudioQueueStart() and it's simplest to do it like
        // this.
        status = AudioQueueEnqueueBuffer( recorder->queue, recorder->buffers[i], 0, 0 );
        if(0 != status)
            printf("AudioQueueEnqueueBuffer[%d] error: %d\n", i, status);
    }

    // Here, we might want to call AudioQueuePrime if we were playing one
    // of the supported compressed formats.  However, since we only have
    // raw PCM buffers to play, I don't see the point.  Maybe playing will
    // start faster with it, after AudioQueueStart() but I still don't see
    // the point for this example; if there's a delay, it'll happen anyway.

    // We add a listener for the start/stop event, so we know when to call
    // exit( 0 ) and terminate the application.  We also give it the vf
    // pointer, even though it's not used in our listener().
    status = AudioQueueAddPropertyListener(recorder->queue, kAudioQueueProperty_IsRunning, audio_queue_property_listener, recorder);
    if (0 != status) {
        printf( "AudioQueueAddPropertyListener status = %d\n", status );
        audio_queue_record_close(recorder);
        return NULL;
    }
    
    return recorder;
}

static int audio_queue_record_start(void* object)
{
    struct audio_queue_recorder_t* recorder;
    recorder = (struct audio_queue_recorder_t*)object;
    return AudioQueueStart( recorder->queue, NULL);
}

static int audio_queue_record_stop(void* object)
{
    struct audio_queue_recorder_t* recorder;
    recorder = (struct audio_queue_recorder_t*)object;
    recorder->running = 0;
    return AudioQueueStop(recorder->queue, 1);
}

int audio_queue_recorder_register(void)
{
    static audio_input_t ai;
    memset(&ai, 0, sizeof(ai));
    ai.open = audio_queue_record_open;
    ai.close = audio_queue_record_close;
    ai.start = audio_queue_record_start;
    ai.stop = audio_queue_record_stop;
    return av_set_class(AV_AUDIO_RECORDER, "audio_queue", &ai);
}
