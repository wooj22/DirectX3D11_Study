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
#include "../WinBase/DebugDraw.h"

#include <iostream>
using namespace std;

#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/CommonStates.h>
#include <directxtk/Effects.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// Screen Space Effect 프로젝트입니다.
/*
    Random -> Noise -> FBM -> Domain Wraping 패턴을 기반으로
    리플(물결), 플라즈마, 필름그레인 효과를 적용합니다.

    1. Water Distortion : Domain Wraping 노이즈로 UV 좌표를 왜곡
    2. Plasma Overlay : Domain Warping + FBM 패턴의 color
    3. Film Grain : FBM 기반 노이즈로 미세한 입자감
*/

class App : public WinApp
{
private:
    // skybox
    SkyBox skybox1;
    SkyBox skybox2;
    SkyBox skybox3;
    SkyBox skybox4;

    // IBL texture
    ID3D11ShaderResourceView* IBL_IrradianceMap1 = nullptr;
    ID3D11ShaderResourceView* IBL_SpecularEnvMap1 = nullptr;
    ID3D11ShaderResourceView* IBL_BRDF_LUT1 = nullptr;

    ID3D11ShaderResourceView* IBL_IrradianceMap2 = nullptr;
    ID3D11ShaderResourceView* IBL_SpecularEnvMap2 = nullptr;
    ID3D11ShaderResourceView* IBL_BRDF_LUT2 = nullptr;

    ID3D11ShaderResourceView* IBL_IrradianceMap3 = nullptr;
    ID3D11ShaderResourceView* IBL_SpecularEnvMap3 = nullptr;
    ID3D11ShaderResourceView* IBL_BRDF_LUT3 = nullptr;

    ID3D11ShaderResourceView* IBL_IrradianceMap4 = nullptr;
    ID3D11ShaderResourceView* IBL_SpecularEnvMap4 = nullptr;
    ID3D11ShaderResourceView* IBL_BRDF_LUT4 = nullptr;

    // light
    DirectionalLight light;

    // models
    vector<StaticModel*> spheres;
    vector<StaticModel*> torus;
    StaticModel* floor = nullptr;
    StaticModel* tree = nullptr;
    StaticModel* zelda = nullptr;
    RigidModel* character = nullptr;
    SkeletalModel* girl = nullptr;
    SkeletalModel* enemy = nullptr;

    // matrix
    Matrix view;
    Matrix projection;
    Matrix lightView;
    Matrix lightProjection;

    // camera
    float fovDeg = 60.0f;

    // shadow debug
    float lookPointDist = 500.f;
    float shadowLightDist = 1000.f;
    float shadowWidth = 3000.0f;
    float shadowHeight = 3000.0f;
    float shadowNear = 0.01f;
    float shadowFar = 5000.0f;

    // PBR debug
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    Vector3 baseColorOverride = { 1,1,1 };
    float metallicOverride = 1.0f;
    float roughnessOverride = 0.0f;
    bool useBaseColorOverride = 0;
    bool useMetallicOverride = 0;
    bool useRoughnessOverride = 0;

    // Gamma debug
    bool useGamma = 1;

    // PostProcess
    bool useTint = 0;
    bool enableRipple;
    bool enablePlasmaOverlay;
    bool enableFilmGrain;
    bool enableDither;

    // IBL debug
    bool useIBL = 1;

    // alpha
    float blendFactor[4] = { 0,0,0,0 };
    UINT sampleMask = 0xffffffff;

    // clear color
    float clearColor[4] = { 0.2, 0.2, 0.2, 1.0f };

    // draw debug
    unique_ptr<CommonStates> m_states;
    unique_ptr<PrimitiveBatch<VertexPositionColor>> m_batch;
    unique_ptr<BasicEffect> m_effect;
    ComPtr<ID3D11InputLayout> m_layout = nullptr;

    // memory debugger
    MemoryDebugger memory_debugger;

public:
    // main process
    virtual bool OnInit() override;
    virtual void OnUninit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;

    void HDRRender();
    void PostProcessing();

    // rendering pipeline
    bool InitRenderPipeLine();
    void UninitRenderPipeLine();

    // gui 
    bool InitGUI();
    void UninitGUI();
    void RenderGUI();

    // debug draw
    void FrustumDebugDraw(const Matrix& frustumView, const Matrix& frustumProj,
        const Matrix& renderView, const Matrix& renderProj, FXMVECTOR color = Colors::Red);

    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
};

