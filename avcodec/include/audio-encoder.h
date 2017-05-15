#ifndef _audio_encoder_h_
#define _audio_encoder_h_

#include "avframe.h"
#include "avpacket.h"

#ifdef __cplusplus
extern "C" {
#endif

struct audio_parameter_t
{
	int format; // enum pcm_sample_format
	int channels; // 1, 2
	int frequency; // 8000, 16000, 24000, 32000, 44100, 48000

	int profile;
	int level;
};

struct audio_encoder_t
{
	void* (*create)(const struct audio_parameter_t* param);
	void (*destroy)(void* audio);

	/// pic->flags & AVPACKET_FLAG_KEY => force IDR
	/// @return >0-ok, other-error
	int (*encode)(void* audio, const struct avframe_t* pic);

	/// @return >=0-got packet, <0-error
	int (*getpacket)(void* audio, struct avpacket_t* pkt);
};

struct audio_encoder_t* opus_encoder();

#ifdef __cplusplus
}
#endif
#endif /* !_audio_encoder_h_ */
