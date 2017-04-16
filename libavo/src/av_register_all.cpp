#include <assert.h>

#if defined(OS_WINDOWS)
extern "C" int d3d9_render_register();
extern "C" int d3d11_render_register();
extern "C" int directsound8_player_register();
extern "C" int directsound8_capture_register();
#elif defined(OS_ANDROID)
extern "C" int gles2_render_register();
extern "C" int opensles_player_register();
#elif defined(OS_LINUX)
extern "C" int alsa_player_register();
extern "C" int alsa_capture_register();
#endif

static int av_register_all()
{
#if defined(OS_WINDOWS)
	d3d11_render_register();
	d3d9_render_register();

	directsound8_player_register();
	directsound8_capture_register();
#elif defined(OS_ANDROID)
	gles2_render_register();
	opensles_player_register();
#elif defined(OS_LINUX)
	alsa_player_register();
	alsa_capture_register();
#endif
	return 0;
}

static int dummy = av_register_all();
