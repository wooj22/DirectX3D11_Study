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
* [ Rigid Skeletal Mesh ]
* 리깅 없는 단순 Transform 애니메이션이 있는 메시.
* 애니메이션은 aiNode 기준으로 매핑되어있기 때문에 모델 로드시 nodename을 저장하고, 해당 값으로 애니메이션을 찾는다.
* 계층 구조를 유지해야하기 때문에 각 메시는 LocalSpace기준으로 저장하고, 매 연산시에 Model Space를 변환해야한다.
* 각 서브메시에 애니메이션 키프레임값을 보간하여 local matrix를 만들고, 부모의 model matrix를 곱하여 model matrix를 만든다.
* vertex_world = world_matrix * modelMatrix * vertex_local(기본저장값, animation 적용)
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
	float currentAnimTime = 0.f;

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

