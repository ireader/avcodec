#pragma once

#include <d3d11.h>
#include <assert.h>

static int d3d11_texture_create(ID3D11Device* device, DXGI_FORMAT format, int width, int height, ID3D11Texture2D** texture, ID3D11ShaderResourceView** shaderView)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Format = shaderResourceViewDesc.Format;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	if (FAILED(hr = device->CreateTexture2D(&textureDesc, NULL, texture))
		|| FAILED(hr = device->CreateShaderResourceView(*texture, &shaderResourceViewDesc, shaderView)))
		return hr;

	return S_OK;
}

static int d3d11_texture_write(ID3D11DeviceContext* d3dContext, ID3D11Texture2D* texture, const BYTE* data, int width, int height, int linesize)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = d3dContext->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(hr))
	{
		assert(mappedResource.RowPitch >= (UINT)width);
		if (mappedResource.RowPitch == (UINT)linesize)
		{
			memcpy(mappedResource.pData, data, linesize * height);
		}
		else if(mappedResource.RowPitch >= (UINT)width)
		{
			BYTE* pd = (BYTE*)mappedResource.pData;
			for (int j = 0; j < height; j++)
				memcpy(&pd[j * mappedResource.RowPitch], &data[j * linesize], width);
		}

		d3dContext->Unmap(texture, 0);
	}

	return hr;
}
