#pragma once
#include "IRenderer.h"
#include <vector>
#include <directxtk/simplemath.h>
using namespace std;
using namespace DirectX::SimpleMath;

/*
    [ Geometry Renderer ]

     Geometry를 그리기 위한 Stage Setting을 하고,
     Deferred Rendering을 위해 G-buffer에 라이팅에 필요한 정보를 기록합니다.


     ** Geometry Renderer의 Renderable 객체 **
     StaticModel, RigidModel, SkeletalModel

     
     ** G-buffer **
      RT0 : Albedo (RGB)
      RT1 : Normal (RGB)
      RT2 : Metallic (R), Roughness (G)
      RT3 : Emissive (RGB)
      ★ Position은 대역폭 절약을 위해 G-buffer에 저장하지 않고,
        Geometry Pass에서 사용한 뎁스 버퍼를 이용해 Position을 복원해 사용합니다.
*/

class StaticModel;
class RigidModel;
class SkeletalModel;

class GeometryRenderer : public IRenderer
{
private:
    // g-buffer clear color
    float clearColor[4] = { 0,0,0,1 };

public:
    // interface (확장성)
    ~GeometryRenderer() override = default;
    void Initialize() override {};
    void RenderPass() override {};

    // non interface function (Legucy)
    void Init() {};
    void GeometryPass(const Matrix& view, const Matrix& projection,
        const vector<StaticModel*>& static_models,
        const vector<RigidModel*>& rigid_models,
        const vector<SkeletalModel*>& skeletal_models);
};

