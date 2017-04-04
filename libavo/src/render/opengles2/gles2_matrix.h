#ifndef _gles2_matrix_h_
#define _gles2_matrix_h_

#include "../opengl_matrix.h"

static int gles2_matrix_mvp(struct gles2_render_t* render, int x, int y, int w, int h)
{
	GLfloat matrix[16];
	opengl_matrix_identity(matrix);
	glUniformMatrix4fv(render->loc_mvpMatrix, 1, GL_FALSE, matrix);
	return 0;
}

static int gles2_matrix_tex(struct gles2_render_t* render, int x, int y, int w, int h)
{
	GLfloat matrix[16];
	opengl_matrix_identity(matrix);
	glUniformMatrix4fv(render->loc_texMatrix, 1, GL_FALSE, matrix);
	return 0;
}

#endif /* !_gles2_matrix_h_ */
