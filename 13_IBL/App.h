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


// IBL(Image Based Lighting) 프로젝트입니다.
/*
    IBL_IrradianceMap, IBL_SpecularEnvMap, IBL_BRDF_LUT 텍스처를 사용하여
    정적 간접광을 구현합니다.
*/

class App : public WinApp
{
private:
    // skybox
    SkyBox skybox1;
    SkyBox skybox2;

    // IBL texture
    ID3D11ShaderResourceView* IBL_IrradianceMap1 = nullptr;
    ID3D11ShaderResourceView* IBL_IrradianceMap2 = nullptr;
    ID3D11ShaderResourceView* IBL_SpecularEnvMap1 = nullptr;
    ID3D11ShaderResourceView* IBL_SpecularEnvMap2 = nullptr;
    ID3D11ShaderResourceView* IBL_BRDF_LUT1 = nullptr;
    ID3D11ShaderResourceView* IBL_BRDF_LUT2 = nullptr;

    // light
    DirectionalLight light;

    // models
    StaticModel* floor = nullptr;
    StaticModel* zelda = nullptr;
    RigidModel* character = nullptr;
    SkeletalModel* girl = nullptr;
    SkeletalModel* enemy = nullptr;

    // matrix
    Matrix view;
    Matrix projection;
    Matrix lightView;
    Matrix lightProjection;

    // PBR debug
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    Vector3 baseColorOverride = { 1,1,1 };
    float metallicOverride = 1.0f;
    float roughnessOverride = 0.0f;
    bool useBaseColorOverride = 1;
    bool useMetallicOverride = 1;
    bool useRoughnessOverride = 1;

    // IBL debug
    bool useIBL = 1;

    // clear color
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

