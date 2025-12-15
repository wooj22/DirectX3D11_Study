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


// Tone Mapping 프로젝트입니다.
/*
    HDR 백버퍼를 사용하여 1.0 이상의 밝기값을 계산하고, 
    Exposure와 ToneMapping을 통해 HDR의 밝은 부분(1.0이 넘는 부분)의 디테일을 살려 출력합니다.

    - 계산 -> HDR 백버퍼를 사용하여 1이상의 밝기값을 계산 (DXGI_FORMAT_R16G16B16A16_FLOAT)
    - 출력 -> LDR 백버퍼에 HDR 텍스처를 샘플링하여 0~1로 압출하여 출력 (DXGI_FORMAT_R8G8B8A8_UNORM)

    - 노출(Exposure) : 밝기값 조정
    - 톤매핑(ToneMapping) : HDR의 넓은 밝기 범위(1~1000 같은 값)를 LDR 디스플레이가 보여줄 수 있는 0~1 범위로 변환
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

    // camera
    float fovDeg = 60.0f;

    // shadow debug
    float lookPointDist = 300.f;
    float shadowLightDist = 200.f;
    float shadowWidth = 1920.0f;
    float shadowHeight = 1080.0f;
    float shadowNear = 0.01f;
    float shadowFar = 2000.0f;

    // PBR debug
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    Vector3 baseColorOverride = { 1,1,1 };
    float metallicOverride = 1.0f;
    float roughnessOverride = 0.0f;
    bool useBaseColorOverride = 0;
    bool useMetallicOverride = 0;
    bool useRoughnessOverride = 1;

    // Gamma debug
    bool useGamma = 0;

    // PostProcess
    bool useTint = 0;

    // IBL debug
    bool useIBL = 1;

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

