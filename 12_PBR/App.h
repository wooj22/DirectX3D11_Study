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


// PBR 프로젝트입니다.
/*
    PBR 라이팅을 구현하고
    베이스컬러,노말,메탈릭,러프니스 텍스처를 사용한 메쉬(FBX 폴더에 char.fbx)를 렌더링합니다.
    PBR 파라메터 기본컬러,메탈릭,러프니스 의 값을 텍스처를 사용할것인지,
    전체에 임의의 값(테스트용도) 베이스컬러,노말,메탈릭,러프니스값을 적용할것인지 구분할수 있게합니다.
*/

class App : public WinApp
{
private:
    // matrix
    Matrix view;
    Matrix projection;
    Matrix lightView;
    Matrix lightProjection;

    // models
    StaticModel* floor = nullptr;
    StaticModel* zelda = nullptr;
    RigidModel* character = nullptr;
    SkeletalModel* girl = nullptr;
    SkeletalModel* enemy = nullptr;

    // skybox
    SkyBox skybox1;

    // light
    DirectionalLight light;

    // PBR debug
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    Vector3 baseColorOverride = { 1,1,1 };
    float metallicOverride = 0.5f;
    float roughnessOverride = 0.5f;
    bool useBaseColorOverride = 0;
    bool useMetallicOverride = 0;
    bool useRoughnessOverride = 0;

    // Gamma debug
    bool useGamma = 0;

    // clear color
    float clearColor[4] = { 0.2, 0.2, 0.2, 1.0f };

    // memory debugger
    MemoryDebugger memory_debugger;

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

