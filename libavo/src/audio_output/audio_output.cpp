#include "audio_output.h"
#include <map>
#include <string>

#define MAX_NAME_LENGTH	60

class audio_output_info
{
	audio_output_info()
	{
		memset(m_name, 0, sizeof(m_name));
	}

public:
	static audio_output_info& get_instance()
	{
		static audio_output_info s_voi;
		return s_voi;
	}

	const char* get_name() const
	{
		return m_name;
	}

	bool set_name(const char* name)
	{
		aos_t::const_iterator it = m_aos.find(name);
		if(it == m_aos.end())
			return false;

		if(strlen(name) >= sizeof(m_name)/sizeof(m_name[0])-1)
			return false;
		strcpy(m_name, name);
		return true;
	}

	const audio_output_t* get_audio_output()
	{
		aos_t::const_iterator it = m_aos.find(m_name);
		if(it != m_aos.end())
			return it->second;
		return m_aos.size() > 0 ? m_aos.begin()->second : NULL;
	}

	bool registe(const char* name, const audio_output_t* vo)
	{
		return m_aos.insert(std::make_pair(name, vo)).second;
	}

	void list(char* buffer, int len)
	{
		int i = 0;
		for(aos_t::const_iterator it=m_aos.begin(); it!=m_aos.end(); ++it)
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
	typedef std::map<std::string, const audio_output_t*> aos_t;
	aos_t	m_aos;
	char	m_name[MAX_NAME_LENGTH+1];
};


//////////////////////////////////////////////////////////////////////////
///
/// video global
///
//////////////////////////////////////////////////////////////////////////
int audio_output_register(const char* name, const audio_output_t* vo)
{
	return audio_output_info::get_instance().registe(name, vo) ? 1 : 0;
}

void audio_output_list(char* buffer, int len)
{
	audio_output_info::get_instance().list(buffer, len);
}

int audio_output_setname(const char* name)
{
	return audio_output_info::get_instance().set_name(name) ? 1 : 0;
}

const char* audio_output_getname()
{
	return audio_output_info::get_instance().get_name();
}

//////////////////////////////////////////////////////////////////////////
///
/// audio function
///
//////////////////////////////////////////////////////////////////////////

typedef struct _ao_context_t
{
	const audio_output_t* ao;
	void* id;
} ao_context_t;

void* audio_output_open(int channels, int bits_per_sample, int samples_per_second)
{
	ao_context_t* context;
	
	context = (ao_context_t*)malloc(sizeof(ao_context_t));
	if (!context)
		return NULL;
	memset(context, 0, sizeof(ao_context_t));

	context->ao = audio_output_info::get_instance().get_audio_output();
	if (NULL != context->ao)
	{
		context->id = context->ao->open(channels, bits_per_sample, samples_per_second);
		if (NULL != context->id)
			return context;
	}
	
	free(context);
	return NULL;
}

int audio_output_close(void* ao)
{
	ao_context_t* context = (ao_context_t*)ao;
	if(context->ao && context->id)
		context->ao->close(context->id);
	free(context);
	return 0;
}

int audio_output_write(void* ao, const void* samples, int count)
{
	ao_context_t* context = (ao_context_t*)ao;
	return context->ao->write(context->id, samples, count);
}

int audio_output_play(void* ao)
{
	ao_context_t* context = (ao_context_t*)ao;
	return context->ao->play(context->id);
}

int audio_output_pause(void* ao)
{
	ao_context_t* context = (ao_context_t*)ao;
	return context->ao->pause(context->id);
}

int audio_output_reset(void* ao)
{
	ao_context_t* context = (ao_context_t*)ao;
	return context->ao->reset(context->id);
}

int audio_output_getbuffersize(void* ao)
{
	ao_context_t* context = (ao_context_t*)ao;
	return context->ao->get_buffer_size(context->id);
}

int audio_output_getavailablesamples(void* ao)
{
	ao_context_t* context = (ao_context_t*)ao;
	return context->ao->get_available_sample(context->id);
}

int audio_output_getinfo(void* ao, int *channel, int *bits_per_sample, int *samples_per_second)
{
	ao_context_t* context;
	context = (ao_context_t*)ao;
	return context->ao->get_info(context->id, channel, bits_per_sample, samples_per_second);
}

int audio_output_getvolume(void* ao, int* v)
{
	ao_context_t* context = (ao_context_t*)ao;
	return (context->ao&&context->ao->get_volume)?context->ao->get_volume(context->id, v):-1;
}

int audio_output_setvolume(void* ao, int v)
{
	ao_context_t* context = (ao_context_t*)ao;
	return (context->ao&&context->ao->set_volume)?context->ao->set_volume(context->id, v):-1;
}
