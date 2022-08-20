#include "ffresample.h"
#include "libswresample/swresample.h"

void* ffresample_create(enum AVSampleFormat in_format, int in_sample_rate, int in_channels, enum AVSampleFormat out_format, int out_sample_rate, int out_channels)
{
    int r;
    SwrContext* swr = NULL;
    AVChannelLayout in_layout, out_layout;
    av_channel_layout_default(&in_layout, in_channels);
    av_channel_layout_default(&out_layout, out_channels);

#if LIBSWRESAMPLE_VERSION_MAJOR >= 4
    r = swr_alloc_set_opts2(&swr, &out_layout, out_format, out_sample_rate, &in_layout, in_format, in_sample_rate, 0, NULL);
    if (0 != r)
        return NULL;
#else
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
#endif

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
    SwrContext* swr;
    swr = (SwrContext*)ff;
    return swr_convert(swr, out_samples, out_count, in_samples, in_count);
}
