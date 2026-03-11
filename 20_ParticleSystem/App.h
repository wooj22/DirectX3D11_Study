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
#include "../WinBase/DefferedLightingRenderer.h"
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


// Particle System 프로젝트입니다.
/*
* [ Render PipeLine ]
*   1. ShadowMap Pass                  -> ShadowMap
*   2. Geometry Pass                   -> G-buffer (Albedo, Normal, MetalRough, Emissive)
*   3. Stencil Pass                    -> Stencil Buffer (Lighting Volume)
*   4. Lighting Pass                   -> Scene HDR Color Pass (Ligting Volume + Deferred Lighting)
*   5. Skybox Pass                     -> sceneHDR의 빈 픽셀에 Skybox Render (Depth Test Onlye)
*   6. Particle Pass                   -> Scene HDR Effect Render (Depth Test Onlye)
*   7. Bloom Prefilter Pass            -> BloomA mip0 : Bloom에 밝은 부분만 남긴 base texture
*   8. Bloom Downsample Blur Pass      -> BloomA/B mips(ping-pong) : 블러처리된 mipamp 체인 texture
*   9. Bloom Upsample Combine Pass     -> BloomFinal(mip0) : mip들을 가산합성한 최종 Bloom texture
*   10. LDR PostProcess Pass            -> LDR (final)
*   11. Frustum Debug Draw
*   12. GUI Draw
*
* 
*  [ Effect 구조 ]
* 
*   Effect
*       L vector<Emitter> : 파티클 생성 규칙
*   Emitter 
*       L SpriteSheet 1
*       L vector<Particle>
* 
*   단일 쿼드 Filpbook Effect와 다른 점은 Emitter 개념의 도입입니다.
*   Effect는 여러 Emitter를 가지며, 각 Emitter가 지정된 규칙에 따라 particle을 매 프레임 Spawn, Update 합니다.
*   Emitter에는 Fixed Mode와 Dynamic Mode가 있습니다.
*   기본 Dynamic Mode(particle system)이며, 
*   Filpbook Effect를 만들고 싶을 경우 Fixed 모드를 지정하고 fixedData에 값을 할당하면 됩니다.
*   자세한 설명은 Emitter.h를 읽어주세용
* 
*  
*  [ Effect Back-to-Front Sorting ] 
*  
*   Particle Render는 Depth Test Only로 진행됩니다.
*   때문에 DrawCall을 카메라에 멀리있는 이펙트 순서대로 진행해야합니다.
*   조금 더 제대로 가면 Emitter 단위로 sorting해야하지만,
*   아마도 그렇게 복잡한 Effect를 제작하지 않을 것 같아서
*   Effect단위로만 정렬 후 render 하고 있습니다. (매 프레임 sort)
* 
* 
*  [ Particle Local Batcing ]
*
*   기하 단위(Quad)로 Draw Call하지 않고,
*   Instance Buffer를 사용하여 같은 Effect끼리는 한번의 DrawCall로 처리합니다.
*
*   - Vertex (ParticleQuadVertex) : Quad 기하 Data
*   - Instance (ParticleInstance) : 매 프레임 살아있는 파티클의 상태 정보를 Update하는 Buffer Data
*   => DrawIndexedInstanced


*  [ Billboard Mode ]
*   1.  ScreenFacing  : Quad의 정면이 항상 카메라를 수직으로 바라봅니다.
*   2.  YAxis         : Y축을 고정하여 카메라의 좌우 이동, 회전에만 반응합니다.
*
*
*  [ Particle Pass를 위한 Stage Setting ]
*
*   Particle은 Skybox Pass 이후 SceneHDR에 Quad를 그리며 렌더합니다.
*   Skybox는 Depth를 wirte 하지 않기 때문에 Depth Test로 지오메트리와는 올바르게 계산되고, 배경은 덮어씌우게 됩니다.
*   Particle은 Alpha Blend를 켜줘야합니다~!
*
*   - RS : CullMode None
*   - DSS : Depth Test Only
*   - BS : Alpha Blend
*/


class App : public WinApp
{
private:
    // renderer
    ShadowRenderer       shadowRenderer;
    GeometryRenderer     geometryRenderer;
    DefferedLightingRenderer        lightRenderer;
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

