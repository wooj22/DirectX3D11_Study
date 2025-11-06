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

#include "ModelLoder.h"
#include "StaticMesh.h"
#include "RigidMesh.h"
#include "SkeletalMesh.h"
#include "Material.h"
#include "DirectionalLight.hpp"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// FBX 파일 로드 프로젝트입니다.
// StaticMesh, RigidMesh(Transform Animation), 투명 텍스처 처리에 대한 내용이 포함되어 있습니다.
class App : public WinApp
{
private:
	// rendering pipeline
	ID3D11Buffer* constantBuffer = nullptr;
    ID3D11Buffer* offsetMatrixCB = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
    ID3D11InputLayout* inputLayout_weight = nullptr;
	ID3D11VertexShader* VS_Basic = nullptr;
    ID3D11VertexShader* VS_Skinning= nullptr;
	ID3D11PixelShader* PS_Basic = nullptr;
    ID3D11PixelShader* PS_Toon = nullptr;

	// matrix
	Matrix view;
	Matrix projection;

	// models
	StaticMesh* character = nullptr;		// Diffuse, Normal, Emisive, Specular
	StaticMesh* zelda = nullptr;			// Diffuse, Opacity
	StaticMesh* tree = nullptr;				// Diffuse, Opacity
	RigidMesh* boxHuman = nullptr;			// transform animation
    SkeletalMesh* skinningTest = nullptr;	// transform animation

	// light
	DirectionalLight light;

	// else
	float ambientHighlight = 0.3;
	float diffuseHighlight = 0.5;
	float specularHighlight = 0.7;
	float shininess = 500;
	float blendFactor[4] = { 0,0,0,0 }; UINT sampleMask = 0xffffffff;
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

