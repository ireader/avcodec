#include "video_read.h"
#include <assert.h>
#include <string.h>

static void video_read_yv12(const unsigned char *data, int *pitches, struct avframe_t* pic)
{
	int i;
	uint8_t *d0, *d1, *d2;
	assert(pitches[0] >= (int)pic->width);
	assert(pic->linesize[0] >= pic->width);
	assert(pic->linesize[1] >= pic->width/2);
	assert(pic->linesize[2] >= pic->width/2);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches[0], d0+=pic->linesize[0])
		memcpy(d0, data, pic->width);
	
	for(i=0, d1=pic->data[1]; i<pic->height/2; i++, data+=pitches[0]/2, d1+=pic->linesize[1])
		memcpy(d1, data, pic->width/2);

	for(i=0, d2=pic->data[2]; i<pic->height/2; i++, data+=pitches[0]/2, d2+=pic->linesize[2])
		memcpy(d2, data, pic->width/2);
}

static void video_read_nv12(const unsigned char *data, int *pitches, struct avframe_t* pic)
{
	int i;
	uint8_t *d0, *d1;
	assert(pitches[0] >= (int)pic->width);
	assert(pic->linesize[0] >= pic->width);
	assert(pic->linesize[1] >= pic->width);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches[0], d0+=pic->linesize[0])
		memcpy(d0, data, pic->width);

	for(i=0, d1=pic->data[1]; i<pic->height/2; i++, data+=pitches[0], d1+=pic->linesize[1])
		memcpy(d1, data, pic->width);
}

static void video_read_rgb24(const unsigned char *data, int *pitches, struct avframe_t* pic)
{
	int i;
	uint8_t *d0;
	assert(pitches[0] >= (int)pic->width * 3);
	assert(pic->linesize[0] >= pic->width * 3);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches[0], d0+=pic->linesize[0])
		memcpy(d0, data, pic->width * 3);
}

static void video_read_rgb32(const unsigned char *data, int *pitches, struct avframe_t* pic)
{
	int i;
	uint8_t *d0;
	assert(pitches[0] >= (int)pic->width * 4);
	assert(pic->linesize[0] >= pic->width * 4);

	for(i=0, d0=pic->data[0]; i<pic->height; i++, data+=pitches[0], d0+=pic->linesize[0])
		memcpy(d0, data, pic->width * 4);
}

int video_read(const void* data, int *pitches, int format, struct avframe_t* pic)
{
	switch(format)
	{
	case PICTURE_YUV420:
		video_read_yv12((const unsigned char*)data, pitches, pic);
		break;

	case PICTURE_NV12:
		video_read_nv12((const unsigned char*)data, pitches, pic);
		break;

	case PICTURE_RGB24:
		video_read_rgb24((const unsigned char*)data, pitches, pic);
		break;

	case PICTURE_RGB32:
		video_read_rgb32((const unsigned char*)data, pitches, pic);
		break;

	default:
		return -1;
	}
	return 0;
}
