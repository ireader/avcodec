#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "video_output.h"
#include "colorspace.h"

static unsigned char yuvbuffer[1024*1024];
static unsigned char rgbbuffer[1024*1024];
int main(int argc, char* argv[])
{
	for(int i=1; i<argc; ++i)
	{
		const char* arg = argv[i];
		if(0==strcmp(arg, "-v") && i+1<argc)
			video_output_setname(argv[i+1]);
	}

	Display* d;
	Window w;
	XEvent e;
	int s;
	
	d = XOpenDisplay(NULL);
	if(d == NULL)
	{
		fprintf(stderr, "Cannot open display.\n");
		exit(1);
	}

	s = DefaultScreen(d);
	printf("default depth: %d\n", DefaultDepth(d, s));
	w = XCreateWindow(d, RootWindow(d, s), 0, 0, 352, 288, 1, CopyFromParent, InputOutput, CopyFromParent, 0, NULL);
	XSelectInput(d, w, ExposureMask|KeyPressMask);
	XMapWindow(d, w);

	const char* filename = "video.yuv";
	FILE* m_fp = fopen(filename, "rb");
	if(NULL == m_fp)
	{
		XCloseDisplay(d);
		fprintf(stderr, "Cannot open file.\n");
		exit(-1);
	}

	video_output m_vo;

	picture_t pic;
	pic.width = 352;
	pic.height = 288;
	pic.stride[0] = pic.width;
	pic.stride[1] = pic.width/2;
	pic.stride[2] = pic.width/2;
	pic.data[0] = yuvbuffer;
	pic.data[1] = yuvbuffer + pic.stride[0]*pic.height;
	pic.data[2] = pic.data[1] + pic.stride[1]*pic.height/2;

	while(1)
	{
		XNextEvent(d, &e);
		if(e.type == Expose)
		{
			XFillRectangle(d, w, DefaultGC(d, s), 0, 0, 352, 288);
			//continue;

			if(!m_vo.open(int(w), video_output_yv12, pic.width, pic.height))
			{
				XCloseDisplay(d);
				fprintf(stderr, "Cannot open video.\n");
				exit(-1);
			}

			while(1)
			{
				pic.stride[0] = pic.width;
				pic.data[0] = yuvbuffer;
				if(1 != fread(yuvbuffer, pic.width*pic.height*3/2, 1, m_fp))
				{
					fclose(m_fp);
					m_fp = NULL;
					return -1;
				}

#if 0
				yv12_rgb32(pic.data[0], pic.data[2], pic.data[1], pic.stride[0], pic.stride[1], pic.width, pic.height, rgbbuffer);
				pic.data[0] = rgbbuffer;
				pic.stride[0] = pic.width*4;
#endif
				m_vo.write(&pic, 0, 0, pic.width, pic.height, 0, 0, 352, 288);

				//for(int i=0; i<50; i++)
				//{
				//	//usleep(40*1000);
				//	//m_vo.write(&pic, 0, 0, 352, 288, 0, 0, 0, 0);
				//	//m_vo.write(&pic, 121+i, 91+i, 176, 144, 0, 0, 0, 0);
				//}
				usleep(1000*1000);
			}
		}
		if(e.type == KeyPress)
			break;
	}

	XCloseDisplay(d);
	return 0;
}
