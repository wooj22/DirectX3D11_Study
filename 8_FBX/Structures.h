#pragma once
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Vertex (Static, Rigid Skeletal)
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

// Vertex (Skinned Skeletal)
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

        for (int i = 0; i < 4; i++){
            if (boneWeights[i] == 0.0f){
                boneIndices[i] = boneIndex;
                boneWeights[i] = weight;
                return;
            }
        }
    }
};

// ConstantBuffer
struct alignas(16) ConstantBuffer
{
    // transform
    Matrix model;
	Matrix world;
	Matrix view;
	Matrix projection;

    // lighting
	Vector4 lightDirection;
	Vector4 lightColor;

	float indirectLight;
	float directLight;
	float ambientHighlight;
	float diffuseHighlight;

	float specularHighlight;
	float shininess;
	Vector2 padding1;

	Vector3 cameraPos;
    float padding2;

	UINT useDiffuse;
	UINT useNormal;
	UINT useSpecular;
	UINT useEmissive;
};

struct alignas(16) OffsetMatrixCB
{
    // bone offset matrix
    // submesh render시에 해당 mesh에 영향을 주고있는 모든 bone의 offset matrix 전달
    // vertex데이터에 참조할 index 4개가 들어있음
    Matrix boneOffset[128];
};