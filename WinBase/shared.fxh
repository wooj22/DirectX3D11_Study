
// ------------------
//  바인딩 슬롯
// ------------------
// [ Constant Buffer ]
// TransformCB      -> b0
// LightingCB       -> b1
// MaterialCB       -> b2
// OffsetMatrixCB   -> b3
// PoseMatrixCB     -> b4
// OutLineCB        -> b5
// DebugCB          -> b6
// PostProcessCB    -> b7
// ScreenFxCB       -> b8
// BloomCB          -> b9

// [ Texture ]
// Texture2D diffuseMap          : register(t0);
// Texture2D normalMap           : register(t1);
// Texture2D specularMap         : register(t2);
// Texture2D emissiveMap         : register(t3);
// TextureCube skyboxTexture     : register(t4);
// Texture2D diffuseRamp         : register(t4);
// Texture2D specualrRamp        : register(t5);
// Texture2D shadowMap           : register(t6);
// Texture2D metallicMap         : register(t7);
// Texture2D roughnessMap        : register(t8);
// Texture2D IBL_IrradianceMap   : register(t9);
// Texture2D IBL_SpecularEnvMap  : register(t10);
// Texture2D IBL_BRDF_LUT        : register(t11);
// Texture2D sceneHDR            : register(t12);
// Texture2D bloomA              : register(t13);
// Texture2D positionTex         : register(t14);       // 사용 x (대역폭 문제로 Depth Buffer 사용)
// Texture2D albedoTex           : register(t15);
// Texture2D normalTex           : register(t16);
// Texture2D metalRoughTex       : register(t17);
// Texture2D emissiveTex         : register(t18);
// Texture2D depthTex            : register(t19);

// [ SamplerState ]
// SamplerState samLinear           : register(s0);
// SamplerComparisonState samShadow : register(s1);
// SamplerState samLinearClamp      : register(s2);


#ifndef Shared
#define Shared

// ------------------
//  Constant Buffer
// ------------------
cbuffer TransformCB : register(b0)
{
    matrix model;
    matrix world;
    matrix view;
    matrix projection;
    matrix shadowView;       
    matrix shadowProjection; 
    
    float3 cameraPos;
    int padding1;
    
    matrix inverseProjection;
}

cbuffer LightingCB : register(b1)
{
    float4 lightDirection;
    float4 lightColor;
    
    float directIntensity;   
    float indirectIntensity; 
   
    bool useIBL;
    int padding2;
}

cbuffer MaterialCB : register(b2)
{
    // use texture
    bool useDiffuse;
    bool useNormal;
    bool useSpecular;
    bool useEmissive;
    bool useMetallic;
    bool useRoughness;
    
    bool roughnessFromShininess;
    float padding3;
    
    // BlinPong
    float ambientFactor;
    float diffuseFactor;
    float specularFactor;
    float shininess;
    
    // PBR Factor
    float emissiveFactor;
    float metallicFactor;
    float roughnessFactor;
    float padding4;

    // PBR override
    bool useBaseColorOverride;
    bool useEmissiveOverride;
    bool useMetallicOverride;
    bool useRoughnessOverride;

    float3 baseColorOverride;
    float metallicOverride;
    float3 emissiveOverride;
    float roughnessOverride;
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
    
}

cbuffer PostProcessCB : register(b7)
{
    // Base
    bool  isHDR;
    bool useDefaultGamma;
    float defalutGamma;
    float exposure;
    
    // Enable
    bool   useColorAdjustments;
    bool   useWhiteBalance;
    bool   useLGG;
    bool   useVignette;
    bool   useFilmGrain;
    bool   useBloom;
    float2 padding0;
    
    // Color Adjustments (대비, 채도, Hue Shift, Tint)
    float  contrast;
    float  saturation; 
    bool   useHueShift;
    float  hueShift;
    
    bool   useTint;
    float3 colorTint;
    float  colorTint_strength;
    float3 padding6;
    
    // White Balance (온도, 색조)
    float temperature;
    float tint;
    float2 padding7;
    
    // Lift / Gamma / Gain
    bool useLift;
    bool useGamma;
    bool useGain;
    float padding8;
    
    float3 lift;
    float  lift_strength;

    float3 gamma;
    float  gamma_strength;

    float3 gain ;
    float  gain_strength;
    
    // Vinette
    float  vignette_intensity;
    float  vignette_smoothness;
    float2 vignetteCenter;
    float3 vignetteColor;
    int    padding9;
    
    // FilmGrain
    float grain_intensity;
    float grain_response;
    float grain_scale;
    int   padding10;
};

cbuffer ScreenFxCB : register(b8)
{
    int enableRipple;
    int enablePlasmaOverlay;  
    int enableFilmGrain;      
    float time;
    
    float cellScale; 
    float randomIntensity;
    float warpStrength; 
    float distortionStrength; 
    
    float plasmaIntensity; 
    float grainIntensity; 
    float2 screenTexelSize;
}

cbuffer BloomCB : register(b9)
{
    float bloom_threshold;
    float bloom_intensity;
    float bloom_scatter;
    float bloom_clamp;

    float3 bloom_tint;
    int padding11;
    
    int srcMip;
    float2 srcTexelSize;
    int padding12;
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
    float3 texCoord : TEXCOORD0; // CubeMap 샘플용 방향 벡터
};

struct PS_FullScreen_Input
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

// ------------------------------------
//  PS Output (Deferred Rendering)
// ------------------------------------
struct PS_Output
{
    //float4 WorldPos   : SV_Target0;       // Position 대신 Depth Buffer 사용
    float4 Base_color : SV_Target0;
    float4 Normal     : SV_Target1;
    float4 Material   : SV_Target2;
    float4 Emissive   : SV_Target3;
};




// Deferred Rendering Normal Gbuffer 
float3 DecodeNormal(float3 enc)
{
    // enc is in [0,1], decode to [-1,1]
    return normalize(enc * 2.0f - 1.0f);
}

float3 EncodeNormal(float3 n)
{
    // n is in [-1,1], encode to [0,1]
    return n * 0.5f + 0.5f;
}


#endif


