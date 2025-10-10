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
#include "Cube.h"
#include "DirectionalLight.h"
#include "Material.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// 블린퐁 라이팅 구현 프로젝트입니다.

// 블린퐁 라이팅 = 환경광 + 난반사광 + 정반사광
// 1) 환경광 = 환경광 반사 계수 * 간접광 세기
// 2) 난반사광 = 난반사 계수 * 광원 세기 * max(0, dot(N,L))
// 3) 정반사광 = 정반사 계수 * 광원 세기 * max(0, dot(N, H))^광택계수
//	       H = (L+V) / (|L+V|)

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

