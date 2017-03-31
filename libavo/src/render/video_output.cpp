#include "video_output.h"
#include <map>
#include <string>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH	60

class video_output_info
{
	video_output_info()
	{
		m_name[0] = 0;
	}

public:
	static video_output_info& get_instance()
	{
		static video_output_info s_voi;
		return s_voi;
	}

	const char* get_name() const
	{
		return m_name;
	}

	bool set_name(const char* name)
	{
		vos_t::const_iterator it = m_vos.find(name);
		if(it == m_vos.end())
			return false;

		if(strlen(name) >= sizeof(m_name)/sizeof(m_name[0])-1)
			return false;
		strcpy(m_name, name);
		return true;
	}

	const video_output_t* get_video_output()
	{
		vos_t::const_iterator it = m_vos.find(m_name);
		if(it != m_vos.end())
			return it->second;
		return m_vos.size() > 0 ? m_vos.begin()->second : NULL;
	}

	bool registe(const char* name, const video_output_t* vo)
	{
		return m_vos.insert(std::make_pair(name, vo)).second;
	}

	void list(char* buffer, int len)
	{
		int i = 0;
		for(vos_t::const_iterator it=m_vos.begin(); it!=m_vos.end(); ++it)
		{
			const std::string name = it->first;
			if(i+(int)name.length()+1 >= len)
				break;

			if(i > 0)
				buffer[i++] = '\n';

			strcpy(buffer+i, name.c_str());
		}
	}

private:
	typedef std::map<std::string, const video_output_t*> vos_t;
	vos_t	m_vos;
	char	m_name[MAX_NAME_LENGTH+1];
};


//////////////////////////////////////////////////////////////////////////
///
/// video global
///
//////////////////////////////////////////////////////////////////////////
int video_output_register(const char* name, const video_output_t* vo)
{
	return video_output_info::get_instance().registe(name, vo) ? 1 : 0;
}

void video_output_list(char* buffer, int len)
{
	video_output_info::get_instance().list(buffer, len);
}

int video_output_setname(const char* name)
{
	return video_output_info::get_instance().set_name(name) ? 1 : 0;
}

const char* video_output_getname()
{
	return video_output_info::get_instance().get_name();
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

void* video_output_open(int window, int format, int width, int height)
{
	const video_output_t* vo = video_output_info::get_instance().get_video_output();
	if(NULL == vo)
		return NULL;

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
