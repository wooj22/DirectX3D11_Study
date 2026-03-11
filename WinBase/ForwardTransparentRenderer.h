#pragma once
#include "IRenderer.h"
#include <vector>
#include <directxtk/simplemath.h>
using namespace std;
using namespace DirectX::SimpleMath;


/*
    [ Forward Transparent Renderer ]

     투명 오브젝트 렌더링을 위한 Forward Rendering을 진행하는 렌더러입니다.
     지오메트리를 그리며 라이팅을 계산하고, 알파블렌딩을 수행합니다.

     ** Forward Transparent Renderer의 Renderable 객체 **
     Model(Transparent)
*/

class StaticModel;
class RigidModel;
class SkeletalModel;
class Light;
class Environment;
class Camera;

class ForwardTransparentRenderer : public IRenderer
{
public:
    // interface (확장성)
    ~ForwardTransparentRenderer() override = default;
    void Initialize() override {};
    void RenderPass() override {};

    // non interface function (Legucy)
    void Init() {}
    void ForwardTransparentPass(const Matrix& view, const Matrix& projection,
        const vector<StaticModel*>& static_models,
        const vector<RigidModel*>& rigid_models,
        const vector<SkeletalModel*>& skeletal_models,
        const std::vector<Light>& lights, const Environment& env);
};

