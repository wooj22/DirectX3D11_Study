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

/* Skybox 구현 방법 */
// 1. 큐브 메시와 큐브맵 DDS를 준비한다. 크기는 상관 없다.
// 2. view 행렬에서 이동 성분을 제거하여 스카이박스를 카메라 원점에 둔다.
// 3. Skybox는 화면이 클리어되고 가장 먼저 그려져야 한다. 
// 4. Skybox는 항상 가장 뒤에 있어야 하기 때문에 D3D11_DEPTH_WRITE_MASK_ZERO인 DepthStencilState를 바인딩해준다.
// 5. 래스터 라이저의 CullMode를 D3D11_CULL_FRONT로 하여 안쪽면이 보이게 그려준다. 만약 메쉬 자체가 뒤집혀있다면 생략 가능하다.
// 6. skybox vertex local position에 이동 성분을 제거한 view행렬을 곱하고, projection한다.
// 7. 큐브맵 텍스처를 샘플링하여 렌더 과정을 마친다.
// Skybox를 그린 뒤, DepthStencilState와 RasterizerState를 다시 원상 복귀 시켜줘야 다른 오브젝트들이 올바르게 렌더링된다.

class SkyBox
{
    ID3D11Buffer* vertexBuffer = nullptr;
    ID3D11Buffer* indexBuffer = nullptr;
    ID3D11ShaderResourceView* skyboxTRV = nullptr;

    UINT vertexBufferStride = 0;
    UINT vertexBufferOffset = 0;
    UINT indexCount = 0;

public:
    void InitRenderPipeLine(const std::wstring& filePath);
    void Render(const Matrix& view, const Matrix& projection) const;
    void UninitRenderPipeLine();
};

