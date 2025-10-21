#pragma once
#include "../WinBase/WinApp.h"
#include "../WinBase/D3D.h"
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include "Cube.h"
#include "DirectionalLight.hpp"
#include "Material.hpp"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// Nomal Map 적용 프로젝트입니다.

// 1. normalamp의 rgb값을 tangent space 기준 normal로 변환 (0~1 -> -1~1)
// 2. tbn행렬곱을 통해 world space 기준 normal로 변환
// 3. 라이팅 연산에 위 normal값 활용

class App : public WinApp
{
private:
	// rendering pipeline
	ID3D11SamplerState* samplerState = nullptr;

	// Objects
	Cube cube;
	Material material;
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

