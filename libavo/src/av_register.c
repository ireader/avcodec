#include "av_register.h"
#include <stdlib.h>
#include <string.h>

static const char* s_name[4];
static const char* s_names[4][8];
static const void* s_classes[4][8];

void av_list(int avtype, void(*item)(void* param, const char*), void* param)
{
	int i;
	if (avtype < AV_AUDIO_CAPTURE || avtype > AV_VIDEO_RENDER)
		return;

	for (i = 0; i < 8 && s_names[avtype][i]; i++)
	{
		item(param, s_names[avtype][i]);
	}
}

static int av_find(int avtype, const char* name)
{
	int i;
	
	if (avtype < AV_AUDIO_CAPTURE || avtype > AV_VIDEO_RENDER)
		return -1;
	
	for (i = 0; i < 8 && s_names[avtype][i]; i++)
	{
		if (0 == strcmp(s_names[avtype][i], name))
			return i;
	}
	return -1;
}

int av_set_name(int avtype, const char* name)
{
	int i;
	i = av_find(avtype, name);
	if (-1 == i) return -1;

	s_name[avtype] = s_names[avtype][i];
	return 0;
}

const char* av_get_name(int avtype)
{
	if (avtype < AV_AUDIO_CAPTURE || avtype > AV_VIDEO_RENDER)
		return NULL;

	return s_name[avtype];
}

const void* av_get_class(int avtype)
{
	int i = -1;
	if (avtype < AV_AUDIO_CAPTURE || avtype > AV_VIDEO_RENDER)
		return NULL;

	if (NULL != s_name[avtype])
		i = av_find(avtype, s_name[avtype]);
	return s_classes[avtype][-1 == i ? 0 : i];
}

int av_set_class(int avtype, const char* name, const void* cls)
{
	int i;
	if (avtype < AV_AUDIO_CAPTURE || avtype > AV_VIDEO_RENDER)
		return -1;

	for (i = 0; i < 8 && s_names[avtype][i]; i++)
	{
		if (0 == strcmp(s_names[avtype][i], name))
			return -1; // exist
	}

	s_classes[avtype][i] = name;
	s_classes[avtype][i] = cls;
	return i;
}
