#include "video_output.h"
#include "av_register.h"
#include <stdlib.h>
#include <string.h>

struct vo_context_t
{
	const video_output_t* vo;
	void* id;
};

void* video_output_open(void* window, int format, int width, int height)
{
	struct vo_context_t* h;
	h = (struct vo_context_t*)malloc(sizeof(struct vo_context_t));
	if (NULL == h)
		return NULL;
	
	h->vo = (const video_output_t*)av_get_class(AV_VIDEO_RENDER);
	if (NULL == h->vo)
	{
		video_output_close(h);
		return NULL;
	}

	h->id = h->vo->open(window, format, width, height);
	if (NULL == h->id)
	{
		video_output_close(h);
		return NULL;
	}

	return h;
}

int video_output_close(void* vo)
{
	struct vo_context_t* h = (struct vo_context_t*)vo;
	if (h->vo && h->id)
		h->vo->close(h->id);
	free(h);
	return 0;
}

int video_output_write(void* vo, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h)
{
	struct vo_context_t* h = (struct vo_context_t*)vo;
	return h->vo->write(h->id, pic, src_x, src_y, src_w, src_h, tgt_x, tgt_y, tgt_w, tgt_h);
}

int video_output_read(void* vo, struct avframe_t* pic)
{
	struct vo_context_t* h = (struct vo_context_t*)vo;
	return h->vo->read?h->vo->read(h->id, pic):-1;
}

int video_output_control(void* vo, int command, void* param1, void* param2)
{
	struct vo_context_t* h = (struct vo_context_t*)vo;
	return h->vo->control?h->vo->control(h->id, command, param1, param2):-1;
}

int video_output_rotation(void* vo, float angle)
{
	struct vo_context_t* h = (struct vo_context_t*)vo;
	return h->vo->rotation?h->vo->rotation(h->id, angle):-1;
}
