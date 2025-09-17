#pragma once
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
struct Vertex
{
	Vector3 position;
	Vector3 normal;	
	Vector2 texcoord;

	Vertex(Vector3 position, Vector3 normal, Vector2 uv)
		: position(position), normal(normal), texcoord(uv) {
	}
};

// ConstantBuffer
// vertex shdaer, pixel shader ����
struct alignas(16) ConstantBuffer
{
	Matrix world;					// world ���
	Matrix view;		 			// view ���
	Matrix projection;				// projection ���

	Vector4 lightDirection;         // directional light�� ���� ����
	Vector4 lightColor;				// directional light ����
};

class DirectionalLight;

class Cube
{
public:
	// transform
	Vector3 position;
	Vector3 rotation;	
	Vector3 scale;		  
	Matrix world;		

	// renderpipeline
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* indexBuffer = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11ShaderResourceView* textureRV = nullptr;

	// vertex info
	UINT vertexBufferStride = 0;
	UINT vertexBufferOffset = 0;
	UINT indexCount = 0;

	Cube()
	{
		position = Vector3::Zero;
		rotation = Vector3::Zero;
		scale = Vector3::One;
		world = XMMatrixIdentity();
	}

	Cube(Vector3 p, Vector3 r, Vector3 s)
	{
		position = p;
		rotation = r;
		scale = s;

		Matrix tm = XMMatrixTranslationFromVector(p);
		XMVECTOR q = XMQuaternionRotationRollPitchYaw(r.x, r.y, r.z);
		Matrix rm = XMMatrixRotationQuaternion(q);
		Matrix sm = XMMatrixScalingFromVector(s);
		world = sm * rm * tm;
	}

	void InitTransform()
	{
		position = Vector3::Zero;
		rotation = Vector3::Zero;
		scale = Vector3::One;
		world = XMMatrixIdentity();
	}
	void SetTransform(Vector3 p, Vector3 r, Vector3 s)
	{
		position = p;
		rotation = r;
		scale = s;

		Matrix tm = XMMatrixTranslationFromVector(p);
		XMVECTOR q = XMQuaternionRotationRollPitchYaw(r.x, r.y, r.z);
		Matrix rm = XMMatrixRotationQuaternion(q);
		Matrix sm = XMMatrixScalingFromVector(s);
		world = sm * rm * tm;
	}

	void InitRenderPipeLine();
	void Update();
	void Render(Matrix& view, Matrix& projection, DirectionalLight& light);
	void UninitRenderPipeLine();
};