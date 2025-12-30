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


// Deferred Rendering 프로젝트입니다.
// Geometry Pass와 Lighting Pass를 분리하여 최종 가시 픽셀에 대해서만 라이팅을 계산합니다.
/*
* [Render Pass]
*   1. ShadowMap Pass                  -> ShadowMap
*   2. Geometry Pass                   -> G-buffer (Albedo, Normal, MetalRough, Emissive)
*   3. Lighting Pass                   -> Scene HDR Color Pass (Deferred Lighting)
*   4. Bloom Prefilter Pass            -> BloomA mip0 : Bloom에 밝은 부분만 남긴 base texture
*   5. Bloom Downsample Blur Pass      -> BloomA/B mips(ping-pong) : 블러처리된 mipamp 체인 texture
*   6. Bloom Upsample Combine Pass     -> BloomFinal(mip0) : mip들을 가산합성한 최종 Bloom texture
*   7. LDR PostProcess Pass            -> LDR (final)
*
* [G-buffer]
*   RT0 : Albedo (RGBA)
*   RT1 : Normal (RGB)
*   RT2 : Metallic (R), Roughness (G)
*   RT3 : Emissive (RGB)
*   -> Position은 대역폭 절약을 위해 G-buffer에 저장하지 않고,
*      Geometry Pass에서 사용한 뎁스 버퍼를 이용해 Position을 복원해 사용합니다.
* 
*  TODO :: SkyBox 라이팅 패스 이후에 적용
*          상용엔진들은 Skybox를 보통 라이팅 패스 이후에 비어있는 픽셀에 적용한다고 함.
*          지금은 Skybox를 먼저 그리고, 지오메트리 패스의 RTV를 투명하게 클리어하여
*          알파블렌딩으로 라이팅 패스에서 합성하는 방식을 사용중 -> 에바임
*/


class App : public WinApp
{
private:
    ID3D11ShaderResourceView* finalBloomSRV = nullptr;

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
    bool useBaseColorOverride = 0;
    bool useEmissiveOverride = 0;
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
    float clearColor[4] = { 0,0,0,1 };

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

    void StageSetting();
    void SkyBoxRender();
    void ShadowMapPass();
    void GeometryPass();
    void LightingPass();
    void BloomProcess();
    void PostProcess();

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

