#include <Windows.h>
#include <stdio.h>
#include "../../video_output/video_output.h"
#include "../../video_output/colorspace.h"
#include <time.h>
#include <assert.h>

static picture_t s_pic;
static unsigned char* yuvbuffer;//[4*1024*1024];
static unsigned int yuvFrames;
static int windows[16];

static int LoadData();
static DWORD OnPlayVideo(LPVOID lpThreadParameter)
{
	int id = (int)lpThreadParameter;

	while(1)
	{
		video_output vo;
		vo.open(windows[id], MAKEFOURCC('Y', 'V', '1', '2'), s_pic.width, s_pic.height);

		for(int i=0; i<5*20; i++)
		{
			int n = i % yuvFrames;
			picture_t pic;
			pic.width = s_pic.width;
			pic.height = s_pic.height;
			pic.data[0] = yuvbuffer + n * pic.width * pic.height * 3 / 2;
			pic.data[1] = pic.data[0] + pic.width*pic.height;
			pic.data[2] = pic.data[1] + pic.width*pic.height/4;
			pic.stride[0] = pic.width;
			pic.stride[1] = pic.width/2;
			pic.stride[2] = pic.width/2;
			vo.write(&pic, 0, 0, s_pic.width, s_pic.height, 0, 0, 0, 0);

			Sleep(40);
		}

		vo.close();
	}
}

static LRESULT CALLBACK WindowProc( HWND   hWnd,
								   UINT   msg,
								   WPARAM wParam,
								   LPARAM lParam )
{
	static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;

	switch( msg )
	{
	case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
			ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bMousing = true;
		}
		break;

	case WM_LBUTTONUP:
		{
			bMousing = false;
		}
		break;

	case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit.x = LOWORD (lParam);
			ptCurrentMousePosit.y = HIWORD (lParam);

			if( bMousing )
			{
				//g_fSpinX -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
				//g_fSpinY -= (ptCurrentMousePosit.y - ptLastMousePosit.y);
			}

			ptLastMousePosit.x = ptCurrentMousePosit.x;
			ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
		break;

	case WM_CLOSE:
		{
			PostQuitMessage(0);
		}

	case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;

	default:
		{
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}

int WINAPI WinMain(	HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR     lpCmdLine,
				   int       nCmdShow )
{
	if(0 != LoadData())
		return 0;
	video_output_setname("d3d9");

	WNDCLASSEX winClass; 
	MSG        uMsg;

	memset(&uMsg,0,sizeof(uMsg));

	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
	winClass.hIcon	       = LoadIcon(NULL, IDI_APPLICATION);
	winClass.hIconSm	   = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(5), IMAGE_ICON,  GetSystemMetrics(SM_CXSMICON),  GetSystemMetrics(SM_CYSMICON),  LR_DEFAULTCOLOR); 
	winClass.hCursor       =  LoadCursor(NULL, IDC_ARROW); 
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;

	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	int width = 1024;
	int height = 768;
	for(int i = 0; i < 16; i++)
	{
		int x = width/4 * (i % 4);
		int y = height/4 * (i / 4);
		int w = width/4;
		int h = height/4;

		char title[64] = {0};
		sprintf(title, "Direct3D (DX9) - Video (Window #%d)", i+1);
		windows[i] = (int)CreateWindowEx( NULL, "MY_WINDOWS_CLASS", title, 
			WS_POPUP | WS_VISIBLE,
			x, y, w, h, 
			NULL, NULL, hInstance, NULL );

		ShowWindow( (HWND)windows[i], nCmdShow );

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnPlayVideo, (LPVOID)i, 0, NULL);
	}

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
		else
		{
			Sleep(1);
		}
	}

	//shutDown();

	UnregisterClass( "MY_WINDOWS_CLASS", winClass.hInstance );

	return uMsg.wParam;
}

static int LoadData()
{
	const char* filename = "704x576.yuv";
	s_pic.width = 704;
	s_pic.height = 576;

	yuvbuffer = new unsigned char[200*1024*1024];
	yuvFrames = 0;

	FILE* fp = fopen(filename, "rb");
	if(!fp)
		return -1;

	s_pic.data[0] = yuvbuffer;
	s_pic.data[1] = s_pic.data[0] + s_pic.width*s_pic.height;
	s_pic.data[2] = s_pic.data[1] + s_pic.width*s_pic.height/4;
	s_pic.stride[0] = s_pic.width;
	s_pic.stride[1] = s_pic.width/2;
	s_pic.stride[2] = s_pic.width/2;

	while(s_pic.width*s_pic.height*3/2 == fread(yuvbuffer, 1, s_pic.width*s_pic.height*3/2, fp))
	{
		++yuvFrames;
		yuvbuffer += s_pic.width * s_pic.height * 3 / 2;
	}
	fclose(fp);
	yuvbuffer = s_pic.data[0];

	return 0;
}
