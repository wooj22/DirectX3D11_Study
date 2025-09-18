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
// vertexbuffer, input layout, vertex shader ����
struct Skybox_Vertex
{
	Vector3 position;
};

// ConstantBuffer
// vertex shdaer, pixel shader ����
struct alignas(16) Skybox_ConstantBuffer
{
	Matrix world;					// world ���
	Matrix view;		 			// view ���
	Matrix projection;				// projection ���
};

/* Skybox ���� ���� */
// 1. ť�긦 ���� �� ī�޶� ��ġ�� ���� ť�긦 �̵���Ų��. (ũ��� ��� x)
// 2. view ��Ŀ��� translation�� �����Ѵ�. (�̹� ī�޶� �߽����� �̵����ױ� ������ vs���� �ߺ� �������� �ʵ���)
// 3. CubeMap �ؽ�ó�� �̿��� uv��ǥ ��� ���� ���͸� �̿��� ���ø��Ѵ�.
// 4. �߿�! Skybox�� �׻� ���� �ڿ� �־�� �ϱ� ������ Depth test�� LESS_EQUAL, Depth=1.0f�� �����Ѵ�.
// Skybox�� ȭ���� Ŭ����ǰ� ���� ���� �׷����� �Ѵ�. 
// LESS_EQUAL���� Skybox�� �׸� ��, �ٽ� LESS�� �����־�� �ٸ� ������Ʈ���� ���� �׽�Ʈ�� �ùٸ��� ����ȴ�.
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

