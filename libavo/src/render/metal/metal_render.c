#include "av_register.h"
#include "video_output.h"
#include "metal_render.h"
#include <stdlib.h>
#include <string.h>

struct metal_render_t
{
    struct metal_render_handler_t handler;
};

static int metal_close(void* vo)
{
    struct metal_render_t* render;
    render = (struct metal_render_t*)vo;
    render->handler.close(render->handler.metal);
    free(render);
    return 0;
}

static void* metal_open(void* window, int format, int width, int height)
{
    struct metal_render_t* render;
    render = (struct metal_render_t*)calloc(1, sizeof(*render));
    if (NULL == render)
        return NULL;

    memcpy(&render->handler, window, sizeof(render->handler));
    if(0 != render->handler.open(render->handler.metal, format, width, height))
    {
        metal_close(render);
        return NULL;
    }
    
    return render;
}

static int metal_write(void* vo, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h)
{
    struct metal_render_t* render;
    render = (struct metal_render_t*)vo;
    return render->handler.present(render->handler.metal, pic, src_x, src_y, src_w, src_h, tgt_x, tgt_y, tgt_w, tgt_h);
}

int metal_render_register(void)
{
    static video_output_t vo;
    vo.open = metal_open;
    vo.close = metal_close;
    vo.write = metal_write;
    vo.read = NULL;
    vo.control = NULL;
    vo.rotation = NULL;
    return av_set_class(AV_VIDEO_RENDER, "metal", &vo);
}
