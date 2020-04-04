#include <assert.h>

#if defined(OS_WINDOWS)
extern "C" int d3d9_render_register(void);
extern "C" int d3d11_render_register(void);
extern "C" int directsound8_player_register(void);
extern "C" int directsound8_recorder_register(void);
#elif defined(OS_ANDROID)
extern "C" int gles2_render_register(void);
extern "C" int opensles_player_register(void);
extern "C" int opensles_recorder_register(void);
#elif defined(OS_LINUX)
extern "C" int glx_render_register(void);
extern "C" int alsa_player_register(void);
extern "C" int alsa_recorder_register(void);
#elif defined(OS_MAC)
extern "C" int cgl_render_register(void);
extern "C" int metal_render_register(void);
extern "C" int audio_queue_player_register(void);
extern "C" int audio_queue_recorder_register(void);
#endif

static int avo_register_all()
{
#if defined(OS_WINDOWS)
	d3d11_render_register();
	d3d9_render_register();

	directsound8_player_register();
	directsound8_recorder_register();
#elif defined(OS_ANDROID)
	gles2_render_register();
	opensles_player_register();
	opensles_recorder_register();
#elif defined(OS_LINUX)
	alsa_player_register();
	alsa_recorder_register();
#elif defined(OS_MAC)
    //cgl_render_register();
    metal_render_register();
    audio_queue_player_register();
    audio_queue_recorder_register();
#endif
	return 0;
}

static int dummy = avo_register_all();

#if defined(OS_MAC)
// Just for xcode static exec avo_register_all
extern "C" int avo_register_all_dump(void)
{
    return dummy;
}
#endif
