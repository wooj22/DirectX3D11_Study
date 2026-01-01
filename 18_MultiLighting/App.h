#pragma once
#include "../WinBase/WinApp.h"
#include "../WinBase/D3D.h"
#include "../WinBase/ModelLoader.h"
#include "../WinBase/StaticModel.h"
#include "../WinBase/RigidModel.h"
#include "../WinBase/SkeletalModel.h"
#include "../WinBase/Material.h"
#include "../WinBase/Light.hpp"
#include "../WinBase/LightVolumeMesh.h"
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


// 다중 라이트 처리 프로젝트입니다.
// Directional, Point, Spot 라이트를 Deferred Rendering 방식으로 처리합니다.
// 01~17 프로젝트는 DirectionalLight.hpp를 사용하여 하나의 DirectioanlLight를 처리했지만,
// 이번 프로젝트에서는 Light.hpp를 사용하여 여러 라이트를 하나의 데이터타입으로 처리하고있습니다.
/*
* [ Render Pass ]
*   1. ShadowMap Pass                  -> ShadowMap
*   2. Geometry Pass                   -> G-buffer (Albedo, Normal, MetalRough, Emissive)
*   3. Stencil Pass
*   4. Lighting Pass                   -> Scene HDR Color Pass (Deferred Lighting)
*   5. Bloom Prefilter Pass            -> BloomA mip0 : Bloom에 밝은 부분만 남긴 base texture
*   6. Bloom Downsample Blur Pass      -> BloomA/B mips(ping-pong) : 블러처리된 mipamp 체인 texture
*   7. Bloom Upsample Combine Pass     -> BloomFinal(mip0) : mip들을 가산합성한 최종 Bloom texture
*   8. LDR PostProcess Pass            -> LDR (final)
*
* [ G-buffer ]
*   RT0 : Albedo (RGB)
*   RT1 : Normal (RGB)
*   RT2 : Metallic (R), Roughness (G)
*   RT3 : Emissive (RGB)
*   ★ Position은 대역폭 절약을 위해 G-buffer에 저장하지 않고,
*      Geometry Pass에서 사용한 뎁스 버퍼를 이용해 Position을 복원해 사용합니다.
* 
* [ Light Volume ]
* 
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
    vector<Light> lights;

    // light volume model
    LightVolumeMesh sphereVolume;
    LightVolumeMesh coneVolume;

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
    void StencilPass();
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

