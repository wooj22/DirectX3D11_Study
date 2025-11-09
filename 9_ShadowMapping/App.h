#pragma once
#include "../WinBase/WinApp.h"
#include "../WinBase/D3D.h"
#include "../WinBase/ModelLoader.h"
#include "../WinBase/StaticMesh.h"
#include "../WinBase/RigidMesh.h"
#include "../WinBase/SkeletalMesh.h"
#include "../WinBase/Material.h"
#include "../WinBase/DirectionalLight.hpp"
#include "../WinBase/SkyBox.h"

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


// 그림자 매핑 프로젝트입니다.
// 이 프로젝트는 WinBase 정적 라이브러리의 Model 렌더 모듈을 사용합니다.

class App : public WinApp
{
private:
    // matrix
    Matrix view;
    Matrix projection;

    // models
    StaticMesh* character = nullptr;		// Diffuse, Normal, Emisive, Specular
    StaticMesh* zelda = nullptr;			// Diffuse, Opacity
    StaticMesh* tree = nullptr;				// Diffuse, Opacity
    RigidMesh* boxHuman = nullptr;			// rigid skeletal animation
    SkeletalMesh* warrior = nullptr;    	// skinned skeletal animation
    SkeletalMesh* enemy = nullptr;    	// skinned skeletal animation

    // light
    DirectionalLight light;

    // skybox
    SkyBox skybox;

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

