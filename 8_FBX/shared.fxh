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
    
    float ambientHighlight;
    float diffuseHighlight;
    float specularHighlight;
    float shininess;
    float2 padding1;
    
    float3 cameraPos;
    
    bool useDiffuse;
    bool useNormal;
    bool useSpecular;
    bool useEmissive;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : WORLD_POSITION;
    float3 normal : NORMAL;
    float3x3 TBN : TBN;
    float2 texCoord : TEXCOORD;
};
