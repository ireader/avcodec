#include "av_register.h"
#include "video_output.h"
#include "gles2_render.h"
#include "gles2_buffer.h"
#include "gles2_matrix.h"
#include "gles2_texture.h"
#include "../yuv_color.h"
#include <android/log.h>
#include <string.h>

const static GLfloat s_vertex[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
};

const static GLushort s_indices[] = {
	0, 1, 2, 3,
};

static void gles2_check_error(const char* api)
{
	GLenum err;
	for(err = glGetError(); GL_NO_ERROR != err; err = glGetError())
	{
		__android_log_print(ANDROID_LOG_ERROR, "gles2", "%s error: %d\n", api, (int)err);
	}
}

static int gles2_get_location(struct gles2_render_t* render)
{
	render->loc_position = glGetAttribLocation(render->shader.program, "v_position");
	render->loc_texture = glGetAttribLocation(render->shader.program, "v_texture");
	render->loc_mvpMatrix = glGetUniformLocation(render->shader.program, "v_mvpMatrix");
	render->loc_texMatrix = glGetUniformLocation(render->shader.program, "v_texMatrix");

	render->loc_color = glGetUniformLocation(render->shader.program, "v_ColorConversion");

	render->loc_sampler[0] = glGetUniformLocation(render->shader.program, "y_sampler");
	render->loc_sampler[1] = glGetUniformLocation(render->shader.program, "u_sampler");
	render->loc_sampler[2] = glGetUniformLocation(render->shader.program, "v_sampler");

	return 0;
}

static int gles2_close(void* vo)
{
	struct gles2_render_t* render = (struct gles2_render_t*)vo;
	gles2_buffer_destroy(render);
	gles2_texture_destroy(render);
	opengl_shader_destroy(&render->shader);
	gles2_egl_destroy(&render->egl);
	free(render);
	return 0;
}

static void* gles2_open(void* window, int format, int width, int height)
{
	struct gles2_render_t* render;
	render = (struct gles2_render_t*)malloc(sizeof(*render));
	if (NULL == render)
		return NULL;
	memset(render, 0, sizeof(struct gles2_render_t));
	render->window = window;
	render->window_width = width;
	render->window_height = height;
	render->thread = pthread_self();

	if ((NULL != window && (EGL_SUCCESS != gles2_egl_create(&render->egl) || EGL_SUCCESS != gles2_egl_bind(&render->egl, window)))
		|| 0 != opengl_shader_create(&render->shader, s_vertex_shader, s_pixel_shader)
		|| 0 != gles2_get_location(render)
		|| 0 != gles2_texture_create(render)
		|| 0 != gles2_buffer_create(render, s_vertex, N_ARRAY(s_vertex), s_indices, N_ARRAY(s_indices)))
	{
		gles2_check_error("gles2_open");
		gles2_close(render);
		return NULL;
	}

	//glViewport(0, 0, width, height);
	return render;
}

static int gles2_write(void* vo, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h)
{
	struct gles2_render_t* render = (struct gles2_render_t*)vo;

	if (0 == pthread_equal(render->thread, pthread_self()))
	{
		__android_log_print(ANDROID_LOG_ERROR, "gles2", "video open/write in different thread.\n");
		return -1;
	}

	if (NULL != render->window)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glUseProgram(render->shader.program);
	//glEnable(GL_CULL_FACE);

	gles2_matrix_mvp(render, tgt_x, tgt_y, tgt_w, tgt_h);
	gles2_matrix_tex(render, src_x, src_y, src_w, src_h);
	glUniformMatrix3fv(render->loc_color, 1, GL_FALSE, s_bt709);

	gles2_buffer_bind(render);
	gles2_texture_yv12(render, pic);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->glBuffers[1]);
	glDrawElements(GL_TRIANGLE_STRIP, N_ARRAY(s_indices), GL_UNSIGNED_SHORT, (const void*)0);

	if(NULL != render->window)
		gles2_egl_present(&render->egl);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//gles2_buffer_unbind(render);

	gles2_check_error("gles2_write");
	return 0;
}

int gles2_render_register()
{
	static video_output_t vo;
	vo.open = gles2_open;
	vo.close = gles2_close;
	vo.write = gles2_write;
	vo.read = NULL;
	vo.control = NULL;
	vo.rotation = NULL;
	return av_set_class(AV_VIDEO_RENDER, "gles2", &vo);
}
