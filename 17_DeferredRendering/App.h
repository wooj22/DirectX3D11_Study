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
#include <directxtk/CommonStates.h>
#include <directxtk/Effects.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// Deferred Rendering 프로젝트입니다.
// Geometry Pass와 Lighting Pass를 분리하여 최종 가시 픽셀에 대해서만 라이팅을 계산합니다.
/*
* [ Render Pass ]
*   1. ShadowMap Pass                  -> ShadowMap
*   2. Geometry Pass                   -> G-buffer (Albedo, Normal, MetalRough, Emissive)
*   3. Lighting Pass                   -> Scene HDR Color Pass (Deferred Lighting)
*   4. Skybox   Pass                   -> Scene HDR의 빈 픽셀에 Skybox Render
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
* [ Skybox ]
*   Deferred Rendering에서는 Skybox를 Lighting Pass 이후에 렌더링 합니다.
*   - Forward : Skybox를 먼저 그리고(Depth 1) 이후 지오메트리 렌더링
*   - Deferred : 라이팅 패스에서 sceneHDR을 계산한 후, Skybox(depth 1)을 그리며
                 지오메트리 패스에서 기록된 Depth에 Depth Test를 진행하여 
                 아무것도 그려지지 않은 픽셀에만 Skybox가 렌더링 됩니다.
* 
* [ Alpha Blending ]
*   Deferred Rendering에서는 최종 가시 픽셀만 연산을 하므로
*   알파 블렌딩이 필요한 오브젝트를 처리하기 어렵다. 
    -> 때문에 해당 프로젝트에서는 알파 블렌딩 State를 사용하지 않는다.
*/


class App : public WinApp
{
private:
    // renderer
    ShadowRenderer       shadowRenderer;
    GeometryRenderer     geometryRenderer;
    LightRenderer        lightRenderer;
    SkyboxRenderer       skyboxRenderer;
    BloomRenderer        bloomRenderer;
    PostProcessRenderer  postRenderer;
    FrustumRenderer      frustumRenderer;

    // light
    vector<Light> lights;

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
    // Init, Uninit
    bool InitResource();

    // CB Update
    void CBSlotBinding();
    void FrameCBUpdate();
    void DebugCBUpdate();

    // GUI
    bool InitGUI();
    void UninitGUI();
    void RenderGUI();

    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

private:
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

    // draw Debug
    bool frustumON = false;
};

