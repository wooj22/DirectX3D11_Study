#pragma once
#define NOMINMAX
#include "Renderable.h"
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include <d3d11.h>
#include <wrl/client.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

class Camera;
struct Position_Vertex;
class Decal;


/*
    [Decal Volume Mesh]

    Decal Rendering을 위해 box volume을 그리며
    그 영역에 decal을 그리기 위해 사용하는 Mesh
*/

class DecalVolumeMesh : public Renderable
{
private:
    // vertex, index
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    UINT indexCount;
    UINT stride;
    DXGI_FORMAT indexFormat;

    // matrix
    Matrix world;

public:
    DecalVolumeMesh();
    ~DecalVolumeMesh() override = default;

    void UpdateWolrd(const Decal* decal);
    void Draw() const;

    // friend
    friend DecalVolumeMesh* CreateDecalVolume(ID3D11Device* device);
};

// 외부 Create Functions
DecalVolumeMesh* CreateDecalVolume(ID3D11Device* device);