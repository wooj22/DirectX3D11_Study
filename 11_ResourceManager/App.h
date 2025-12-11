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
#include "../WinBase/MemoryDebugger.h"

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


// 리소스매니저 테스트 프로젝트입니다.
// AssetManager를 통한 Model Load로 같은 Model끼리는 읽기 전용 asset을 공유하여 메모리를 절약합니다.
// MemoryDebugger를 통해 메모리 사용량을 체크할 수 있습니다.

// [1] : 모델 생성
// [2] : 모델 삭제
// [T] : Trim() 요청

class App : public WinApp
{
private:
    // ramptexture
    ID3D11ShaderResourceView* diffuseRampTexture = nullptr;
    ID3D11ShaderResourceView* specualrRampTexture = nullptr;

    // matrix
    Matrix view;
    Matrix projection;

    // models
    vector<SkeletalModel*> skeletals;

    // skybox
    SkyBox skybox1;

    // light
    DirectionalLight light;

    // else
    float ambientHighlight = 0.3;
    float diffuseHighlight = 0.7;
    float specularHighlight = 0.9;
    float shininess = 500;
    float blendFactor[4] = { 0,0,0,0 }; UINT sampleMask = 0xffffffff;
    float clearColor[4] = { 0.2, 0.2, 0.2, 1.0f };

    // memory debugger
    MemoryDebugger debugger;

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

