#pragma once
#include "Skybox.h"
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include "../D3DBase/D3DBase.h"
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

// Vertex Structure
// vertexbuffer, input layout, vertex shader 연결
struct Skybox_Vertex
{
	Vector3 position;
};

// ConstantBuffer
// vertex shdaer, pixel shader 연결
struct alignas(16) Skybox_ConstantBuffer
{
	Matrix world;					// world 행렬
	Matrix view;		 			// view 행렬
	Matrix projection;				// projection 행렬
};

/* Skybox 구현 원리 */
// 1. 큐브를 만든 후 카메라 위치에 맞춰 큐브를 이동시킨다. (크기는 상관 x)
// 2. view 행렬에서 translation을 제거한다. (이미 카메라 중심으로 이동시켰기 때문에 vs에서 중복 연산하지 않도록)
// 3. CubeMap 텍스처를 이용해 uv좌표 대신 방향 벡터를 이용해 샘플링한다.
// 4. 중요! Skybox는 항상 가장 뒤에 있어야 하기 때문에 Depth test를 LESS_EQUAL, Depth=1.0f로 고정한다.
// Skybox는 화면이 클리어되고 가장 먼저 그려져야 한다. 
// LESS_EQUAL모드로 Skybox를 그린 뒤, 다시 LESS로 돌려주어야 다른 오브젝트들의 깊이 테스트가 올바르게 진행된다.
class Skybox
{
public:
	// renderpipeline
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D11VertexShader* skyboxVS = nullptr;
	ID3D11PixelShader* skyboxPS = nullptr;
	ID3D11ShaderResourceView* skyboxTRV = nullptr;
	ID3D11DepthStencilState* skyboxDSS = nullptr;

	// vertex info
	UINT vertexBufferStride = 0;
	UINT vertexBufferOffset = 0;
	UINT indexCount = 0;

	void InitRenderPipeLine();
	void Render(Matrix& view, Matrix& projection);
	void UninitRenderPipeLine();
};

