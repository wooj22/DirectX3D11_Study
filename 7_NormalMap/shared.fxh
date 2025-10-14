// constant buffer
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    
    float4 lightDirection;
    float4 lightColor;
    
    float indirectLight;
    float directLight;
    
    float ambientReflection;
    float diffuseReflection;
    float specularReflection;
    float shininess;
    float2 padding1;
    
    float3 cameraPos;
    float padding2;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_Position;
    float3 worldPos : WORLD_POSITION;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD; 
};
