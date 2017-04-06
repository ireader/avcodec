#ifndef _gles2_egl_h_
#define _gles2_egl_h_

#include <EGL/egl.h>
#include <assert.h>

struct gles2_egl_t
{
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLConfig config;
};

static EGLContext gles2_egl_context(struct gles2_egl_t* egl)
{
	EGLint numConfig;
	const EGLint configAttribs[] = {
#if defined(EGL_OPENGL_ES2_BIT)
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#endif
		EGL_SURFACE_TYPE,	EGL_WINDOW_BIT,
		EGL_RED_SIZE,		8,
		EGL_GREEN_SIZE,		8,
		EGL_BLUE_SIZE,		8,
		//EGL_ALPHA_SIZE,	0,
		//EGL_DEPTH_SIZE,	16,
		//EGL_STENCIL_SIZE,	0,
		EGL_NONE
	};

	const EGLint contextAttribs[] = {
#if defined(EGL_CONTEXT_CLIENT_VERSION)
		EGL_CONTEXT_CLIENT_VERSION, 2, // openGL ES 2.0
#endif
		EGL_NONE
	};

	if (EGL_TRUE == eglChooseConfig(egl->display, configAttribs, &egl->config, 1, &numConfig))
	{
		assert(1 == numConfig);
		egl->context = eglCreateContext(egl->display, egl->config, EGL_NO_CONTEXT, contextAttribs);
	}
	return egl->context;
}

static int gles2_egl_create(struct gles2_egl_t* egl)
{
	EGLint major, minor;
	egl->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (EGL_NO_DISPLAY == egl->display 
		|| EGL_TRUE != eglInitialize(egl->display, &major, &minor)
		|| EGL_NO_CONTEXT == gles2_egl_context(egl))
	{
		return eglGetError();
	}
	return EGL_SUCCESS;
}

static int gles2_egl_destroy(struct gles2_egl_t* egl)
{
	if (EGL_NO_DISPLAY != egl->display)
	{
		if (EGL_NO_SURFACE != egl->surface)
		{
			eglDestroySurface(egl->display, egl->surface);
			egl->surface = EGL_NO_SURFACE;
		}

		if (EGL_NO_CONTEXT != egl->context)
		{
			eglDestroyContext(egl->display, egl->context);
			egl->context = EGL_NO_CONTEXT;
		}

		eglTerminate(egl->display);
		egl->display = EGL_NO_DISPLAY;
	}

	//eglReleaseThread();
	return EGL_SUCCESS;
}

static int gles2_egl_bind(struct gles2_egl_t* egl, EGLNativeWindowType window)
{
	assert(EGL_NO_DISPLAY != egl->display && 0 != egl->config);
	egl->surface = eglCreateWindowSurface(egl->display, egl->config, window, NULL);
	if (EGL_NO_SURFACE != egl->surface)
		eglMakeCurrent(egl->display, egl->surface, egl->surface, egl->context);

	return eglGetError();
}

static int gles2_egl_unbind(struct gles2_egl_t* egl)
{
	eglMakeCurrent(egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	if (EGL_NO_SURFACE != egl->surface && EGL_NO_DISPLAY != egl->display)
	{
		eglDestroySurface(egl->display, egl->surface);
		egl->surface = EGL_NO_SURFACE;
	}

	return eglGetError();
}

/// @return EGL_SUCCESS-ok, EGL_CONTEXT_LOST-context lost
static int gles2_egl_present(struct gles2_egl_t* egl)
{
	if (EGL_TRUE != eglSwapBuffers(egl->display, egl->surface))
	{
		return eglGetError(); // EGL_CONTEXT_LOST
	}
	return EGL_SUCCESS;
}

#endif /* !_gles2_egl_h_ */
