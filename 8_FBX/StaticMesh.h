#pragma once
#include "StaticSubMesh.h"
#include "Material.h"
#include <string>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;
using namespace std;

class Camera;
class DirectionalLight;

// ConstantBuffer
struct alignas(16) ConstantBuffer
{
	Matrix world;
	Matrix view;
	Matrix projection;

	Vector4 lightDirection;
	Vector4 lightColor;

	float indirectLight;
	float directLight;

	float ambientReflection;
	float diffuseReflection;
	float specularReflection;
	float shininess;
	Vector2 padding1;

	Vector3 cameraPos;
	float padding2;
};

/*
* [ Static Mesh ]
* StaticSubMesh와 Material을 트리구조로 가지는 정적 메시 모델
* 각 StaticSubMesh는 StaticMesh의 world를 기준으로 변환하여 각자 렌더링함
*/
class StaticMesh
{
public:
	// sub mesh, material
	int subMeshNum;
	vector<StaticSubMesh> subMeshes;
	vector<Material> materials;

	// transform
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
	Matrix world;

public:
	StaticMesh();
	StaticMesh(Vector3 p, Vector3 r, Vector3 s);
	~StaticMesh();
	void InitTransform();
	void SetTransform(Vector3 p, Vector3 r, Vector3 s);

	void Update();
	void Render(ID3D11Buffer* constantBuffer, ConstantBuffer& cb);
};

