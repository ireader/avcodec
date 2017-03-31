// SDL_render_d3d11.c

Texture2D yTex : register(t0);
Texture2D uTex : register(t2);
Texture2D vTex : register(t1);
SamplerState colorSampler : register(s0);

cbuffer ConstantBuffer : register( b0 )
{
    matrix cb_view;
    float4 cb_tex;
}

struct VS_INPUT
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
};

VS_OUTPUT VS_Main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	float4 pos = float4(input.pos, 1.0f);
	output.pos = mul(pos, cb_view);
	output.tex = input.tex;
	output.color = input.color;

	return output;
}

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
	const float3 offset = {-0.0627451017, -0.501960814, -0.501960814};
    const float3 Rcoeff = {1.164,  0.000,  1.596};
    const float3 Gcoeff = {1.164, -0.391, -0.813};
    const float3 Bcoeff = {1.164,  2.018,  0.000};

    float3 yuv;
    yuv.x = yTex.Sample(colorSampler, input.tex).r;
    yuv.y = uTex.Sample(colorSampler, input.tex).r;
    yuv.z = vTex.Sample(colorSampler, input.tex).r;
    yuv += offset;
    
	float4 Output;
	Output.r = dot(yuv, Rcoeff);
    Output.g = dot(yuv, Gcoeff);
    Output.b = dot(yuv, Bcoeff);
    Output.a = 1.0f;

    return Output * input.color;
}
