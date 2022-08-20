#pragma once

#include <d3dcompiler.h>
#include <stdio.h>

typedef HRESULT (WINAPI *pD3DCompileFromFile)(_In_ LPCWSTR pFileName,
	_In_reads_opt_(_Inexpressible_(pDefines->Name != NULL)) CONST D3D_SHADER_MACRO* pDefines,
	_In_opt_ ID3DInclude* pInclude,
	_In_ LPCSTR pEntrypoint,
	_In_ LPCSTR pTarget,
	_In_ UINT Flags1,
	_In_ UINT Flags2,
	_Out_ ID3DBlob** ppCode,
	_Out_opt_ ID3DBlob** ppErrorMsgs);

static pD3DCompileFromFile fpD3DCompileFromFile;

static int CompileShaderFromFile(LPCWSTR filename, const char* functionName, const char* profile, ID3DBlob** buffer)
{
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	//shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* errBuffer = NULL;
	HRESULT hr = fpD3DCompileFromFile(filename, 0, 0, functionName, profile, shaderFlags, 0, buffer, &errBuffer);
	if (errBuffer)
	{
		errBuffer->Release();
	}
	return hr;
}

static int d3d11_compile()
{
	HMODULE hD3dCompile = LoadLibraryExA("D3DCompiler_47.dll", NULL, 0);
	fpD3DCompileFromFile = (pD3DCompileFromFile)GetProcAddress(hD3dCompile, "D3DCompileFromFile");
	if (NULL == fpD3DCompileFromFile)
		return -1;

	HRESULT hr = S_OK;
	ID3DBlob* vsBuffer = NULL;
	ID3DBlob* psBuffer = NULL;
	if (SUCCEEDED(hr = CompileShaderFromFile(L"d3d11_shader.hlsl", "VS_Main", "vs_4_0", &vsBuffer))
		&& SUCCEEDED(hr = CompileShaderFromFile(L"d3d11_shader.hlsl", "PS_Main", "ps_4_0", &psBuffer)))
	{
		BYTE* p = (BYTE*)vsBuffer->GetBufferPointer();
		size_t n = vsBuffer->GetBufferSize();
		for (size_t i = 0; i < vsBuffer->GetBufferSize(); i++)
		{
			if (i % 16 == 0)
				printf("\n");
			printf("0x%02X, ", *p++);
		}

		p = (BYTE*)psBuffer->GetBufferPointer();
		n = psBuffer->GetBufferSize();
		for (size_t i = 0; i < psBuffer->GetBufferSize(); i++)
		{
			if (i % 16 == 0)
				printf("\n");
			printf("0x%02X, ", *p++);
		}
	}

	FreeLibrary(hD3dCompile);
	return hr;
}
