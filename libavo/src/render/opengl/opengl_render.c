#if defined(OS_IOS) || defined(OS_ANDROID)
#include  <GLES2/gl2.h>
#elif defined(OS_MAC)
#include <OpenGL/gl3.h>
#endif

#include "../yuv_color.h"
#include "opengl_render.h"
#include "opengl_matrix.h"
#include "opengl_shader.h"
#include "opengl_vertex.h"
#include "opengl_texture.h"
#include <stdio.h>
#include <stdlib.h>

struct opengl_render_t
{
    struct opengl_shader_t shader;
    struct opengl_vertex_t vertex;// buffer
    struct opengl_texture_t texture;

    // shader location
    GLint loc_mvpMatrix; // model-view-project matrix
    GLint loc_texMatrix; // texture matrix
    GLint loc_color;
};

const static GLfloat s_vertex[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
};

const static GLushort s_indices[] = {
    0, 1, 2, 3,
};

static void opengl_check_error(const char* api)
{
    GLenum err;
    for(err = glGetError(); GL_NO_ERROR != err; err = glGetError())
    {
        printf("opengl %s error: %d\n", api, (int)err);
    }
}

static int opengl_matrix_mvp(struct opengl_render_t* render, int x, int y, int w, int h)
{
    GLfloat matrix[16];
    opengl_matrix_identity(matrix);
    glUniformMatrix4fv(render->loc_mvpMatrix, 1, GL_FALSE, matrix);
    return 0;
}

static int opengl_matrix_tex(struct opengl_render_t* render, int x, int y, int w, int h)
{
    GLfloat matrix[16];
    opengl_matrix_identity(matrix);
    glUniformMatrix4fv(render->loc_texMatrix, 1, GL_FALSE, matrix);
    return 0;
}

static int opengl_get_location(struct opengl_render_t* render)
{
    render->vertex.loc_position = glGetAttribLocation(render->shader.program, "v_position");
    render->vertex.loc_texture = glGetAttribLocation(render->shader.program, "v_texture");
    render->loc_mvpMatrix = glGetUniformLocation(render->shader.program, "v_mvpMatrix");
    render->loc_texMatrix = glGetUniformLocation(render->shader.program, "v_texMatrix");
    render->loc_color = glGetUniformLocation(render->shader.program, "v_ColorConversion");

    render->texture.loc_sampler[0] = glGetUniformLocation(render->shader.program, "y_sampler");
    render->texture.loc_sampler[1] = glGetUniformLocation(render->shader.program, "u_sampler");
    render->texture.loc_sampler[2] = glGetUniformLocation(render->shader.program, "v_sampler");

    return 0;
}

struct opengl_render_t* opengl_render_open(const char* vertex_shader, const char* fragment_shader)
{
    struct opengl_render_t* render;
    render = (struct opengl_render_t*)calloc(1, sizeof(*render));
    if(!render)
        return NULL;

    if (0 != opengl_shader_create(&render->shader, vertex_shader, fragment_shader)
        || 0 != opengl_get_location(render)
        || 0 != opengl_texture_create(&render->texture, 3)
        || 0 != opengl_vertex_create(&render->vertex, s_vertex, sizeof(s_vertex)/sizeof(s_vertex[0]), s_indices, sizeof(s_indices)/sizeof(s_indices[0])))
    {
        opengl_check_error("opengl_render_open");
        opengl_render_close(render);
        return NULL;
    }

    //glViewport(0, 0, width, height);
    return render;
}

int opengl_render_close(struct opengl_render_t* render)
{
    if(render)
    {
        opengl_vertex_destroy(&render->vertex);
        opengl_texture_destroy(&render->texture);
        opengl_shader_destroy(&render->shader);
        free(render);
    }
    return 0;
}

int opengl_render_write(struct opengl_render_t* render, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(render->shader.program);
    //glEnable(GL_CULL_FACE);
    
//    opengl_matrix_mvp(render, tgt_x, tgt_y, tgt_w, tgt_h);
//    opengl_matrix_tex(render, src_x, src_y, src_w, src_h);
    glUniformMatrix3fv(render->loc_color, 1, GL_FALSE, s_bt709);
    
    opengl_vertex_bind(&render->vertex);
    opengl_texture_yv12(&render->texture, pic);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->vertex.buffers[1]);
    glDrawElements(GL_TRIANGLE_STRIP, sizeof(s_indices)/sizeof(s_indices[0]), GL_UNSIGNED_SHORT, (const void*)0);
    
    glFlush();

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //opengl_buffer_unbind(render);

    opengl_check_error("opengl_render_write");
    return 0;
}
