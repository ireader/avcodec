#include "av_register.h"
#include <stdlib.h>
#include <string.h>

#define N_CLASSES 8

struct av_class_t
{
	const char* name;
	const void* avclass;
};

static struct av_class_t s_classes[4][N_CLASSES];
static const struct av_class_t* s_default[4];

void av_list(int avtype, void(*item)(void* param, const char*), void* param)
{
	int i;
	if (avtype < AV_AUDIO_RECORDER || avtype > AV_VIDEO_RENDER)
		return;

	for (i = 0; i < N_CLASSES && s_classes[avtype][i].name; i++)
	{
		item(param, s_classes[avtype][i].name);
	}
}

static struct av_class_t* av_find(int avtype, const char* name)
{
	int i;
	
	if (avtype < AV_AUDIO_RECORDER || avtype > AV_VIDEO_RENDER)
		return NULL;
	
	for (i = 0; i < N_CLASSES && s_classes[avtype][i].name; i++)
	{
		if (0 == strcmp(s_classes[avtype][i].name, name))
			return &s_classes[avtype][i];
	}
	return NULL;
}

int av_set_name(int avtype, const char* name)
{
	struct av_class_t* avclass;
	avclass = av_find(avtype, name);
	if (NULL == avclass) return -1;

	s_default[avtype] = avclass;
	return 0;
}

const char* av_get_name(int avtype)
{
	if (avtype < AV_AUDIO_RECORDER || avtype > AV_VIDEO_RENDER)
		return NULL;

	return s_default[avtype] ? s_default[avtype]->name : NULL;
}

const void* av_get_class(int avtype)
{
	if (avtype < AV_AUDIO_RECORDER || avtype > AV_VIDEO_RENDER)
		return NULL;

	return s_default[avtype] ? s_default[avtype]->avclass : s_classes[avtype][0].avclass;
}

int av_set_class(int avtype, const char* name, const void* cls)
{
	int i;
	if (avtype < AV_AUDIO_RECORDER || avtype > AV_VIDEO_RENDER)
		return -1;

	for (i = 0; i < N_CLASSES && s_classes[avtype][i].name; i++)
	{
		if (0 == strcmp(s_classes[avtype][i].name, name))
			return -1; // exist
	}

	if (i >= N_CLASSES)
		return -1;

	s_classes[avtype][i].name = name;
	s_classes[avtype][i].avclass = cls;
	return i;
}
