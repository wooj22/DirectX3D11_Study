#pragma once
#include <vector>
#include <directxtk/simplemath.h>
using namespace std;
using namespace DirectX::SimpleMath;

/*
    [ Shadow Renderer ]

    ShadowMap 생성을 위한 Stage를 맞추고, 전달받은 model들로 depth only pass를 진행합니다.
    Light 시점에서 바라본 Scene을 렌더링하며 Depth(ShadowMap)을 기록합니다.


    ** Shadow Renderer의 Renderable 객체 **
     StaticModel, RigidModel, SkeletalModel
*/

class StaticModel;
class RigidModel;
class SkeletalModel;

class ShadowRenderer
{
public:
    void Init() {};
    void ShadowMapPass(const Matrix& view, const Matrix& projection, 
        const vector<StaticModel*>& static_models,
        const vector<RigidModel*>& rigid_models,
        const vector<SkeletalModel*>& skeletal_models);
};

