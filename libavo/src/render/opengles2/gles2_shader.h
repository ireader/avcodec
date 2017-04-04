#ifndef _gles2_shader_h_
#define _gles2_shader_h_

#include "../opengl_shader.h"

const static char s_vertex_shader[] =
	"precision highp float;" \
	"attribute vec4 v_position;" \
	"attribute vec2 v_texture;" \
	"uniform   mat4 v_mvpMatrix;" \
	"uniform   mat4 v_texMatrix;" \
	"varying   vec2 f_texture;" \
	"void main() {" \
	"    gl_Position = v_mvpMatrix * v_position;" \
	"    f_texture = (v_texMatrix * vec4(v_texture, 0.0, 1.0)).xy;" \
	"}";

const static char s_pixel_shader[] =
	"precision highp float;" \
	"varying   highp vec2 f_texture;" \
	"uniform         mat3 v_ColorConversion;" \
	"uniform   lowp  sampler2D y_sampler;" \
	"uniform   lowp  sampler2D u_sampler;" \
	"uniform   lowp  sampler2D v_sampler;" \
	"void main()" \
	"{" \
		"mediump vec3 yuv;" \
		"lowp    vec3 rgb;" \
		"yuv.x = (texture2D(y_sampler, f_texture).r - (16.0 / 255.0));" \
		"yuv.y = (texture2D(u_sampler, f_texture).r - 0.5);" \
		"yuv.z = (texture2D(v_sampler, f_texture).r - 0.5);" \
		"rgb = v_ColorConversion * yuv;" \
		"gl_FragColor = vec4(rgb, 1);" \
	"}";

#endif /* !_gles2_shader_h_ */
