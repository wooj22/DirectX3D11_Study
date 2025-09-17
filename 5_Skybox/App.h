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
#include "Object.h"
#include "DirectionalLight.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// Skybox Test 프로젝트입니다.
class App : public WinApp
{
private:
	// rendering pipeline (공용)
	ID3D11SamplerState* samplerState = nullptr;
	ID3D11DepthStencilView* depthStencilView = nullptr;

	// Objects
	Object cube;
	DirectionalLight light;

	// matrix
	Matrix view;
	Matrix projection;

	// else
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

