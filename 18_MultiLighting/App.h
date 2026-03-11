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

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::SimpleMath;


// 다중 라이트 처리 프로젝트입니다.
// Directional, Point, Spot 라이트를 Lighting Volume Rendering 합니다.
/*
* [ Render PipeLine ]
*   1. ShadowMap Pass                  -> ShadowMap
*   2. Geometry Pass                   -> G-buffer (Albedo, Normal, MetalRough, Emissive)
*   3. Stencil Pass                    -> Stencil Buffer (Lighting Volume)
*   4. Lighting Pass                   -> Scene HDR Color Pass (Ligting Volume + Deferred Lighting)
*   5. Skybox Pass                     -> sceneHDR의 빈 픽셀에 Skybox Render
*   6. Bloom Prefilter Pass            -> BloomA mip0 : Bloom에 밝은 부분만 남긴 base texture
*   7. Bloom Downsample Blur Pass      -> BloomA/B mips(ping-pong) : 블러처리된 mipamp 체인 texture
*   8. Bloom Upsample Combine Pass     -> BloomFinal(mip0) : mip들을 가산합성한 최종 Bloom texture
*   9. LDR PostProcess Pass            -> LDR (final)
*   10. Frustum Debug Draw
*   11. GUI Draw
* 
* 
* [ Multi Lighting ]
*  - Directional, Point, Spot 라이트를 모두 처리합니다.
*  - Shaodw와 IBL은 sunLight로 설정된 하나의 directional에 대해서만 처리합니다.
*  - Lighting Volume Rendering을 활용하여 관련없는 픽셀에 대한 PS 실행을 최소화합니다.
*  - Additive Blending을 통해 라이트별로 실행한 결과를 RTV에 누적합니다.
*
* 
* [ Light Volume ]
*   - Directioanl : Full Screen Quad
*   - Point : Light Volume (Sphere)  // InSide라면 Full Screen Quad
*   - Spot : Light volume (Cone)     // InSide라면 Full Screen Quad
*   
*   라이트 볼륨 렌더링을 위해 Stencil Pass가 추가됩니다.
*    1) Stencil Pass : Depth test On/write off + Stencil test on/write On (test는 항상 통과)
                       => 라이트 볼륨을 그리며, 깊이테스트를 통해 표면이 있는 픽셀에만 Stencil을 write
*    2) Lighting Pass : Depth disable + Stencil test on/wrtie off
*                      => Stencil Test를 통해 Stencil Pass에서 표시한 픽셀들에 대해서만
*                         Pixel Shader를 실행합니다.
*                      => 카메라가 볼륨 내부에 있는 경우는 Full Screen Render를 진행합니다.
* 
* 
* [ InSide / OutSide ]
*  1) Camera Inside Volume : 카메라가 볼륨 안에 있는 경우
*     라이팅 볼륨을 그리지 않고 그냥 FullScreenPass를 진행한다.
*     - Stencil Test off, FullScreen Render
*     - 모든 영역에 대해 PS를 실행하지만, 안전함
*     - 이걸 CullFront + Z-Fail 처리 하려면 더 복잡함
* 
*  2) Camera Outside Volume : 카메라가 볼륨 밖에 있는 경우
*     라이팅 볼륨 패스를 진행한다.
*     - Stencil Test on + Lighting Volume Render
*     - Stencil 패스에서 Stencil이 1로 기록된 픽셀에 대해서만 PS 실행
*     - Cull Back
* 
* 
* [ G-buffer ]
*   RT0 : Albedo (RGB)
*   RT1 : Normal (RGB)
*   RT2 : Metallic (R), Roughness (G)
*   RT3 : Emissive (RGB)
*   ★ Position은 대역폭 절약을 위해 G-buffer에 저장하지 않고,
*      Geometry Pass에서 사용한 뎁스 버퍼를 이용해 Position을 복원해 사용합니다.
*/


class App : public WinApp
{
private:
    // renderer
    ShadowRenderer       shadowRenderer;
    GeometryRenderer     geometryRenderer;
    DefferedLightingRenderer        lightRenderer;
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
    bool useIBL = true;

    // draw Debug
    bool frustumON = false;
    bool lightVolumeON = false;
};

