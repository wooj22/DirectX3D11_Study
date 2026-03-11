#pragma once
#include "../WinBase/WinApp.h"
#include "../WinBase/D3D.h"
#include "../WinBase/ModelLoader.h"
#include "../WinBase/StaticModel.h"
#include "../WinBase/RigidModel.h"
#include "../WinBase/SkeletalModel.h"
#include "../WinBase/Material.h"
#include "../WinBase/DirectionalShadowCamera.h"
#include "../WinBase/Light.h"
#include "../WinBase/ShadowRenderer.h"
#include "../WinBase/GeometryRenderer.h"
#include "../WinBase/LightRenderer.h"
#include "../WinBase/ParticleRenderer.h"
#include "../WinBase/SkyboxRenderer.h"
#include "../WinBase/BloomRenderer.h"
#include "../WinBase/PostProcessRenderer.h"
#include "../WinBase/FrustumRenderer.h"
#include "../WinBase/MemoryDebugger.h"
#include "../WinBase/Environment.hpp"

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

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;



// Decal 프로젝트입니다.
// Deffered Rendering의 한계인 투명 객체의 알파 블렌딩 문제를 해결하기 위해
// Opaque와 Transparent 객체를 구분하여 각각 Deffered, Forward Rendering으로 처리하는 Hybrid Rendering 방식을 도입하였습니다.

/*
* [ Render PipeLine ]
*   1. ShadowMap Pass                     -> ShadowMap
*   2. Geometry Pass (Opaque)             -> G-buffer (Albedo, Normal, MetalRough, Emissive)
*   3. Deffered Lighting Pass (Opaque)    -> Scene HDR, 최종 가시 픽셀에 대해서만 라이팅 계산
        1) Lighting Volume Stencil Pass
*       2) Lighting Pass
*   5. Skybox Pass                            -> sceneHDR의 빈 픽셀에 Skybox Render (Depth Test Onlye)
*   6. Forward Transparent Pss (Transparent)  -> Scene HDR Transparent Render (Depth Test Onlye)
*                                                Deffered Rendering의 한계를 보완하기 위해 투명 객체는 Forward Rendering으로 처리
*   7. Effect / Particle Pass                 -> Scene HDR Effect Render (Depth Test Onlye)
*   8  Bloom Pass                             -> Bloom 이미지 생성
        1) Bloom Prefilter Pass : BloomA mip0 : Bloom에 밝은 부분만 남긴 base texture
*       2) Bloom Downsample Blur Pass : BloomA/B mips(ping-pong) : 블러처리된 mipamp 체인 texture
*       3) Bloom Upsample Combine Pass : BloomFinal(mip0) : mip들을 가산합성한 최종 Bloom texture
*   11. PostProcess / Tone Mapping Pass            -> SDR (final)
*   12. Frustum Debug Draw
*   13. GUI Draw
*/


class App : public WinApp
{
private:
    // renderer
    ShadowRenderer       shadowRenderer;
    GeometryRenderer     geometryRenderer;
    LightRenderer        lightRenderer;
    ParticleRenderer     particleRenderer;
    SkyboxRenderer       skyboxRenderer;
    BloomRenderer        bloomRenderer;
    PostProcessRenderer  postRenderer;
    FrustumRenderer      frustumRenderer;

    // light
    vector<Light> lights;

    // effect
    vector<Effect> effects;

    // camera
    Matrix view;
    Matrix projection;

    // shadow camera
    DirectionalShadowCamera shadowCamera;
    ShadowOrthoDesc shadowOrthoDesc;
    Matrix shadowView;
    Matrix shadowProjection;

    // models
    vector<StaticModel*> static_models;
    vector<RigidModel*> rigid_models;
    vector<SkeletalModel*> skeletal_models;

    // environment
    vector<Environment> environments;

    // memory debugger
    MemoryDebugger memory_debugger;

public:
    // main process
    virtual bool OnInit() override;
    virtual void OnUninit() override;
    virtual void OnUpdate() override;
    virtual void OnRender() override;

private:
    void CBSlotBinding();
    void FrameCBUpdate();
    void DebugCBUpdate();

    // Init, Uninit
    bool InitResource();
    void UninitRenderPipeLine();

    // GUI
    bool InitGUI();
    void UninitGUI();
    void RenderGUI();

    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
    // Debug -----------------------------------------
    // camera
    float fovDeg = 60.0f;

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
    bool useLGG = 1;
    bool useVignette = 1;
    bool useFilmGrain = 0;
    bool useBloom = 1;

    bool useHueShift = 0;
    bool useColorTint = 0;
    bool useLift = 0;
    bool useGamma = 0;
    bool useGain = 1;

    // Screen Space Effect
    bool enableRipple;
    bool enablePlasmaOverlay;
    bool enableFilmGrain;
    bool enableDither;

    // IBL debug
    bool useIBL = true;

    // draw Debug
    bool frustumON = false;
    bool lightVolumeON = false;
};

