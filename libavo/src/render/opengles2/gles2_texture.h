#ifndef _gles2_texture_h_
#define _gles2_texture_h_

static int gles2_texture_create(struct gles2_render_t* render)
{
	GLsizei i;
	glGenTextures(N_ARRAY(render->glTextures), render->glTextures);

	for (i = 0; i < N_ARRAY(render->glTextures); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, render->glTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return 0;
}

static int gles2_texture_destroy(struct gles2_render_t* render)
{
	glDeleteTextures(N_ARRAY(render->glTextures), render->glTextures);
	return 0;
}

static int gles2_texture_yv12(struct gles2_render_t* render, const struct avframe_t* frame)
{
	int i;
	int height = frame->height;
	assert(N_ARRAY(frame->data) >= N_ARRAY(render->glTextures));
	for (i = 0; i < N_ARRAY(render->glTextures); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, render->glTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 
			0,
			GL_LUMINANCE,
			frame->linesize[i],
			height,
			0,
			GL_LUMINANCE,
			GL_UNSIGNED_BYTE,
			frame->data[i]);
		
		glUniform1i(render->loc_sampler[i], i);
		height = frame->height / 2;
	}
}

#endif /* !_gles2_texture_h_ */
