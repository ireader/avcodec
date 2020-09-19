#include "audio-mixer.h"

void audio_mixer_float(float* dst, const float* src, float mul, int len)
{
    int i;
    for (i = 0; i < len; i++)
        dst[i] += src[i] * mul;
}

void audio_mixer_double(double* dst, const double* src, double mul, int len)
{
    int i;
    for (i = 0; i < len; i++)
        dst[i] += src[i] * mul;
}
