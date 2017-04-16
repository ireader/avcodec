#ifndef _gles2_render_h_
#define _gles2_render_h_

#include <GLES2/gl2.h>
#include "gles2_egl.h"
#include "gles2_shader.h"
#include <pthread.h>

struct gles2_render_t
{
	struct gles2_egl_t egl;
	struct opengl_shader_t shader;

	void* window;
	int window_width;
	int window_height;
	pthread_t thread; // thread id

	GLuint glTextures[3];// yuv
	GLuint glBuffers[2];// buffer

	// shader location
	GLint loc_position;
	GLint loc_texture;
	GLint loc_mvpMatrix; // model-view-project matrix
	GLint loc_texMatrix; // texture matrix
	GLint loc_color;
	GLint loc_sampler[3];
};

#define N_ARRAY(arr) (sizeof(arr)/sizeof(arr[0]))

static void gles2_check_error(const char* api);

#endif /* !_gles2_render_h_ */
