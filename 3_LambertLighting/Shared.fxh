// .fxh file
// 쉐이더 파일들이 공유하는 구조체와 정의들을 포함

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
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : TEXCOORD0;
};