#ifndef _gles2_buffer_h_
#define _gles2_buffer_h_

/// vertex: x/y/z + u/v per vertex
static int gles2_buffer_create(struct gles2_render_t* render, const GLfloat vertex[], GLsizei n, const GLushort indices[], GLsizei m)
{
	glGenBuffers(N_ARRAY(render->glBuffers), render->glBuffers);

	glBindBuffer(GL_ARRAY_BUFFER, render->glBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(GLfloat), vertex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->glBuffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m * sizeof(GLushort), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return 0;
}

static int gles2_buffer_destroy(struct gles2_render_t* render)
{
	glDeleteBuffers(N_ARRAY(render->glBuffers), render->glBuffers);
	return 0;
}

static int gles2_buffer_bind(struct gles2_render_t* render)
{
	glBindBuffer(GL_ARRAY_BUFFER, render->glBuffers[0]);
	glVertexAttribPointer(render->loc_position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0);
	glEnableVertexAttribArray(render->loc_position);
	glVertexAttribPointer(render->loc_texture, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(render->loc_texture);
	return 0;
}

static int gles2_buffer_unbind(struct gles2_render_t* render)
{
	glDisableVertexAttribArray(render->loc_position);
	glDisableVertexAttribArray(render->loc_texture);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return 0;
}

#endif /* !_gles2_buffer_h_ */
