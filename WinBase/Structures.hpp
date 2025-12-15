#pragma once
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

////////////////////////////////////////////
////////////    [ Vertex ]     /////////////
////////////////////////////////////////////

// 2D FullscreenVertex
//struct FullscreenVertex
//{
//    Vector2 position;
//    Vector2 uv;
//};

// Default Model
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

// SkyBox
struct Skybox_Vertex
{
    Vector3 position;
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
    Matrix shadowView;       // 광원 view
    Matrix shadowProjection; // 광원 projection
};

// LightingCB -> b1
struct alignas(16) LightingCB
{
    Vector4 lightDirection;
    Vector4 lightColor;

    float directIntensity;            // blinpong, PBR
    float indirectIntensity;          // blinpong, PBR

    float ambientHighlight;       // blinpong  // TODO :: debug CB로 옮기기
    float diffuseHighlight;       // blinpong
    float specularHighlight;      // blinpong
    float shininess;              // blinpong
    Vector2 padding;

    Vector3 cameraPos;
    float padding2;
};

// MaterialCB -> b2
struct alignas(16) MaterialCB
{
    UINT useDiffuse;
    UINT useNormal;
    UINT useSpecular;
    UINT useEmissive;
    UINT useMetallic;
    UINT useRoughness;
    Vector2 padding;
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
    // factor
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;

    // override
    float metallicOverride = 1.0f;
    float roughnessOverride = 1.0f;
    UINT useMetallicOverride = 0;
    UINT useRoughnessOverride = 0;

    UINT useBaseColorOverride = 0;
    UINT useIBL = 0;

    Vector3 baseColorOverride = { 1,1,1 };
    float padding2;
};

// PostProcess CB
struct alignas(16) PostProcessCB
{
    // gamma
    UINT    useGamma = 1;            // LDR 감마보정 (PBR_PS)
    UINT    useTint = 0;             // Color Tint
    float   exposure = 1.0f;         // 노출
    float   padding1;

    Vector3 hueShift = { 0,0,0 };       // 색상 이동
    float   contrast = 1;               // 대비
    float   saturation = 1;             // 채도
    Vector3 colorTint = { 0,0,0 };      // 색상 톤
};
