#ifndef _opengl_render_h_
#define _opengl_render_h_

#include "avframe.h"

struct opengl_render_t;

struct opengl_render_t* opengl_render_open(const char* vertex_shader, const char* fragment_shader);
int opengl_render_close(struct opengl_render_t* render);

int opengl_render_write(struct opengl_render_t* render, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h);

#endif /* _opengl_render_h_ */
