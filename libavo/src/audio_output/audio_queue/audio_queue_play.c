#include <AudioToolbox/AudioToolbox.h>
#include "audio_output.h"
#include "av_register.h"
#include "avframe.h"
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>

struct audio_queue_player_t
{
    AudioQueueRef queue;
    AudioQueueBufferRef buffers[ 4 ];
    AudioQueueTimelineRef timeline;
    AudioStreamBasicDescription format;
    
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    
    Float64 samples;
    UInt32 running;
    atomic_int offset;
    atomic_int count;
};

static void audio_queue_output_callback(void * __nullable inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    OSStatus r;
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)inUserData;
    
    //this is called by the audio queue when it has finished decoding our data.
    //The buffer is now free to be reused.
    
    //printf("audio_queue_output_callback: %d, offset: %d, count: %d\n", (int)((AudioQueueBufferRef*)inBuffer->mUserData - player->buffers), player->offset, player->count);
    assert(inBuffer == player->buffers[ (player->offset + player->count) % (sizeof(player->buffers)/sizeof(player->buffers[0])) ]);
    r = atomic_fetch_add(&player->count, 1) + 1;
    assert(r <= sizeof(player->buffers)/sizeof(player->buffers[0]));

    //TODO: signal waiting thread that the buffer is free.
}

static void audio_queue_property_listener(void * __nullable inUserData, AudioQueueRef inAQ, AudioQueuePropertyID inID)
{
    UInt32 size;
    OSStatus err;
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)inUserData;
    
    size = sizeof(player->running);
    err = AudioQueueGetProperty(player->queue, kAudioQueueProperty_IsRunning, &player->running, &size);
    if (err)
    {
        printf("get kAudioQueueProperty_IsRunning");
        return;
    }
    
    pthread_mutex_lock(&player->mutex);
    pthread_cond_signal(&player->cond);
    pthread_mutex_unlock(&player->mutex);
    //printf("get kAudioQueueProperty_IsRunning = %d\n", player->running);
}

static int audio_queue_close(void* object)
{
    int i;
    OSStatus ret;
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)object;
    
    if(!player)
        return 0;
    
    ret = 0;
    if(player->queue)
    {
        // Flush data, to make sure we play to the end.
        //AudioQueueFlush(player->queue);
        // Stop immediately/asynchronously.
        AudioQueueStop(player->queue, 1);
    }
    
    for(i = 0; i < sizeof(player->buffers)/sizeof(player->buffers[0]); i++)
    {
        if(player->buffers[i])
        {
            AudioQueueFreeBuffer(player->queue, player->buffers[i]);
            player->buffers[i] = NULL;
        }
    }
    
    if(player->timeline)
    {
        AudioQueueDisposeTimeline(player->queue, player->timeline);
        player->timeline = NULL;
    }
    
    if(player->queue)
    {
        ret = AudioQueueDispose(player->queue, 1);
        player->queue = NULL;
    }
    
    pthread_cond_destroy(&player->cond);
    pthread_mutex_destroy(&player->mutex);
    free(player);
    return ret;
}

static void* audio_queue_open(int channels, int samples_per_second, int fmt, int samples)
{
    // For linear PCM, only interleaved formats are supported. Compressed formats are supported.
    
    int i;
    OSStatus status;
    pthread_mutexattr_t mutex;
    AudioStreamBasicDescription* format;
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)calloc(1, sizeof(*player));
    if(!player)
        return NULL;
    
    player->offset = 0;
    player->count = sizeof(player->buffers)/sizeof(player->buffers[0]);
    
    pthread_mutexattr_init(&mutex);
    pthread_mutex_init(&player->mutex, &mutex);
    pthread_cond_init(&player->cond, NULL);

    // The audio queue format specification.  This structure must be set
    // to values congruent to the ones we use with ov_read().
    format = &player->format;

    // First, the constants.  The format is quite hard coded, both here
    // and in the calls to ov_read().
    format->mFormatID = kAudioFormatLinearPCM;
    format->mFormatFlags = (PCM_SAMPLE_FLOAT(fmt) ? kAudioFormatFlagIsFloat : ((PCM_SAMPLE_FMT_U8==fmt || PCM_SAMPLE_FMT_U8P==fmt) ? 0 : kAudioFormatFlagIsSignedInteger)) | (PCM_SAMPLE_PLANAR(fmt) ? kAudioFormatFlagIsNonInterleaved : 0);
    format->mFramesPerPacket = 1;
    format->mBitsPerChannel = PCM_SAMPLE_BITS(fmt);
    // Load the sample rate and channel number from the vorbis file.
    format->mSampleRate = samples_per_second;
    format->mChannelsPerFrame = channels;
    // The number of bytes depends on the channel number.
    format->mBytesPerPacket = format->mBytesPerFrame = channels * PCM_SAMPLE_BITS(fmt) / 8; // times two, for 16bit

    // Create the audio queue with the desired format.  Notice that we
    // use the OggVorbis_File pointer as the data far the callback.
    status = AudioQueueNewOutput(format, audio_queue_output_callback, player, NULL, NULL, 0, &player->queue );
    if (0 != status) {
        printf( "AudioQueueNewOutput status = %d\n", status );
        audio_queue_close(player);
        return NULL;
    }

    // For me distortions happen with 3 buffers; hence the magic number 5.
    for ( i = 0; i < sizeof(player->buffers) / sizeof (player->buffers[0]); i++ )
    {
        // For each buffer...

        // The size of the buffer is a magic number.  4096 is good enough, too.
        status = AudioQueueAllocateBuffer( player->queue, samples * format->mBytesPerFrame / 4, &player->buffers[ i ] );
        if (0 != status) {
            printf( "AudioQueueAllocateBuffer status = %d\n", status );
            audio_queue_close(player);
            return NULL;
        }
        
        player->buffers[i]->mUserData = &player->buffers[i];

        // Enqueue buffers, before play.  According to the process outlined
        // in the Audio Queue Services Programming Guide, we must do this
        // before calling AudioQueueStart() and it's simplest to do it like
        // this.
        //audio_queue_output_callback(player, player->queue, player->buffers[ i ] );
    }

    // Here, we might want to call AudioQueuePrime if we were playing one
    // of the supported compressed formats.  However, since we only have
    // raw PCM buffers to play, I don't see the point.  Maybe playing will
    // start faster with it, after AudioQueueStart() but I still don't see
    // the point for this example; if there's a delay, it'll happen anyway.

    // We add a listener for the start/stop event, so we know when to call
    // exit( 0 ) and terminate the application.  We also give it the vf
    // pointer, even though it's not used in our listener().
    status = AudioQueueAddPropertyListener(player->queue, kAudioQueueProperty_IsRunning, audio_queue_property_listener, player);
    if (0 != status) {
        printf( "AudioQueueAddPropertyListener status = %d\n", status );
        audio_queue_close(player);
        return NULL;
    }
    
    status = AudioQueueCreateTimeline(player->queue, &player->timeline);
    if (0 != status) {
        printf( "AudioQueueCreateTimeline status = %d\n", status );
    }

    // A value from 0.5 to 2.0 indicating the rate at which the queue is to play. A value of
    // 1.0 (the default) indicates that the queue should play at its normal rate. Only
    // applicable when the time/pitch processor has been enabled and on Mac OS X 10.6 and higher.
    // kAudioQueueParam_PlayRate
    
    return player;
}

static int audio_queue_write(void* object, const void* pcm, int samples)
{
    int i, n;
    OSStatus r;
    AudioQueueBufferRef buffer;
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)object;
    
    for(i = 0; i < samples; i += n)
    {
        //TODO: signal waiting thread that the buffer is free.
        if(player->count < 1)
            return i; // don't have free buffer to write
        
        buffer = player->buffers[player->offset];
        if(buffer->mAudioDataBytesCapacity < (samples-i) * player->format.mBytesPerFrame)
            n = buffer->mAudioDataBytesCapacity / player->format.mBytesPerFrame;
        else
            n = samples - i;
        
        assert(buffer->mAudioDataBytesCapacity >= n * player->format.mBytesPerFrame);
        buffer->mAudioDataByteSize = n * player->format.mBytesPerFrame;
        memcpy(buffer->mAudioData, (uint8_t*)pcm + i * player->format.mBytesPerFrame, n * player->format.mBytesPerFrame);
        
        // update offset
        player->offset = (player->offset+1) % (sizeof(player->buffers)/sizeof(player->buffers[0]));
        r = atomic_fetch_sub(&player->count, 1) - 1;
        assert(r >= 0);
        
        r = AudioQueueEnqueueBuffer( player->queue, buffer, 0, 0 );
        if(r < 0)
        {
            printf( "AudioQueueEnqueueBuffer status = %d\n", r);
            return r;
        }
        
        //printf("AudioQueueEnqueueBuffer[%d] samples: %d\n", player->offset,  n);
        player->samples += n; // total sample
    }
    
    return i;
}

static int audio_queue_play(void* object)
{
    OSStatus r;
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)object;
    
    // And then start to play the file.
    r = AudioQueueStart( player->queue, NULL);
    if(0 == r)
    {
        pthread_mutex_lock(&player->mutex);
        pthread_cond_wait(&player->cond, &player->mutex);
        pthread_mutex_unlock(&player->mutex);
    }
    return r;
}

static int audio_queue_pause(void* object)
{
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)object;
    return AudioQueuePause(player->queue);
}

static int audio_queue_reset(void* object)
{
    struct audio_queue_player_t* player;
    player = (struct audio_queue_player_t*)object;
    return AudioQueueReset(player->queue);
}

static int audio_queue_get_samples(void* object)
{
    OSStatus r;
    Boolean discontinuity;
    AudioTimeStamp timestamp;
    struct audio_queue_player_t* player;
    
    player = (struct audio_queue_player_t*)object;
//    state = snd_pcm_state(ao->handle);
//    if(SND_PCM_STATE_RUNNING  != state)
//        return 0;

    r = AudioQueueGetCurrentTime(player->queue, player->timeline, &timestamp, &discontinuity);
    if(0 != r)
    {
        printf("AudioQueueGetCurrentTime %d\n", r);
        return 0;
    }
    
    //printf("sample time: %.1f, host time: %llu, remain: %.1f\n", timestamp.mSampleTime / player->format.mSampleRate, timestamp.mHostTime, (player->samples - timestamp.mSampleTime)/player->format.mSampleRate);
    if(kAudioTimeStampSampleTimeValid & timestamp.mFlags)
    {
        if(timestamp.mSampleTime > player->samples)
            player->samples = timestamp.mSampleTime; // reset total samples
        return player->samples - timestamp.mSampleTime;
    }
    
//    if(kAudioTimeStampHostTimeValid & timestamp.mFlags)
//        return player->samples > timestamp.mSampleTime ? player->samples - timestamp.mSampleTime : 0;
    
    return 0;
}

//static int audio_queue_set_volume(void* object, int v)
//{
//    struct audio_queue_player_t* player;
//    player = (struct audio_queue_player_t*)object;
//
//    // A value from 0.0 to 1.0 indicating the linearly scaled gain for the queue. A value of
//    // 1.0 (the default) indicates unity gain. A value of 0.0 indicates zero gain, or silence.
//    return AudioQueueSetParameter( player->queue, kAudioQueueParam_Volume, 1.0 );
//}

int audio_queue_player_register(void)
{
    static audio_output_t ao;
    memset(&ao, 0, sizeof(ao));
    ao.open = audio_queue_open;
    ao.close = audio_queue_close;
    ao.write = audio_queue_write;
    ao.play = audio_queue_play;
    ao.pause = audio_queue_pause;
    ao.reset = audio_queue_reset;
    ao.get_frames = audio_queue_get_samples;
    return av_set_class(AV_AUDIO_PLAYER, "audio_queue", &ao);
}
