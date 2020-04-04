#ifndef _opengl_texture_h_
#define _opengl_texture_h_

#include <assert.h>

struct opengl_texture_t
{
    int n; // plane count
    GLuint textures[3];// yuv
    
    GLint loc_sampler[3];
};

static int opengl_texture_create(struct opengl_texture_t* render, int n)
{
    GLsizei i;
    assert(n <= sizeof(render->textures)/sizeof(render->textures[0]));
    glGenTextures(n, render->textures);

    for (i = 0; i < n; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, render->textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    render->n = n;
    return 0;
}

static int opengl_texture_destroy(struct opengl_texture_t* render)
{
    assert(render->n <= sizeof(render->textures)/sizeof(render->textures[0]));
    glDeleteTextures(render->n, render->textures);
    return 0;
}

static int opengl_texture_yv12(struct opengl_texture_t* render, const struct avframe_t* frame)
{
    int i;
    int height;
    assert(render->n >= 3);
    
    height = frame->height;
    for (i = 0; i < 3; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, render->textures[i]);
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_RED, //GL_LUMINANCE,
            frame->linesize[i],
            height,
            0,
            GL_RED, //GL_LUMINANCE,
            GL_UNSIGNED_BYTE,
            frame->data[i]);

        glUniform1i(render->loc_sampler[i], i);
        height = frame->height / 2;
    }
    
    return 0;
}

#endif /* _opengl_texture_h_ */
