#pragma once
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include "D3D.h"
#include <string>
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

// [Decal Type]
enum class DecalType
{
    TextureMap = 0,
    RingEffect = 1
};

/*
    [ Decal ]

    일단 Ground Decal (Stencil Masked)는 안넣음. 오브젝트에 isGround 변수 추가하고 DSS 비트 분리해야됨
*/

class Decal
{
public:
    DecalType type = DecalType::TextureMap;    // TextureMap, RingEffect

    // transform
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    Vector3 scale = { 1.0f, 1.0f, 1.0f };

    // opacity
    float opacity = 1.0f;

    // ground decal
    bool isGroundDecal = false;
    float upThreshold = 0.9f;

    // decal texture
    Vector2 tiling = { 1.0f, 1.0f };
    Vector2 offset = { 0.0f, 0.0f };
    ComPtr<ID3D11ShaderResourceView> decalSRV = nullptr;

    // ring effect
    float ringStartTime = 0.0f;     // 스폰 시점
    float ringDuration = 10.0f;     // 링 수명
    float ringMaxRadius = 0.95f;    // 0~1(uv공간 반경)
    float ringSpeed = 0.45f;        // Speed
    float ringThickness = 0.005f;   // 링 두께 (uv공간)
    float ringFeather = 0.01f;      // 링 가장자리 소프트
    Vector3 ringColor = { 0.0f, 0.0f, 1.0f };


public:
    // functions
    void StartRingEffect(float startTime, float duration, float speed);
};

