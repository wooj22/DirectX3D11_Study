Texture2D txColorMap : register(t0);
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

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 materialColor = txColorMap.Sample(samLinear, input.texCoord);
    return saturate(dot((float3) lightDirection, input.normal) * (lightColor * materialColor));
}