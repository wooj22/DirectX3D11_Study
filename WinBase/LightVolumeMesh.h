#pragma once
#define NOMINMAX

#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include <d3d11.h>
#include <wrl/client.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

struct Light;
struct Position_Vertex;
enum class LightVolumeType;

/*
    [Light Volume Mesh]

    Deferred Rendering의 멀티 라이트 처리(Point, Spot)시에
    라이팅 연산 영역을 필터링 하기 위해 사용하는 Mesh
    - Stencil Pass를 통해 라이팅 연산 후보 픽셀을 마킹하고
    - Lighting Pass에서 Stencil Test를 통해 해당 픽셀들에 대해서만 라이팅을 계산한다.
*/

class LightVolumeMesh
{
public:  
    // volume type
    LightVolumeType volumeType;

    // vertex, index
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    UINT indexCount;
    UINT stride;
    DXGI_FORMAT indexFormat;

    // matrix
    Matrix world;

public:
    LightVolumeMesh();
    void UpdateWolrd(Light& light);
    void Draw() const;
};

// 외부 Create Functions
LightVolumeMesh* CreateLightVolumeSphere(ID3D11Device* device, int slices = 24, int stacks = 16);
LightVolumeMesh* CreateLightVolumeCone(ID3D11Device* device, int slices = 24, bool capBase = false);


