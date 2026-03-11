#pragma once
#include "Renderable.h"
#include "SkeletalModelAsset.hpp"
#include <iostream>
#include <string>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;
using namespace std;

class Camera;
class DirectionalLight;

/*
* [ Skeletal Model ]
*
*
*/

class SkeletalModel : public Renderable
{
public:
    /*---- [Model Asset] ----*/         // submeshs, skeleton, materials, animation clips
    shared_ptr<SkeletalModelAsset> model_data;

    /*--- [Instance Data] ---*/
    // model transform
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
    Matrix world;

    // bone transform
    vector<Matrix> localMatrix;      // bone animation
    vector<Matrix> poseMatrix;       // bone parent * local

    // animation
    float currentAnimTime = 0.f;

public:
    SkeletalModel();
    SkeletalModel(Vector3 p, Vector3 r, Vector3 s);
    ~SkeletalModel() override = default;

    void InitTransform();
    void SetTransform(Vector3 p, Vector3 r, Vector3 s);
    void SetPosition(Vector3 p);
    void SetRotation(Vector3 r);
    void SetScale(Vector3 s);
    void MakeWorld();

    void Update();
    void Draw();
};

