#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "video_output.h"

// The main window class name.
static TCHAR szWindowClass[] = _T("win32app");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Win32 Guided Tour Application");

HINSTANCE hInst;

static unsigned char yuvbuffer[8*1920*1080];
static unsigned char yuvbuffer2[8*1920*1080];

static unsigned int GetTime()
{
	LARGE_INTEGER freq;
	LARGE_INTEGER count;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&count);
	return (unsigned int)(count.QuadPart * 1000 / freq.QuadPart);
}

static void VideoOutProf(HWND hWnd)
{
	int i;
	void* vo;
	picture_t pic;
	unsigned int tbegin, tnow;
	RECT rc;
	char msg[128];
	FILE* fp;

	pic.width = 352;
	pic.height = 288;
	pic.data[0] = yuvbuffer;
	pic.data[1] = yuvbuffer + pic.width*pic.height;
	pic.data[2] = pic.data[1] + pic.width*pic.height/4;
	pic.stride[0] = pic.width;
	pic.stride[1] = pic.width/2;
	pic.stride[2] = pic.width/2;

	//GetClientRect(hWnd, &rc);
	//MoveWindow(hWnd, rc.left, rc.top, pic.width, pic.height, FALSE);
	GetClientRect(hWnd, &rc);

	fp = fopen("352x288.yuv","rb");
	fread(yuvbuffer, 1, pic.width*pic.height*3/2, fp);
	fread(yuvbuffer2, 1, pic.width*pic.height*3/2, fp);

	vo = video_output_open(hWnd, video_output_yv12, pic.width, pic.height);

	tbegin = GetTime();
	for(i=0; i<10000; i++)
	{
		video_output_write(vo, &pic, 0, 0, pic.width, pic.height, 0, 0, rc.right-rc.left, rc.bottom-rc.top);
		if(0 == (i % 100))
		{
			tnow = GetTime();
			printf("fps: %0.1f\n", i*1000.0f/(tnow-tbegin));
		}
	}
	tnow = GetTime();
	printf("fps: %0.1f\n", i*1000.0f/(tnow-tbegin));
	video_output_close(vo);
	fclose(fp);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("Hello, World!");

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		// Here your application is laid out.
		// For this introduction, we just print out "Hello, World!"
		// in the top left corner.
		TextOut(hdc, 5, 5, greeting, _tcslen(greeting));
		// End application specific layout section.

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_RBUTTONDOWN:
		VideoOutProf(hWnd);

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

static int CreateSimpleWindow(HINSTANCE hInstance, int width, int height, HWND* hWnd)
{
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = szWindowClass;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Win32 Guided Tour"), NULL);
		return 1;
	}

	//hInst = hInstance; // Store instance handle in our global variable

	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application dows not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	*hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!*hWnd)
	{
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Win32 Guided Tour"), NULL);
		return 1;
	}

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(*hWnd, SW_SHOW);
	UpdateWindow(*hWnd);

	return 0;
}

int main(int argc, char* argv[])
{
	MSG msg;
	HWND hWnd;

	hInst = (HINSTANCE)GetModuleHandle(NULL);
	CreateSimpleWindow(hInst, 704, 576, &hWnd);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;	
}
