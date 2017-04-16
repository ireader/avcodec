#ifndef _av_register_h_
#define _av_register_h_

#ifdef __cplusplus
extern "C" {
#endif

enum {
	AV_AUDIO_CAPTURE = 0,
	AV_AUDIO_OUTPUT,
	AV_VIDEO_CAPTURE,
	AV_VIDEO_RENDER,
};

void av_list(int avtype, void (*item)(void* param, const char*), void* param);

int av_set_name(int avtype, const char* name);
const char* av_get_name(int avtype);

const void* av_get_class(int avtype);
int av_set_class(int avtype, const char* name, const void* cls);

#ifdef __cplusplus
}
#endif
#endif /* !_av_register_h_ */
