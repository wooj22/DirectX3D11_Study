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
struct Cube_Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector2 texcoord;

	Cube_Vertex(Vector3 position, Vector3 normal, Vector2 uv)
		: position(position), normal(normal), texcoord(uv) { }
};

// ConstantBuffer
struct alignas(16) ConstantBuffer
{
	Matrix world;					// world 행렬
	Matrix view;		 			// view 행렬
	Matrix projection;				// projection 행렬

	Vector4 lightDirection;         // directional light의 방향
	Vector4 lightColor;				// directional light 색상

	float indirectLight;			// 간접광
	float directLight;				// 직접광

	float ambientHighlight;		// 환경광 반사 계수
	float diffuseHighlight;		// 난반사 계수(텍스처에서 사용)
	float specularHighlight;		// 정반사 계수
	float shininess;				// 광택 계수
	Vector2 padding1;				

	Vector3 cameraPos;				// 카메라 위치
	float padding2;
};

class Camera;
class DirectionalLight;
class Material;

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
	ID3D11ShaderResourceView* diffuseTRV = nullptr;

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
	void Render(Matrix& view, Matrix& projection, Camera& camera, DirectionalLight& light, Material& material);
	void UninitRenderPipeLine();
};