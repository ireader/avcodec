#include "video_output.h"
#include <map>
#include <string>
#include <stdlib.h>
#include <string.h>

typedef std::map<std::string, const video_output_t*> renders_t;
static renders_t s_renders;
static std::string s_render;

//////////////////////////////////////////////////////////////////////////
///
/// video global
///
//////////////////////////////////////////////////////////////////////////
extern "C" int video_output_register(const char* name, const video_output_t* vo)
{
	return s_renders.insert(std::make_pair(name, vo)).second ? 0 : -1;
}

void video_output_list(char* buffer, int len)
{
	int i = 0;
	for (renders_t::const_iterator it = s_renders.begin(); it != s_renders.end(); ++it)
	{
		const std::string& name = it->first;
		if (i + (int)name.length() + 1 >= len)
			break;

		if (i > 0) buffer[i++] = ',';
		memcpy(buffer + i, name.c_str(), name.length());
		i += name.length();
		buffer[i] = '\0';
	}
}

int video_output_setname(const char* name)
{
	if (s_renders.find(name) == s_renders.end())
		return -1;
	s_render = name;
	return 0;
}

const char* video_output_getname()
{
	return s_render.c_str();
}

//////////////////////////////////////////////////////////////////////////
///
/// video function
///
//////////////////////////////////////////////////////////////////////////
struct video_output_handle
{
	const video_output_t* vo;
	void* id;
};

void* video_output_open(void* window, int format, int width, int height)
{
	renders_t::const_iterator it = s_render.empty() ? s_renders.begin() : s_renders.find(s_render);
	if (it == s_renders.end())
		return NULL;

	const video_output_t* vo = it->second;
	void* id = vo->open(window, format, width, height);
	if(!id)
		return NULL;

	video_output_handle* handle = (video_output_handle*)malloc(sizeof(video_output_handle));
	if(NULL == handle)
	{
		vo->close(id);
		return NULL;
	}

	handle->vo = vo;
	handle->id = id;
	return handle;
}

int video_output_close(void* vo)
{
	video_output_handle* handle = (video_output_handle*)vo;
	int r = handle->vo->close(handle->id);
	free(handle);
	return r;
}

int video_output_write(void* vo, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h)
{
	video_output_handle* handle = (video_output_handle*)vo;
	return handle->vo->write(handle->id, pic, src_x, src_y, src_w, src_h, tgt_x, tgt_y, tgt_w, tgt_h);
}

int video_output_read(void* vo, struct avframe_t* pic)
{
	video_output_handle* handle = (video_output_handle*)vo;
	return handle->vo->read?handle->vo->read(handle->id, pic):-1;
}

int video_output_control(void* vo, int command, void* param1, void* param2)
{
	video_output_handle* handle = (video_output_handle*)vo;
	return handle->vo->control?handle->vo->control(handle->id, command, param1, param2):-1;
}

int video_output_rotation(void* vo, float angle)
{
	video_output_handle* handle = (video_output_handle*)vo;
	return handle->vo->rotation?handle->vo->rotation(handle->id, angle):-1;
}
