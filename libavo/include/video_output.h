#ifndef _video_output_h_
#define _video_output_h_

#include "avframe.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	void* (*open)(void* window, int format, int width, int height);
	int (*close)(void* vo);
	int (*write)(void* vo, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h);
	int (*read)(void* vo, struct avframe_t* pic);
	int (*control)(void* vo, int command, void* param1, void* param2);
	int (*rotation)(void* vo, float angle);
} video_output_t;

/// Create video output handle
/// @param[in] window windows-window handle, X11-drawable object
/// @param[in] format video format, e.g. YV12, RGB32
/// @param[in] width video width
/// @param[in] height video height
/// @return NULL-error, other-video output handle
void* video_output_open(void* window, int format, int width, int height);

/// Close video output handle
/// @param[in] vo video output handle
/// @return 0-success, other-error
int video_output_close(void* vo);

/// Show video
/// @param[in] vo video output handle
/// @param[in] pic picture
/// @param[in] src_x src_y/src_w/src_h picture rect area
/// @param[in] tgt_x tgt_y/tgt_w/tgt_h window rect area
/// @return 0-success, other-error
int video_output_write(void* vo, const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h);

/// read image
/// @param[in] vo video output handle
/// @param[in,out] pic video picture, picture->data malloc/free by caller
/// @return 0-success, other-error
int video_output_read(void* vo, struct avframe_t* pic);

/// video control
/// @param[in] vo video output handle
/// @param[in] command video control code
/// @param[in] param1 user defined parameter
/// @param[in] param2 user defined parameter
/// @return 0-success, other-error
int video_output_control(void* vo, int command, void* param1, void* param2);

/// rotation in center point
/// @param[in] vo video output handle
/// @param[in] angle angle in radians(clockwise)
/// @return 0-success, other-error
int video_output_rotation(void* vo, float angle);

#ifdef __cplusplus
}

class video_output
{
public:
	video_output():m_vo(0), m_window(0), m_format(0), m_width(0), m_height(0){}
	~video_output(){ close(); }

public:
	bool open(void* window, int format, int width, int height)
	{
		if(m_window!=window || m_format!=format || m_width!=width || m_height!=height)
			close();

		if(m_vo)
			return true;

		m_vo = video_output_open(window, format, width, height);
		if(!m_vo)
			return false;

		m_window = window;
		m_format = format;
		m_width = width;
		m_height = height;
		return true;
	}

	int close()
	{
		if(m_vo)
			video_output_close(m_vo);
		m_vo = 0;
		return 0;
	}

	int write(const struct avframe_t* pic, int src_x, int src_y, int src_w, int src_h, int tgt_x, int tgt_y, int tgt_w, int tgt_h)
	{
		if(!m_vo)
			return -1;
		return video_output_write(m_vo, pic, src_x, src_y, src_w, src_h, tgt_x, tgt_y, tgt_w, tgt_h);
	}

	int read(struct avframe_t* pic) const
	{
		if(!m_vo)
			return -1;
		return video_output_read(m_vo, pic);
	}

	int control(int command, void* param1, void* param2)
	{
		if(!m_vo)
			return -1;
		return video_output_control(m_vo, command, param1, param2);
	}

	int rotation(float angle)
	{
		if(!m_vo)
			return -1;
		return video_output_rotation(m_vo, angle);
	}

private:
	video_output(const video_output&){}
	video_output& operator=(const video_output&){ return *this; }

private:
	void*	m_vo;
	void*	m_window;
	int		m_format;
	int		m_width;
	int		m_height;
	struct avframe_t m_picture;
};

#endif

#endif /* !_video_output_h_ */
