#include "video_write.h"
#include <assert.h>
#include <string.h>

static void video_write_y8(const struct avframe_t* pic, uint8_t *data, int pitches)
{
	int i;
	uint8_t *d0;
	assert(pitches >= (int)pic->width);
	assert(pic->linesize[0] >= pic->width);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches, d0+=pic->linesize[0])
		memcpy(data, d0, pic->width);

	memset(data, 128, pitches * pic->height/2);
}

static void video_write_yv12(const struct avframe_t* pic, uint8_t *data, int pitches)
{
	int i;
	uint8_t *d0, *d1, *d2;
	assert(pitches >= (int)pic->width);
	assert(pic->linesize[0] >= pic->width);
	assert(pic->linesize[1] >= pic->width/2);
	assert(pic->linesize[2] >= pic->width/2);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches, d0+=pic->linesize[0])
		memcpy(data, d0, pic->width);

	for(i=0, d1=pic->data[1]; i<pic->height/2; i++, data+=pitches/2, d1+=pic->linesize[1])
		memcpy(data, d1, pic->width/2);

	for(i=0, d2=pic->data[2]; i<pic->height/2; i++, data+=pitches/2, d2+=pic->linesize[2])
		memcpy(data, d2, pic->width/2);
}

static void video_write_yv16(const struct avframe_t* pic, uint8_t *data, int pitches)
{
	int i;
	uint8_t *d0, *d1, *d2;
	assert(pitches >= (int)pic->width);
	assert(pic->linesize[0] >= pic->width);
	assert(pic->linesize[1] >= pic->width/2);
	assert(pic->linesize[2] >= pic->width/2);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches, d0+=pic->linesize[0])
		memcpy(data, d0, pic->width);

	for(i=0, d1=pic->data[1]; i<pic->height/2; i++, data+=pitches/2, d1+=pic->linesize[1]*2)
		memcpy(data, d1, pic->width/2);

	for(i=0, d2=pic->data[2]; i<pic->height/2; i++, data+=pitches/2, d2+=pic->linesize[2]*2)
		memcpy(data, d2, pic->width/2);
}

static void video_write_nv12(const struct avframe_t* pic, uint8_t *data, int pitches)
{
	int i;
	uint8_t *d0, *d1;
	assert(pitches >= (int)pic->width);
	assert(pic->linesize[0] >= pic->width);
	assert(pic->linesize[1] >= pic->width);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches, d0+=pic->linesize[0])
		memcpy(data, d0, pic->width);

	for(i=0, d1=pic->data[1]; i<pic->height/2; i++, data+=pitches, d1+=pic->linesize[1])
		memcpy(data, d1, pic->width);
}

static void video_write_rgb24_planar(const struct avframe_t* pic, uint8_t *data, int pitches)
{
	int i, j;
	uint8_t *r, *g, *b;
	assert(pitches >= (int)pic->width * 4);
	assert(pic->linesize[0] >= pic->width);
	assert(pic->linesize[1] >= pic->width);
	assert(pic->linesize[2] >= pic->width);

	r = pic->data[0];
	g = pic->data[1];
	b = pic->data[2];

	for(i=0; i<pic->height; i++, data+=pitches)
	{
		for(j = 0; j < pic->width; j++)
		{
			data[j*4 + 0] = b[i*pic->linesize[2] + j];
			data[j*4 + 1] = g[i*pic->linesize[1] + j];
			data[j*4 + 2] = r[i*pic->linesize[0] + j];
			data[j*4 + 3] = 0;
		}
	}
}

static void video_write_rgb24(const struct avframe_t* pic, uint8_t *data, int pitches)
{
	int i, j;
	uint8_t *d0;
	assert(pitches >= (int)pic->width * 4);
	assert(pic->linesize[0] >= pic->width * 3);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches, d0+=pic->linesize[0])
	{
		for(j = 0; j < pic->width; j++)
		{
			data[j*4 + 0] = d0[j*3 + 0];
			data[j*4 + 1] = d0[j*3 + 1];
			data[j*4 + 2] = d0[j*3 + 2];
			data[j*4 + 3] = 0;
		}
	}
}

static void video_write_rgb32(const struct avframe_t* pic, uint8_t *data, int pitches)
{
	int i;
	uint8_t *d0;
	assert(pitches >= (int)pic->width * 4);
	assert(pic->linesize[0] >= pic->width * 4);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches, d0+=pic->linesize[0])
		memcpy(data, d0, pic->width * 4);
}

int video_write(const struct avframe_t* pic, void* data, int pitches)
{
	switch(pic->format)
	{
	//case PICTURE_RGBA:
	//	video_write_rgb24_planar(pic, (uint8_t*)data, pitches);
 //       break;

	case PICTURE_RGB24:
		video_write_rgb24(pic, (uint8_t*)data, pitches);
		break;

	case PICTURE_RGB32:
		video_write_rgb32(pic, (uint8_t*)data, pitches);
		break;

	//case video_output_y8:
	//	video_write_y8(pic, (uint8_t*)data, pitches);
	//	break;

	case PICTURE_YUV420:
		video_write_yv12(pic, (uint8_t*)data, pitches);
		break;

	case PICTURE_YUV422:
		video_write_yv16(pic, (uint8_t*)data, pitches);
		break;

	case PICTURE_NV12:
		video_write_nv12(pic, (uint8_t*)data, pitches);
		break;

	default:
		return -1;
	}
	return 0;
}
