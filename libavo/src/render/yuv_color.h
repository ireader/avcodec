#ifndef _yuv_color_h_
#define _yuv_color_h_

// BT.709, which is the standard for HDTV.
const static float s_bt709[] = {
	1.164f,  1.164f,  1.164f,
	0.0f,   -0.213f,  2.112f,
	1.793f, -0.533f,  0.0f,
};

// ITU-R BT.601 - for SD (720x576)
const static float s_bt601[] = {
	1.164f,  1.164f, 1.164f,
	0.0f,   -0.392f, 2.017f,
	1.596f, -0.813f, 0.0f,
};

#endif /* !_yuv_color_h_ */
