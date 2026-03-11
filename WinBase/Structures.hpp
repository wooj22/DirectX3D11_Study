#pragma once
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

////////////////////////////////////////////
////////////    [ Vertex ]     /////////////
////////////////////////////////////////////

// Static / Rigid Mesh
struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
    Vector2 texcoord;

    Vertex() = default;

    Vertex(const Vector3& pos, const Vector3& n, const Vector3& t,
        const Vector3& b, const Vector2& uv)
        : position(pos), normal(n), tangent(t), bitangent(b), texcoord(uv)
    {
    }
};

// Skeletal Mesh
struct BoneWeightVertex
{
    Vector3 position;
    Vector3 normal;
    Vector3 tangent;
    Vector3 bitangent;
    Vector2 texcoord;

    // bone, weight
    // Bone의 영향을 받는 정점의 최대 개수를 4개로 제한하여 import했으므로, 배열 size는 4
    int boneIndices[4] = {};        // vertex에 영향을 주는 Bone index
    float boneWeights[4] = {};      // 각 Bone의 가중치 (합 = 1)

    void AddBoneData(int boneIndex, float weight)
    {
        assert(boneWeights[0] == 0.0f || boneWeights[1] == 0.0f ||
            boneWeights[2] == 0.0f || boneWeights[3] == 0.0f);

        for (int i = 0; i < 4; i++) {
            if (boneWeights[i] == 0.0f) {
                boneIndices[i] = boneIndex;
                boneWeights[i] = weight;
                return;
            }
        }
    }
};

// Only Position
struct Position_Vertex
{
    Vector3 position;
};

// Particle Quad
struct ParticleQuadVertex
{
    Vector2 corner; // (-0.5 ~ 0.5) : 빌보드용 로컬 오프셋
    Vector2 uv;
};

// ParticleInstance
struct ParticleInstance
{
    Vector3 pos;
    float   rotation;
    Vector2 size;
    float   frame;
    Vector4 color;
};


////////////////////////////////////////////
////////    [ ConstantBuffer ]     /////////
////////////////////////////////////////////

// Transform -> b0
struct alignas(16) TransformCB
{
    Matrix model;
    Matrix world;

    Matrix view;
    Matrix projection;
    Matrix invViewProjection;  // Deferred 화면 좌표 -> view 좌표 변환용

    Matrix shadowView;         // 광원 view
    Matrix shadowProjection;   // 광원 projection
};

// LightingCB -> b1
struct alignas(16) LightingCB
{
    int     lightType;         // 0: Directional, 1: Point, 2: Spot
    Vector3 padding;

    UINT    isSunLight = 0;    // Directional Light가 태양광인지 여부 (그림자 처리)
    Vector3 lightColor;

    float   directIntensity;
    Vector3 lightDirection;    
    
    Vector3 lightPos;         
    float   lightRange;       

    float innerAngle;         
    float outerAngle;         

    float indirectIntensity;
    UINT  useIBL = 0;
};

// MaterialCB -> b2
struct alignas(16) MaterialCB
{
    // use texture
    UINT useDiffuse;
    UINT useNormal;
    UINT useSpecular;
    UINT useEmissive;
    UINT useMetallic;
    UINT useRoughness;

    UINT roughnessFromShininess; // roughness가 aiTextureType_SHININESS로 들어오는 경우
    int  padding1;

    // BlinPong
    float ambientFactor;       
    float diffuseFactor_;       
    float specularFactor;      
    float shininess;           

    // PBR Factor
    Vector3 diffuseFactor = { 1,1,1 };
    float alphaFactor = 1.0f;
    float emissiveFactor = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    float padding2;

    // PBR override
    UINT useBaseColorOverride = 0;
    UINT useEmissiveOverride = 0;
    UINT useMetallicOverride = 0;
    UINT useRoughnessOverride = 0;

    Vector3 baseColorOverride = { 1,1,1 };
    float metallicOverride = 1.0f;
    Vector3 emissiveOverride = { 1,1,1 };
    float roughnessOverride = 1.0f;
};

// OffsetMatrix -> b3
struct alignas(16) OffsetMatrixCB
{
    // bone offset matrix
    // vertex데이터에 참조할 index 4개가 들어있음
    Matrix boneOffset[128];
};

// PoseMatrix -> b4
struct alignas(16) PoseMatrixCB
{
    // bone world matrix
    // bone의 local matrix(animation)을 계층 구조에 따라 누적한 bone transform 배열
    // vertex데이터에 참조할 index 4개가 들어있음
    Matrix bonePose[128];
};

// OutLine -> b5
struct alignas(16) OutLineCB
{
    float outlineThickness = 0.3f;
    Vector3 outlineColor = Vector3(0,0,0);
};

// Debug Constant Buffer -> b6
struct alignas(16) DebugCB
{
    UINT lightVolumeON;
    Vector3 padding;
};

// PostProcess CB -> b7
struct alignas(16) PostProcessCB
{
    // Base
    UINT    isHDR = 0;               // LDR/HDR
    UINT    useDefaultGamma = 1;     // Linear -> SRGB
    float   defaultGamma = 2.2f;     // Gamma (defalut)
    float   exposure = 0;            // 노출

    // Enable
    UINT useColorAdjustments = 0;
    UINT useWhiteBalance = 0;
    UINT useLGG = 0;
    UINT useVignette = 0;
    UINT useFilmGrain = 0;
    UINT useBloom = 0;
    Vector2 padding0;

    // Color Adjustments (대비, 채도, Hue Shift, Tint)
    float   contrast = 1;            
    float   saturation = 1;     
    UINT    useHueShift = 0;
    float   hueShift = 0;

    UINT    useColorTint = 0;             
    Vector3 colorTint = { 1,1,1 };   
    float   colorTint_strength = 0.5;
    Vector3 padding2;

    // White Balance (온도, 색조)
    float temperature = 0;
    float tint = 0;
    Vector2 padding3;

    // Lift / Gamma / Gain (어두운톤, 미드톤, 밝은톤 밝기 조정)
    UINT  useLift = 0;
    UINT  useGamma = 0;
    UINT  useGain = 0;
    int   padding4;

    Vector3 lift = { 0,0,0 };
    float   lift_strength = 0.5;
    Vector3 gamma = { 0,0,0 };
    float   gamma_strength = 1.0;
    Vector3 gain = { 0,0,0 };
    float   gain_strength = 0.5;

    // Vinette
    float   vignette_intensity = 0.5;  
    float   vignette_smoothness = 0.5; 
    Vector2 vignetteCenter = { 0.5,0.5 };
    Vector3 vignetteColor = { 0,0,0 };
    int     padding5;

    // FilmGrain
    float grain_intensity = 0.2; 
    float grain_response = 0.8;  
    float grain_scale = 1;  
    int   padding6;
};

// ScreenFx CB -> b8
struct alignas(16) ScreenFxCB
{
    int enableWaterDistortion = 0;  
    int enablePlasmaOverlay = 0;    
    int enableFilmGrain = 0;        
    int padding1;

    float cellScale = 3.0f;                  // Noise Cell Scale         
    float randomIntensity = 43758.5453f;     // Random 해시 분산 강도
    float warpStrength = 0.8f;               // Domain Warping 강도  
    float distortionStrength = 0.008f;       // UV 왜곡 강도

    float plasmaIntensity = 0.35;       
    float grainIntensity = 0.05f; 
    Vector2 padding2;
};

// Bloom CB -> b9
struct alignas(16) BloomCB
{
    float bloom_threshold = 1.0f; 
    float bloom_intensity = 0.8f; 
    float bloom_scatter = 0.5f;
    float bloom_clamp = 0.0f;

    Vector3 bloom_tint = { 1.0f, 1.0f, 1.0f };
    int     padding; 
    
    int     srcMip = 0;                      // SampleLevel용 mip 인덱스
    Vector2 srcTexelSize = { 0.0f, 0.0f };   // 패스에서 읽고 있는 mip 레벨의 텍스처 해상도를 기준으로 한 texel size
    int     padding2;
};

// Frame CB -> b10
struct alignas(16) FrameCB
{
    float    time;
    float    deltaTime;
    Vector2  padding1;

    Vector2 screenSize;
    Vector2 shadowMapSize = { 8192,8192 };      // 이렇게 생성 고정함

    Vector3 cameraPos;
    int padding2;
};

// Effect CB -> b11
struct alignas(16) EffectCB
{
    Vector2 atlasGrid;      // (cols, rows)
    Vector2 invAtlasGrid;   // (1/cols, 1/rows)

    float baseSizeScale = 1.0f;
    int billboardType;
    Vector2 padding;
};

// Decal CB -> b12
struct alignas(16) DecalCB
{
    Matrix decalInvWorld;       // decal world->local 변환용 역행렬 (박스 내부 판정 + uv 생성)

    Vector2 tiling;
    Vector2 offset;

    float opacity;
    float upThreshold;
    int   decalType;      // 0=TextureMap, 1=RingEffect
    float pad0;

    // ring
    float ringStartTime;
    float ringDuration;
    float ringMaxRadius;
    float ringSpeed;

    float ringThickness;
    float ringFeather;
    float pad1;

    Vector3 ringColor;
    float pad2;
};