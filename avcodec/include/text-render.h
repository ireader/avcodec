#ifndef _text_render_h_
#define _text_render_h_

#include <stddef.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif
	
struct text_parameter_t
{
	char font[128];
	int size; // font size
};

void* text_render_create(int width, int height);

void text_render_destroy(void* render);

/// @param[in] txt unicode string
int text_render_draw(void* render, const wchar_t* txt, int x, int y);

const void* text_render_getimage(void* render);

int text_render_config(void* render, struct text_parameter_t* param);

#ifdef __cplusplus
}
#endif
#endif /* !_text_render_h_ */
