#ifndef metal_render_h
#define metal_render_h

#ifdef __cplusplus
extern "C"{
#endif

struct metal_render_handler_t
{
    void* metal;
    
    int (*open)(void* metal, int format, int width, int height);
    int (*present)(void* metal, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h);
    int (*close)(void* metal);
};

#ifdef __cplusplus
}
#endif
#endif /* metal_render_h */
