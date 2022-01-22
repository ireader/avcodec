#include "ffresample.h"
#include "libswresample/swresample.h"

void* ffresample_create(enum AVSampleFormat in_format, int in_sample_rate, int in_channels, enum AVSampleFormat out_format, int out_sample_rate, int out_channels)
{
    int r;
    SwrContext* swr;

    swr = swr_alloc_set_opts(NULL,
        av_get_default_channel_layout(out_channels),
        out_format,
        out_sample_rate,
        av_get_default_channel_layout(in_channels),
        in_format,
        in_sample_rate,
        0, NULL);
    if (!swr) {
        return NULL;
    }

    r = swr_init(swr);
    if (r < 0) {
        swr_free(&swr);
        return NULL;
    }

    return swr;
}

void ffresample_destroy(void* ff)
{
    SwrContext* swr;
    swr = (SwrContext*)ff;
    swr_free(&swr);
}

int ffresample_convert(void* ff, const uint8_t** in_samples, int in_count, uint8_t** out_samples, int out_count)
{
    int r;
    SwrContext* swr;
    swr = (SwrContext*)ff;

    r = swr_convert(swr, out_samples, out_count, in_samples, in_count);
    if (r < 0) {
        return r;
    }

    return r;
}
