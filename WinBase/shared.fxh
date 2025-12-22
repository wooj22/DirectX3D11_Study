
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
// DebugCB -> b6
// PostProcessCB -> b7
// ScreenFxCB -> b8
// BloomCB -> b9

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
// Texture2D IBL_IrradianceMap : register(t9);
// Texture2D IBL_SpecularEnvMap : register(t10);
// Texture2D IBL_BRDF_LUT : register(t11);
// Texture2D sceneHDR : register(t12);

// [ SamplerState ]
// SamplerState samLinear : register(s0)
// SamplerComparisonState samShadow : register(s1);
// SamplerState samLinearClamp : register(s2);


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
    
    float directIntensity; // blinpong, PBR
    float indirectIntensity; // blinpong, PBR
   
    float ambientHighlight;         // blinpong
    float diffuseHighlight;         // blinpong
    float specularHighlight;        // blinpong
    float shininess;                // blinpong
    float2 padding1;
    
    float3 cameraPos;
    float padding2;
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
    float metallicFactor;
    float roughnessFactor;
    float metallicOverride;
    float roughnessOverride;
    
    bool useMetallicOverride;
    bool useRoughnessOverride;
    
    bool useBaseColorOverride ;
    bool useIBL;
    float3 baseColorOverride;
    float padding3;
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
    bool useVignette;
    bool   useFilmGrain;
    float3 padding0;
    
    // Color Adjustments (´ëºñ, Ã¤µµ, Hue Shift, Tint)
    float  contrast;
    float  saturation; 
    bool   useHueShift;
    float  hueShift;
    
    bool   useTint;
    float3 colorTint;
    float  colorTint_strength;
    float3 padding4;
    
    // White Balance (¿Âµµ, »öÁ¶)
    float temperature;
    float tint;
    float2 padding5;
    
    // Lift / Gamma / Gain
    bool useLift;
    bool useGamma;
    bool useGain;
    float padding6;
    
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
    int    padding7;
    
    // FilmGrain
    float grain_intensity;
    float grain_response;
    float grain_scale;
    int   padding8;
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
    int padding9;
    
    int srcMip;
    float2 srcTexelSize;
    int padding10;
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

struct PS_FullScreen_Input
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};




