#pragma once
#include "../WinBase/WinApp.h"
#include "../D3DBase/D3DBase.h"
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


class Object
{
public:
	Vector3 position;		// ��ġ
	Vector3 rotation;		// ȸ��
	Vector3 scale;		    // ������
	Matrix world;			// ���� ���

	Object()
	{
		position = Vector3::Zero;
		rotation = Vector3::Zero;
		scale = Vector3::One;
		world = XMMatrixIdentity();
	}

	Object(Vector3 p, Vector3 r, Vector3 s)
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

	void Set(Vector3 p, Vector3 r, Vector3 s)
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
};

struct Camera
{
	Vector3 eye = { 0,1,-5 };		// camera position
	Vector3 at = { 0,1, 0 };		// look at point
	Vector3 up = { 0,1, 0 };		// y-up vector

	float FovY = 90.0f;
	float Near = 0.01f;
	float Far = 100.0f;
};

struct DirectionalLight
{
	// ���� direction�� light�� ����������, ���� ������ ���� ǥ��->light�� ������ ���
	Vector4 direction = { 0.5f, 0.8f, -0.8, 1.0f };		
	Vector4 color{ 1.0, 1.0f, 1.0, 1.0 };
};


// ����Ʈ ��Ģ�� ���� Directional Light Test ������Ʈ�Դϴ�.
class App : public WinApp
{
private:
	// rendering pipeline 
	ID3D11Buffer*				vertexBuffer = nullptr;			
	ID3D11Buffer*				indexBuffer = nullptr;			
	ID3D11Buffer*				constantBuffer = nullptr;		
	ID3D11InputLayout*			inputLayout = nullptr;
	ID3D11VertexShader*			vertexShader = nullptr;
	ID3D11PixelShader*			pixelShader = nullptr;
	ID3D11DepthStencilView*		depthStencilView = nullptr;		

	// vertex info
	UINT vertexBufferStride = 0;		
	UINT vertexBufferOffset = 0;		
	UINT indexCount = 0;				

	// Objects
	Object cube;
	Object cube2;
	Camera camera2;
	DirectionalLight light;

	// matrix
	Matrix view;
	Matrix projection;

	// else
	float time = 0.0f;
	float clearColor[4] = { 0.2, 0.2, 0.2, 1.0f };

public:
	// main process
	virtual bool OnInit() override;
	virtual void OnUninit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;

	// rendering pipeline
	bool InitRenderPipeLine();
	void UninitRenderPipeLine();

	// gui 
	bool InitGUI();
	void UninitGUI();
	void RenderGUI();

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
};

