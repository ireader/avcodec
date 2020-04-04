#ifndef _opengl_context_h_
#define _opengl_context_h_

#ifdef __cplusplus
extern "C"{
#endif

struct opengl_context_handler_t
{
    int (*create)(void* ctx, void* window);
    int (*destroy)(void* ctx);
    
    int (*draw)(void* ctx);
    int (*flush)(void* ctx);
};

void* cgl_context(struct opengl_context_handler_t* ctx);
void* egl_context(struct opengl_context_handler_t* ctx);
void* glx_context(struct opengl_context_handler_t* ctx);

#ifdef __cplusplus
}
#endif
#endif /* _opengl_context_h_ */
