#include "text-render.h"
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <ft2build.h> 
#include FT_FREETYPE_H
#include FT_GLYPH_H
//#include FT_STROKER_H

// https://stuff.mit.edu/afs/athena/astaff/source/src-9.0/third/freetype/docs/tutorial/step1.html
// http://freetype.sourceforge.net/freetype2/docs/tutorial/step1.html

struct text_render_t
{
	FT_Library  library;
	FT_Face     face;

	char font[256];
	int fontsize;

	void *bitmap;

	int width;
	int height;
};

static int free_type_init(struct text_render_t* s)
{
	int r;
	r = FT_Init_FreeType(&s->library);
	if (0 != r)
	{
		printf("%s Could not load FreeType: %d\n", __FUNCTION__, r);
		return r;
	}

	r = FT_New_Face(s->library, s->font, 0, &s->face);
	if (0 != r)
	{
		// FT_Err_Unknown_File_Format
		printf("%s Could not load font \"%s\": %d\n", __FUNCTION__, s->font, r);
		return r;
	}

	assert(s->face->num_faces > 0);

	r = FT_Set_Pixel_Sizes(s->face, 0, s->fontsize);
	if (0 != r)
	{
		printf("%s Could not set font size to %d pixels: %d\n", __FUNCTION__, s->fontsize, r);
		return r;
	}

	return r;
}

void* text_render_create(int width, int height)
{
	struct text_render_t* render;
	render = malloc(sizeof(struct text_render_t) + width * height * 4);
	if (NULL == render)
		return NULL;

	memset(render, 0, sizeof(struct text_render_t));
#if defined(OS_WINDOWS)
	strcpy(render->font, "C:\\WINDOWS\\Fonts\\msyh.ttc");
#else
	strcpy(render->font, "/usr/share/fonts/truetype/arial.ttf");
#endif
	render->fontsize = 32;
	render->width = width;
	render->height = height;
	render->bitmap = render + 1;

	if (0 != free_type_init(render))
	{
		text_render_destroy(render);
		return NULL;
	}
	return render;
}

void text_render_destroy(void* p)
{
	struct text_render_t* render = (struct text_render_t*)p;
	if (render->face)
	{
		FT_Done_Face(render->face);
		render->face = NULL;
	}

	if (render->library)
	{
		FT_Done_FreeType(render->library);
		render->library = NULL;
	}
}

static int text_render_bitmap(struct text_render_t* render, const FT_BitmapGlyph bmpGlyph, const FT_BBox *bbox, int x, int y)
{
	int r, c;
	//int advance = render->face->glyph->advance.x >> 6;
	//int left = render->face->glyph->bitmap_left;
	//int top = render->face->glyph->bitmap_top;
	//FT_Bitmap bitmap = render->face->glyph->bitmap;
	FT_Bitmap* bitmap = &bmpGlyph->bitmap;
	unsigned char* s;
	unsigned char* d;

	x += bmpGlyph->left;
	for (r = 0; r < bitmap->rows; r++)
	{
		s = (unsigned char*)bitmap->buffer + r * bitmap->pitch;
		d = (unsigned char*)render->bitmap + ((bitmap->rows - r + y) * render->width + x) * 4;
		for (c = 0; c < bitmap->width; c++)
		{
			*d++ = *s;	// b
			*d++ = 0;	// g
			*d++ = 0;	// r
			*d++ = 0;	// a

			++s;
		}
	}

	return 0;
}

int text_render_draw(void* p, const wchar_t* txt, int x, int y)
{
	FT_BBox bbox;
	FT_UInt glyph_index;
	FT_Bool use_kerning;
	FT_UInt previous;
	FT_Glyph glyph;
	FT_BitmapGlyph glyph_bitmap;

	int r, pen_x, pen_y;
	const wchar_t *s;
	struct text_render_t* render = (struct text_render_t*)p;

	use_kerning = FT_HAS_KERNING(render->face);
	previous = 0;
	pen_x = 0;
	pen_y = 50;
	for (s = txt; *s; s++)
	{
		/* convert character code to glyph index */ 
		glyph_index = FT_Get_Char_Index(render->face, *s);

		/* retrieve kerning distance and move pen position */
		if (use_kerning && previous && glyph_index)
		{
			FT_Vector delta;
			FT_Get_Kerning(render->face, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
			pen_x += delta.x >> 6; // 1/64
		}

		/* load glyph image into the slot (erase previous one) */
		r = FT_Load_Glyph(render->face, glyph_index, FT_LOAD_DEFAULT);
		if (r)
		{
			printf("%s load char(%x) => %d\n", __FUNCTION__, (unsigned int)*s, r);
			continue; /* ignore errors */
		}

		/* load glyph into s->face->glyph */
		// FT_Load_Char(FT_LOAD_RENDER) == FT_Get_Char_Index + FT_Load_Glyph(FT_LOAD_DEFAULT) + FT_Render_Glyph(FT_RENDER_MODE_NORMAL)
		//r = FT_Load_Char(render->face, *s, FT_LOAD_RENDER);
		//if (0 != r)
		//{
		//	printf("%s load char(%x) => %d\n", __FUNCTION__, (unsigned int)*s, r);
		//	return r;
		//}

		r = FT_Get_Glyph(render->face->glyph, &glyph);
		if (r)
		{
			printf("%s FT_Get_Glyph(%x) => %d\n", __FUNCTION__, (unsigned int)*s, r);
			continue; /* ignore errors */
		}

		FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_pixels, &bbox);

		if (glyph->format != FT_GLYPH_FORMAT_BITMAP)
		{
			r = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, 0);
			if (r)
			{
				printf("%s FT_Glyph_To_Bitmap(%x) => %d\n", __FUNCTION__, (unsigned int)*s, r);
				continue; /* ignore errors */
			}
		}

		glyph_bitmap = (FT_BitmapGlyph)glyph;
		text_render_bitmap(render, (FT_BitmapGlyph)glyph, &bbox, pen_x, pen_y);

		printf("x/y: %d/%d, box: %d/%d/%d/%d, left/top: %d/%d, bitmap row/width/pitch: %d/%d/%d\n",
			pen_x, pen_y, 
			bbox.xMin, bbox.yMin, bbox.xMax, bbox.yMax,
			glyph_bitmap->left, glyph_bitmap->top,
			glyph_bitmap->bitmap.rows, glyph_bitmap->bitmap.width, glyph_bitmap->bitmap.pitch);

		FT_Done_Glyph(glyph);

		/* increment pen position */
		pen_x += render->face->glyph->advance.x >> 6; // 1/64

		/* record current glyph index */
		previous = glyph_index;
	}

	return 0;
}

const void* text_render_getimage(void* p)
{
	struct text_render_t* render = (struct text_render_t*)p;
	return render->bitmap;
}

int text_render_config(void* p, struct text_parameter_t* param)
{
	int r;
	struct text_render_t* render = (struct text_render_t*)p;
	r = FT_Set_Pixel_Sizes(render->face, 0, render->fontsize);
	if (0 != r)
	{
		printf("%s Could not set font size to %d pixels: %d\n", __FUNCTION__, render->fontsize, r);
		return r;
	}

	return 0;
}
