#ifndef _fftranscode_h_
#define _fftranscode_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"

/// @param[in] decode FFmpeg decode parameters
/// @param[in] encode FFmpeg encode parameters
/// @param[in] opts FFmpeg encode options
/// @return NULL-error, other-transcode pointer
void* fftranscode_create(const AVCodecParameters* decode, const AVCodecParameters *encode, AVDictionary** opts);

int fftranscode_destroy(void* transcode);

/// transcode audio/video packet
/// @param[in] transcode create by fftranscode_create
/// @param[in] in input audio/video packet
/// @return >=0-fftranscode_getpacket, <0-need more packets
int fftranscode_input(void* transcode, const AVPacket* in);

/// get transcoded audio/video packet(MUST call many times until return code <0)
/// @param[out] out transcoded audio/video packet
/// @return >=0-get a packet(maybe have more), <0-need more input
int fftranscode_getpacket(void* transcode, AVPacket* out);

/// @return should call avcodec_parameters_free
AVCodecParameters* fftranscode_getcodecpar(void* transcode);



/// AAC transcode helper, from any other audio to AAC
/// @param[in] decode FFmpeg decode parameters
void* fftranscode_create_aac(const AVCodecParameters* decode, int sample_rate, int channel, int bitrate);

/// Opus transcode helper, from any other audio to AAC
/// @param[in] decode FFmpeg decode parameters
void* fftranscode_create_opus(const AVCodecParameters* decode, int sample_rate, int channel, int bitrate);

/// H.264 transcode helper
/// /// @param[in] decode FFmpeg decode parameters
/// @param[in] preset H.264 encdoing preset: medium
/// @param[in] profile H.264 profie: baseline, main, high
/// @param[in] tune zerolatency
void* fftranscode_create_h264(const AVCodecParameters* decode, const char* preset, const char* profile, const char* tune, int gop, int width, int height, int bitrate);

#ifdef __cplusplus
}
#endif
#endif /* !_fftranscode_h_ */
