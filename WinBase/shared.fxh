
// ------------------
//  ¹ÙÀÎµù ½½·Ô
// ------------------
// [ Constant Buffer ]
// TransformCB -> b0
// LightingCB -> b1
// MaterialCB -> b2
// OffsetMatrixCB -> b3
// PoseMatrixCB -> b4
// OutLineCB -> b5

// [ Texture ]
// Texture2D diffuseMap : register(t0)
// Texture2D normalMap : register(t1)
// Texture2D specularMap : register(t2)
// Texture2D emissiveMap : register(t3)
// TextureCube skyboxTexture : register(t4)
// Texture2D diffuseRamp : register(t4);
// Texture2D specualrRamp : register(t5);
// Texture2D shadowMap : register(t6);
// Texture2D metallicMap : register(t7);
// Texture2D roughnessMap : register(t8);


// [ SamplerState ]
// SamplerState samLinear : register(s0)
// SamplerComparisonState samShadow : register(s1);


// ------------------
//  Constant Buffer
// ------------------
cbuffer TransformCB : register(b0)
{
    matrix model;
    matrix world;
    matrix view;
    matrix projection;
    matrix shadowView;       // ±¤¿ø view
    matrix shadowProjection; // ±¤¿ø projection
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
    bool useMetallic;
    bool useRoughness;
    float2 padding;
}

cbuffer OffsetMatrixCB : register(b3)
{
    matrix boneOffset[128];
}

cbuffer PoseMatrixCB : register(b4)
{
    matrix bonePose[128];
}

cbuffer OutLineCB : register(b5)
{
    float outlineThickness;
    float3 outlineColor;
}

cbuffer DebugCB : register(b6)
{
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    float metallicOverride = 1.0f;
    float roughnessOverride = 1.0f;
    
    bool useMetallicOverride = false;
    bool useRoughnessOverride = false;
    float2 padding2;
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

struct VS_Weight_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 texCoord : TEXCOORD;
    uint4  boneIndices : BONE_INDICES;
    float4 boneWeights : BONE_WEIGHTS;
};

struct VS_Skybox_INPUT
{
    float3 position : POSITION;
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
    float4 posShadow : TEXCOORD1;
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