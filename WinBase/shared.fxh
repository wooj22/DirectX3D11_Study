
// ------------------
//  ¹ÙÀÎµù ½½·Ô
// ------------------
// [ Constant Buffer ]
// TransformCB -> b0
// LightingCB -> b1
// MaterialCB -> b2
// OffsetMatrixCB -> b3
// PoseMatrixCB -> b4

// [ Texture ]
// Texture2D diffuseMap : register(t0)
// Texture2D normalMap : register(t1)
// Texture2D specularMap : register(t2)
// Texture2D emissiveMap : register(t3)
// TextureCube skyboxTexture : register(t4)
// Texture2D diffuseRamp : register(t4);
// Texture2D specualrRamp : register(t5);


// [ SamplerState ]
// SamplerState samLinear : register(s0)


// ------------------
//  Constant Buffer
// ------------------
cbuffer TransformCB : register(b0)
{
    matrix model;
    matrix world;
    matrix view;
    matrix projection;
}

cbuffer LightingCB : register(b1)
{
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
    float padidng2;
}

cbuffer MaterialCB : register(b2)
{
    bool useDiffuse;
    bool useNormal;
    bool useSpecular;
    bool useEmissive;
}

cbuffer OffsetMatrixCB : register(b3)
{
    matrix boneOffset[128];
}

cbuffer PoseMatrixCB : register(b4)
{
    matrix bonePose[128];
}

// ----------------------
//  Vertex Input Layout
// ----------------------
struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 texCoord : TEXCOORD;
};

struct VS_Skybox_INPUT
{
    float3 position : POSITION;
};

struct VS_Weight_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 texCoord : TEXCOORD;
    uint4 boneIndices : BONE_INDICES;
    float4 boneWeights : BONE_WEIGHTS;
};

// ----------------------
//  PS Input (VS Output)
// ----------------------
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 worldPos : WORLD_POSITION;
    float3 normal : NORMAL;
    float3x3 TBN : TBN;
    float2 texCoord : TEXCOORD;
    matrix finalWorld : FINAL_WORLD;
};

struct PS_OutLine_INPUT
{
    float4 pos : SV_POSITION;
};

struct PS_Skybox_INPUT
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0; // CubeMap »ùÇÃ¿ë ¹æÇâ º¤ÅÍ
};