// .fxh file
// ���̴� ���ϵ��� �����ϴ� ����ü�� ���ǵ��� ����

// Texture ���� ����
Texture2D txColorMap : register(t0);

// SamplerState ���� ����
SamplerState samLinear : register(s0);

// constant buffer
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    
    float4 lightDirection;
    float4 lightColor;
}

struct VS_INPUT
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};