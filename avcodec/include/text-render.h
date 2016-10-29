#ifndef _text_render_h_
#define _text_render_h_

#include <stddef.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif
	
struct text_parameter_t
{
	const char* font;
	int size; // font size
};

void* text_render_create(const struct text_parameter_t* param);

void text_render_destroy(void* render);

/// @param[in] txt unicode string
/// @param[out] w bitmap width
/// @param[out] h bitmap height
/// @param[out] pitch bitmap line bytes(>=w)
/// @return NULL-failed, other-bitmap data
const void* text_render_draw(void* render, const wchar_t* txt, int *w, int *h, int* pitch);

int text_render_config(void* render, const struct text_parameter_t* param);

#ifdef __cplusplus
}
#endif
#endif /* !_text_render_h_ */
