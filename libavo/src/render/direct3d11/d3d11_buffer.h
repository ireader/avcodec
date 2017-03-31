#pragma once

#include <d3d11.h>

struct TVertexPos
{
	float pos[3];
	float tex[2];
	float color[4];
};

struct TConstantParameter
{
	float view[4][4];
//	XMMATRIX view;
	//XMFLOAT4 tex;
	//XMFLOAT4 clip; // texture clip
};

static int d3d11_vertex_buffer_create(ID3D11Device* device, ID3D11Buffer** buffer)
{
	static TVertexPos vertices[] =
	{
		//{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		//{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		//{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },

		//{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		//{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		//{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ { 1.0f, 1.0f, 1.0f },  { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
		{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },

		{ { -1.0f, -1.0f, 1.0f },{ 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
		{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
		{ { 1.0f, 1.0f, 1.0f },  { 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } },
		//{ XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
		//{ XMFLOAT3(1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		//{ XMFLOAT3(0.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },

		//{ XMFLOAT3(0.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
		//{ XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
		//{ XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(vertices);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA res;
	ZeroMemory(&res, sizeof(res));
	res.pSysMem = vertices;

	return device->CreateBuffer(&desc, &res, buffer);
	//vo->d3dContext->IASetVertexBuffers(0, 1, *buffer, &stride, &offset);
}

static int d3d11_constant_buffer_create(ID3D11Device* device, ID3D11Buffer** buffer)
{
	static float identifyMatix[4][4] = { { 1.0f, 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f, 0.0f, 1.0f } };

	TConstantParameter param;
	ZeroMemory(&param, sizeof(param));
	CopyMemory(param.view, identifyMatix, sizeof(param.view));

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(param);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA res;
	ZeroMemory(&res, sizeof(res));
	res.pSysMem = &param;

	return device->CreateBuffer(&desc, &res, buffer);
	//vo->d3dContext->VSSetConstantBuffers(0, 1, *buffer);
}
