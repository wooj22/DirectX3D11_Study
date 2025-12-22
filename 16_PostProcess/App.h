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


// PostProcess 테스트 프로젝트입니다.
/*
* [Render Pass]
* 1. ShadowMap Pass                  -> ShadowMap
* 2. Scene HDR Color Pass            -> SceneHDR
* 3. Bloom Prefilter Pass            -> BloomA mip0 : Bloom에 밝은 부분만 남긴 base texture
* 4. Bloom Downsample Blur Pass      -> BloomA/B mips(ping-pong) : 블러처리된 mipamp 체인 texture
* 5. Bloom Upsample Combine Pass     -> BloomFinal(mip0) : mip들을 가산합성한 최종 Bloom texture
* 6. LDR PostProcess Pass            -> LDR (final)
* 
* [PostProcess]
*   - 노출
    - Color Adjustments (채도, 대비, Hue Shift, Tint)
    - Bloom
    - Film Grain
    - Vignette
    - Lift, Gamma, Gain
    - White Balance (온도, 색조)
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
    bool usedefalutGamma = 1;

    // PostProcess
    bool useColorAdjustments = 0;
    bool useWhiteBalance = 0;
    bool useLGG = 0;
    bool useVignette = 0;
    bool useFilmGrain = 0;
    bool useBloom = 0;

    bool useHueShift = 0;
    bool useColorTint = 0;
    bool useLift = 0;
    bool useGamma = 0;
    bool useGain = 0;

    // Screen Space Effect
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

