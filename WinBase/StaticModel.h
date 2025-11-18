#pragma once
#include <iostream>
#include "StaticModelAsset.hpp"
#include <string>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;
using namespace std;

class Camera;
class DirectionalLight;


/*
* [ Static Model ]
* StaticSubMesh와 Material을 가지는 애니메이션 없는 정적 모델
* 각 StaticSubMesh는 Model Space기준으로 저장되어있음
*
* vertex_world = world_matrix * vertex_model(기본저장값)
*/

class StaticModel
{
public:
    // sub mesh, material
    vector<StaticSubMesh> subMeshes;
    vector<Material> materials;

    // 공유 리소스
    // TODO :: StaticModelAsset 구조로 변경
    shared_ptr<StaticModelAsset> model;

    // 인스턴스 데이터
    // transform
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Matrix world;

public:
    StaticModel();
    StaticModel(Vector3 p, Vector3 r, Vector3 s);
    void InitTransform();
    void SetTransform(Vector3 p, Vector3 r, Vector3 s);
    void SetPosition(Vector3 p);
    void SetRotation(Vector3 r);
    void SetScale(Vector3 s);
    void MakeWorld();

    void Update();
    void Render();
};

