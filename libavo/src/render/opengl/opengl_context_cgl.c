#include "av_register.h"
#include "video_output.h"
#include <OpenGL/gl3.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/gltypes.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "opengl_render.h"

#define kCGLCPSurfaceWindowNumber 504

struct cgl_context_t
{
    CGLContextObj context;
    
    void* window;
    int window_width;
    int window_height;
    pthread_t thread; // thread id
    
    struct opengl_render_t* render;
};

const static char s_vertex_shader[] =
    "attribute vec4 v_position;" \
    "attribute vec2 v_texture;" \
    "varying   vec2 f_texture;" \
    "void main() {" \
    "    gl_Position = v_position;" \
    "    f_texture = v_texture;" \
    "}";

const static char s_fragment_shader[] =
    "varying vec2 f_texture;" \
    "uniform mat3 v_ColorConversion;" \
    "uniform sampler2D y_sampler;" \
    "uniform sampler2D u_sampler;" \
    "uniform sampler2D v_sampler;" \
    "void main()" \
    "{" \
        "vec3 yuv;" \
        "vec3 rgb;" \
        "yuv.x = (texture2D(y_sampler, f_texture).r - (16.0 / 255.0));" \
        "yuv.y = (texture2D(u_sampler, f_texture).r - 0.5);" \
        "yuv.z = (texture2D(v_sampler, f_texture).r - 0.5);" \
        "rgb = v_ColorConversion * yuv;" \
        "gl_FragColor = vec4(rgb, 1);" \
    "}";


static int cgl_create(struct cgl_context_t* cgl, void* window)
{
    GLint npix;
    CGLError err;
    CGLPixelFormatObj pixel;
//    CGDisplayCount maxDisplays = 32;
//    CGDirectDisplayID activeDspys[32];
//    CGDisplayCount newDspyCnt = 0;
//    CGLRendererInfoObj info;
    
    //GLint num[1]={[[_view window] windowNumber]};

//    CGLPixelFormatAttribute attribs[] = {
//        //kCGLPFADisplayMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay), /* Display */
//        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
//        kCGLPFAColorSize, 24,
//        kCGLPFAAlphaSize, 8,
//        kCGLPFADoubleBuffer,
//        kCGLPFASampleBuffers, 1,
//        kCGLPFASamples, 2,
//        kCGLPFAAccelerated,
//        kCGLPFAAcceleratedCompute,
//        kCGLPFAAllRenderers,
//        0
//    };

    //CGGetActiveDisplayList(maxDisplays, activeDspys, &newDspyCnt);
    //CGLQueryRendererInfo (aDisplayCaps[i].cglDisplayMask, &info, &numRenderers);
    
    cgl->context = (CGLContextObj)window;
    pixel = CGLGetPixelFormat(cgl->context);
    CGLDescribePixelFormat(pixel, 0, kCGLPFADisplayMask, &npix);
    CGLDescribePixelFormat(pixel, 0, kCGLPFAOpenGLProfile, &npix); // 4096
    CGLDescribePixelFormat(pixel, 0, kCGLPFAColorSize, &npix); // 32
    CGLDescribePixelFormat(pixel, 0, kCGLPFAAlphaSize, &npix); // 8
    CGLDescribePixelFormat(pixel, 0, kCGLPFADoubleBuffer, &npix); // 0
    CGLDescribePixelFormat(pixel, 0, kCGLPFASampleBuffers, &npix); // 0
    CGLDescribePixelFormat(pixel, 0, kCGLPFASamples, &npix); // 0
    CGLDescribePixelFormat(pixel, 0, kCGLPFAAccelerated, &npix); // 1
    CGLDescribePixelFormat(pixel, 0, kCGLPFAAllRenderers, &npix);
    

//    /* Context init. */
//    err = CGLChoosePixelFormat(attribs, &pixel, &npix);
//    //err = CGLCreateContext(pixel, NULL, &cgl->context);
//    err = CGLCreateContext(pixel, (CGLContextObj)window, &cgl->context);
//    CGLReleasePixelFormat(pixel);
    //cgl->context = (CGLContextObj)window;
    
    //err = CGLLockContext(cgl->context);
    //err = CGLSetParameter(cgl->context, kCGLCPSurfaceWindowNumber, num);
    //[self update];
    //err = CGLUnlockContext(cgl->context);
    
    err = CGLSetCurrentContext(cgl->context);
    printf("OpenGL Render: %s; Version:%s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    
    return kCGLNoError == err ? 0 : -1;
}

static int cgl_destroy(struct cgl_context_t* cgl)
{
    //CGLReleaseContext(cgl->context);
    
    return 0;
}

static int cgl_draw(struct cgl_context_t* cgl)
{
    CGLError err;
    //err = CGLClearDrawable(cgl->context);
    err = CGLSetCurrentContext(cgl->context);
    err = CGLLockContext(cgl->context); // This is needed because this isn't running on the main thread.

    /* FIXME: Deprecated, find another way to grab screen. */
    /* WTF: Can't use previously assigned display_mask... */
    //err = CGLSetFullScreenOnDisplay(cgl->context, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay));
    
    return kCGLNoError == err ? 0 : -1;
}

static int cgl_present(struct cgl_context_t* cgl)
{
    CGLError err;
//    GLint sync = 1;
//    GLint order = -1; // below window
//    GLint opaque = 0;
//    GLint fragmentGPUProcessing, vertexGPUProcessing;
//    GLint dim[2] = {720, 480};
    
    err = kCGLNoError;
//    // Using CGL to check whether the GPU is processing vertices and fragments
//    err = CGLGetParameter (CGLGetCurrentContext(), kCGLCPGPUFragmentProcessing, &fragmentGPUProcessing);
//    err = CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUVertexProcessing, &vertexGPUProcessing);
//
//    // Controlling the Back Buffer Size
//    err = CGLSetParameter(cgl->context, kCGLCPSurfaceBackingSize, dim);
//    err = CGLEnable (cgl->context, kCGLCESurfaceBackingSize);
//
//    // Surface Drawing Order Specifies the Position of the OpenGL Surface Relative to the Window
//    err = CGLSetParameter (cgl->context, kCGLCPSurfaceOrder, &order);
//
//    // Surface Opacity Specifies How the OpenGL Surface Blends with Surfaces Behind It
//    err = CGLSetParameter (cgl->context, kCGLCPSurfaceOpacity, &opaque);
//
//    // Swap Interval Allows an Application to Synchronize Updates to the Screen Refresh
//    err = CGLSetParameter (cgl->context, kCGLCPSwapInterval, &sync);
//
    err = CGLUnlockContext(cgl->context);
    //err = CGLUpdateContext(cgl->context);
    err = CGLFlushDrawable(cgl->context);
    return kCGLNoError == err ? 0 : -1;
}

static int cgl_close(void* vo)
{
    struct cgl_context_t* cgl;
    cgl = (struct cgl_context_t*)vo;
    opengl_render_close(cgl->render);
    free(cgl);
    return 0;
}

static void* cgl_open(void* window, int format, int width, int height)
{
    struct cgl_context_t* cgl;
    cgl = (struct cgl_context_t*)calloc(1, sizeof(*cgl));
    if (NULL == cgl)
        return NULL;

    cgl->window = window;
    cgl->window_width = width;
    cgl->window_height = height;
    cgl->thread = pthread_self();

    if(!window || 0 != cgl_create(cgl, window))
    {
        cgl_close(cgl);
        return NULL;
    }
    
    cgl->render = opengl_render_open(s_vertex_shader, s_fragment_shader);
    if(!cgl->render)
    {
        cgl_close(cgl);
        return NULL;
    }
    
    return cgl;
}

static int cgl_write(void* vo, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h)
{
    struct cgl_context_t* cgl;
    cgl = (struct cgl_context_t*)vo;
    
    if (0 == pthread_equal(cgl->thread, pthread_self()))
    {
        printf("cgl video open/write in different thread.\n");
        return -1;
    }
    
    if(NULL == cgl->window || 0 != cgl_draw(cgl))
    {
        printf("opengl prepare draw error\n");
        return -1;
    }
    
    opengl_render_write(cgl->render, pic, src_x, src_y, src_w, src_h, tgt_x, tgt_y, tgt_w, tgt_h);
    
    return cgl_present(cgl);
}

int cgl_render_register(void)
{
    static video_output_t vo;
    //printf("OpenGL Render: %s; Version:%s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    
    vo.open = cgl_open;
    vo.close = cgl_close;
    vo.write = cgl_write;
    vo.read = NULL;
    vo.control = NULL;
    vo.rotation = NULL;
    return av_set_class(AV_VIDEO_RENDER, "opengl-cgl", &vo);
}
