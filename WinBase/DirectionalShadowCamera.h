#pragma once
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;
using namespace DirectX;
class Camera;

// 직교행렬
struct ShadowOrthoDesc
{
    float lookPointDist = 500.f;
    float shadowLightDist = 1000.f;
    float shadowWidth = 3000.0f;
    float shadowHeight = 3000.0f;
    float shadowNear = 0.01f;
    float shadowFar = 5000.0f;
};


/*
    [ DirectionalShadowCamera ]

     Directional 라이트에 대해 ShadowMap을 형성하기 위한
     Frustum의 matrix를 계산하는 Class 모듈입니다.
*/

class DirectionalShadowCamera
{
private:
    Matrix view;
    Matrix projection; 

public:
    void Udate(Camera& camera, const Vector3 lightDir, const ShadowOrthoDesc& desc);
    const Matrix& GetView() const { return view; }
    const Matrix& GetProjection() const { return projection; }
};

