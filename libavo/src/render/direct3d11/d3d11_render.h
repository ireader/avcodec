#ifndef _d3d11_render_h_
#define _d3d11_render_h_

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdio.h>

inline void DxLog(const char* format, ...)
{
	static TCHAR msg[2 * 1024] = { 0 };

	va_list vl;
	va_start(vl, format);
	vsnprintf(msg, sizeof(msg) - 1, format, vl);
	va_end(vl);

	OutputDebugString(msg);
}

typedef struct
{
	int width;
	int height;
	int format;
	int window_width;
	int window_height;
	HWND window;
//	TConstantParameter zoom;
	float rotation;

	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* backBufferTarget;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* constantBuffer;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11Texture2D* yuv[3];
	ID3D11ShaderResourceView* shaderView[3];
	ID3D11SamplerState* samplerState;

	D3D_FEATURE_LEVEL featureLevel;
} d3d11_render_t;

#endif /* !_d3d11_render_h_ */
