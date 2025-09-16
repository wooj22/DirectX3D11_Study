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


// Cube �޽��� �������ϰ�, OutputMerger�ܰ��� ���� �׽�Ʈ�� Ȯ���ϴ� ������Ʈ�Դϴ�.

class App : public WinApp
{
private:
	// rendering pipeline 
	ID3D11Buffer*		    vertexBuffer = nullptr;	     // ���� data
	ID3D11Buffer*			indexBuffer = nullptr;       // ���� index data
	ID3D11Buffer*	        constantBuffer = nullptr;    // world, view, projection matrix data
	ID3D11InputLayout*		inputLayout = nullptr;
	ID3D11VertexShader*		vertexShader = nullptr;
	ID3D11PixelShader*		pixelShader = nullptr;
	ID3D11DepthStencilView*	depthStencilView = nullptr;  // ����, ���ٽ� �׽�Ʈ (depth 24bit, stencil 8bit)

	// vertex info
	UINT vertexBufferStride = 0;				// ���ؽ� �ϳ��� ũ��
	UINT vertexBufferOffset = 0;				// ���ؽ� ������ ������
	UINT indexCount = 0;						// �ε��� ����

	// cube
	Vector3 cube1_position = { 0,0,0 };
	Vector3 cube2_position = { 3,0,0 };
	Vector3 cube3_position = { 2,0,0 };

	// camera
	Vector3 eye = { 0,1,-5 };		// camera position
	Vector3 at = { 0,1, 0 };		// look at point
	Vector3 up = { 0,1, 0 };		// y-up vector
	float FovY = 90.0f;	
	float Near = 0.01f;			
	float Far = 100.0f;

	// matrix
	Matrix cube1_world;
	Matrix cube2_world;
	Matrix cube3_world;
	Matrix view;
	Matrix projection;

	// else
	float time = 0.0f;
	float clearColor[4] = { 0, 0, 0, 1.0f };

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

