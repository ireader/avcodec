#ifndef _opengl_buffer_h_
#define _opengl_buffer_h_

struct opengl_vertex_t
{
    GLuint buffers[2]; // buffer
    
    // shader location
    GLint loc_position;
    GLint loc_texture;
};

/// vertex: x/y/z + u/v per vertex
static int opengl_vertex_create(struct opengl_vertex_t* render, const GLfloat vertex[], GLsizei n, const GLushort indices[], GLsizei m)
{
    glGenBuffers(sizeof(render->buffers)/sizeof(render->buffers[0]), render->buffers);

    glBindBuffer(GL_ARRAY_BUFFER, render->buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, n * sizeof(GLfloat), vertex, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->buffers[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m * sizeof(GLushort), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return 0;
}

static int opengl_vertex_destroy(struct opengl_vertex_t* render)
{
    glDeleteBuffers(sizeof(render->buffers)/sizeof(render->buffers[0]), render->buffers);
    return 0;
}

static int opengl_vertex_bind(struct opengl_vertex_t* render)
{
    glBindBuffer(GL_ARRAY_BUFFER, render->buffers[0]);
    
    glVertexAttribPointer(render->loc_position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0);
    glEnableVertexAttribArray(render->loc_position);
    glVertexAttribPointer(render->loc_texture, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(render->loc_texture);
    return 0;
}

static int opengl_vertex_unbind(struct opengl_vertex_t* render)
{
    glDisableVertexAttribArray(render->loc_position);
    glDisableVertexAttribArray(render->loc_texture);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return 0;
}

#endif /* _opengl_buffer_h_ */
