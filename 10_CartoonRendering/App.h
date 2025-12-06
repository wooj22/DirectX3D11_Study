#pragma once
#include "../WinBase/WinApp.h"
#include "../WinBase/D3D.h"
#include "../WinBase/ModelLoader.h"
#include "../WinBase/StaticModel.h"
#include "../WinBase/RigidModel.h"
#include "../WinBase/SkeletalModel.h"
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


// 카툰렌더링 프로젝트입니다.
// 램프텍스처를 활용한 Diffuse, Specular 툰쉐이딩 + 아웃라인 쉐이더를 사용합니다.

class App : public WinApp
{
private:
    // RampTexture
    ID3D11ShaderResourceView* diffuseRampTexture = nullptr;
    ID3D11ShaderResourceView* specualrRampTexture = nullptr;

    // matrix
    Matrix view;
    Matrix projection;

    // models
    SkeletalModel* warrior = nullptr;
    SkeletalModel* character = nullptr;

    // skybox
    SkyBox skybox;

    // light
    DirectionalLight light;

    // else
    float ambientHighlight = 0.3;
    float diffuseHighlight = 0.7;
    float specularHighlight = 0.9;
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

