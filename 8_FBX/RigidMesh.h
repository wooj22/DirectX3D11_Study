#pragma once
#include "RigidSubMesh.h"
#include "Material.h"
#include "AnimationClip.h"
#include "Structures.h"
#include <string>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;
using namespace std;

class Camera;
class DirectionalLight;

/*
* [ Rigid Mesh ]
* 
* vertex_world = world_matrix * modelMatrix * animMatrix * vertex_local(기본저장값)
*/

class RigidMesh
{
public:
	// sub mesh, material, animation
	vector<RigidSubMesh> subMeshes;
	vector<Material> materials;
	vector<AnimationClip> animationClips;

	// transform
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
	Matrix world;

	// animation
	

public:
	RigidMesh();
	RigidMesh(Vector3 p, Vector3 r, Vector3 s);
	void InitTransform();
	void SetTransform(Vector3 p, Vector3 r, Vector3 s);
	void SetPosition(Vector3 p);
	void SetRotation(Vector3 r);
	void SetScale(Vector3 s);
	void MakeWorld();

	void Update();
	void Render(ID3D11Buffer* constantBuffer, ConstantBuffer& cb);
};

