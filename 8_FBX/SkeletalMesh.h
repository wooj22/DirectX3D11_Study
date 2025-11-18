#pragma once
#include "SkeletalSubMesh.h"
#include "Material.h"
#include "AnimationClip.h"
#include "Structures.h"
#include "Skeleton.h"
#include <string>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;
using namespace std;

class Camera;
class DirectionalLight;

/*
* [ Skeletal Mesh ]
* 
* 
*/

class SkeletalModel
{
public:
	// sub mesh, material, animation
	vector<SkeletalSubMesh> subMeshes;
	vector<Material> materials;
    Skeleton skeleton;
	vector<AnimationClip> animationClips;

	// transform
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
	Matrix world;

	// animation
	float currentAnimTime = 0.f;

public:
	SkeletalModel();
	SkeletalModel(Vector3 p, Vector3 r, Vector3 s);
	void InitTransform();
	void SetTransform(Vector3 p, Vector3 r, Vector3 s);
	void SetPosition(Vector3 p);
	void SetRotation(Vector3 r);
	void SetScale(Vector3 s);
	void MakeWorld();

	void Update();
	void Render(ID3D11Buffer* constantBuffer, ID3D11Buffer* offsetMatrixCB, ID3D11Buffer* poseMatrixCB, 
        ConstantBuffer& cb, OffsetMatrixCB& offsetCB, PoseMatrixCB& poseCB);
};

