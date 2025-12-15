#include "App.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Helper.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>
#include "../WinBase/Camera.h"
#include "../WinBase/AssetManager.h"


#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define USE_FLIPMODE 1

static int currentSkybox = 0;
const char* skyboxes[] = { "OutDoor", "InDoor" };

// Main process
bool App::OnInit()
{
    if (!D3D::Init(hWnd, screenWidth, screenHeight)) return false;
    if (!InitRenderPipeLine()) return false;
    if (!InitGUI()) return false;

    // skybox
    skybox1.InitRenderPipeLine(L"../Resource/Skybox/skybox_cubmap.dds");
    skybox2.InitRenderPipeLine(L"../Resource/Skybox/indoorEnvHDR.dds");

    // model
    floor = new StaticModel();
    zelda = new StaticModel();
    character = new RigidModel();
    girl = new SkeletalModel();
    enemy = new SkeletalModel();
    AssetManager::Instance().LoadStaticModelAsset(floor, "../Resource/Plane.fbx");
    AssetManager::Instance().LoadStaticModelAsset(zelda, "../Resource/zeldaPosed001.fbx");
    AssetManager::Instance().LoadRigidModelAsset(character, "../Resource/char.fbx");
    AssetManager::Instance().LoadSkeletalModelAsset(girl, "../Resource/Girl.fbx");
    AssetManager::Instance().LoadSkeletalModelAsset(enemy, "../Resource/Enemy.fbx");

    floor->SetPosition({ 0,-5, 50 });
    floor->SetScale({ 0.4,1,0.3 });
    zelda->SetPosition({ -100,0,0 });
    character->SetPosition({ 0,0,0 });
    girl->SetPosition({ 100,0,70 });
    enemy->SetPosition({ 250,0,20 });

    // light
    light.direction = { 0,-0.5,1 };
    light.color = { 1.0f, 0.9608f, 0.8980f, 1.0f };
    light.directIntensity = 1.0f;
    light.indirectIntensity = 0.8f;

    // view maxtrix
    camera.position = { 0, 80, -300 };
    camera.Far = 2000.0f;
    camera.moveSpeed = 300.f;
    camera.GetViewMatrix(view);

    // projection matrix 
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    // use IBL
    D3D::debugCBData.useIBL = 1;

    // memory debugger
    memory_debugger.Init();

    // debug draw set up

    m_states = std::make_unique<CommonStates>(D3D::device.Get());
    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(D3D::deviceContext.Get());
    m_effect = std::make_unique<BasicEffect>((D3D::device.Get()));
    m_effect->SetVertexColorEnabled(true);
    m_effect->SetView(view);
    m_effect->SetProjection(projection);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        HR_T(D3D::device.Get()->CreateInputLayout(
            VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_layout.ReleaseAndGetAddressOf())
        );
    }

    return true;
}

void App::OnUninit()
{
    UninitGUI();
    UninitRenderPipeLine();
    D3D::UnInit();
    CheckDXGIDebug();
}

void App::OnUpdate()
{
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    // model update(local, model matrix)
    character->Update();
    girl->Update();
    enemy->Update();
    camera.GetViewMatrix(view);

    // light update
    Vector3 sceneCenter = camera.position + camera.GetForward() * lookPointDist;
    Vector3 lightPos = sceneCenter - light.direction * shadowLightDist;
    lightView = XMMatrixLookAtLH(lightPos, sceneCenter, Vector3::Up);
    lightProjection = XMMatrixOrthographicLH(shadowWidth, shadowHeight, shadowNear, shadowFar);

    // Memory Cheak
    memory_debugger.CheakMemoryUsage();

    // Trim
    if (Input::GetKeyDown('T'))
    {
        memory_debugger.Trim();
    }
}

void App::OnRender()
{
    // Render
    HDRRender();          // Gaometry + Lighting + Shadow
    PostProcessing();     // ToneMapping + Exposure  (TODO :: Bloom, ColorGrading, Vignette, Film Grain)

    // Debug Draw
    FrustumDebugDraw(view, projection, view, projection, Colors::FloralWhite);
    FrustumDebugDraw(lightView, lightProjection, view, projection, Colors::GreenYellow);

    // GUI
    RenderGUI();

    // Present
    D3D::swapChain->Present(1, 0);
}

// [ Gaometry + Lighting + Shadow Pass ]
void App::HDRRender()
{
    // Clear
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::hdrRTV.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->ClearRenderTargetView(D3D::hdrRTV.Get(), clearColor);

    // death buffer clear
    D3D::deviceContext->ClearDepthStencilView(D3D::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);


    // --------------------- Stage Setting -----------------------
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());
    D3D::deviceContext->PSSetSamplers(1, 1, D3D::shadowSamplerState.GetAddressOf());
    //D3D::deviceContext->OMSetBlendState(D3D::blendState.Get(), blendFactor, sampleMask);

    D3D::deviceContext->VSSetConstantBuffers(0, 1, D3D::transformBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(0, 1, D3D::transformBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(1, 1, D3D::lightingBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(1, 1, D3D::lightingBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(2, 1, D3D::materialBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(3, 1, D3D::offsetMatrixBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(4, 1, D3D::poseMatrixBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(6, 1, D3D::debugBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(7, 1, D3D::postprocessBuffer.GetAddressOf());

    // Skybox Render  --------------------------------
    switch (currentSkybox)
    {
    case 0:
        skybox1.Render(view, projection);
        break;
    case 1:
        skybox2.Render(view, projection);
        break;
    }


    // Buffer Data Update -----------------------------------
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::transformCBData.shadowView = XMMatrixTranspose(lightView);
    D3D::transformCBData.shadowProjection = XMMatrixTranspose(lightProjection);

    D3D::lightingCBData.lightDirection = light.direction;
    D3D::lightingCBData.lightColor = light.color;
    D3D::lightingCBData.directIntensity = light.directIntensity;
    D3D::lightingCBData.indirectIntensity = light.indirectIntensity;
    D3D::lightingCBData.cameraPos = camera.position;

    D3D::debugCBData.metallicFactor = metallicFactor;
    D3D::debugCBData.roughnessFactor = roughnessFactor;
    D3D::debugCBData.baseColorOverride = baseColorOverride;
    D3D::debugCBData.metallicOverride = metallicOverride;
    D3D::debugCBData.roughnessOverride = roughnessOverride;
    D3D::debugCBData.useBaseColorOverride = useBaseColorOverride ? 1 : 0;
    D3D::debugCBData.useMetallicOverride = useMetallicOverride ? 1 : 0;
    D3D::debugCBData.useRoughnessOverride = useRoughnessOverride ? 1 : 0;
    D3D::debugCBData.useIBL = useIBL ? 1 : 0;

    D3D::postprocessCBData.useGamma = useGamma ? 1 : 0;

    D3D::deviceContext->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::debugBuffer.Get(), 0, nullptr, &D3D::debugCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::postprocessBuffer.Get(), 0, nullptr, &D3D::postprocessCBData, 0, 0);


    // 1. Depth Only Pass -------------------------------------
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_shadowMap);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(0, nullptr, D3D::shadowDSV.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->ClearDepthStencilView(D3D::shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    // Static, Rigid Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::ShadowDepth_Static_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(nullptr, NULL, 0);
    zelda->Render();
    character->Render();

    // Skeletal Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::ShadowDepth_Skinned_VS.Get(), NULL, 0);
    girl->Render();
    enemy->Render();


    // 2. PBR Mesh Render Pass -------------------------------------
    // Static, Rigid Model
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::hdrRTV.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    //D3D::deviceContext->OMSetDepthStencilState(D3D::depthStencilState.Get(), 0);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Static_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PBR_PS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    switch (currentSkybox)
    {
    case 0:
        D3D::deviceContext->PSSetShaderResources(9, 1, &IBL_IrradianceMap1);
        D3D::deviceContext->PSSetShaderResources(10, 1, &IBL_SpecularEnvMap1);
        D3D::deviceContext->PSSetShaderResources(11, 1, &IBL_BRDF_LUT1);
        break;
    case 1:
        D3D::deviceContext->PSSetShaderResources(9, 1, &IBL_IrradianceMap2);
        D3D::deviceContext->PSSetShaderResources(10, 1, &IBL_SpecularEnvMap2);
        D3D::deviceContext->PSSetShaderResources(11, 1, &IBL_BRDF_LUT2);
        break;
    }
    floor->Render();
    zelda->Render();
    character->Render();

    // Skeletal Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Skinned_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    girl->Render();
    enemy->Render();

    // SRV hazard 방지
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(6, 1, nullSRV);
}

// [ PostProcessing Pass ]
// ToneMapping + Exposure  (TODO :: Bloom, ColorGrading, Vignette, Film Grain)
// Tone Mapping 패스는 화면을 덮는 FullScreen 사각형을 그리면서,
// HDR SRV를 샘플링해 색을 계산하고, 그 결과를 BackBuffer에 기록하는 단계
void App::PostProcessing()
{
    // view port
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);

    // SRV hazard 방지
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);

    // OM 
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), nullptr);
    D3D::deviceContext->ClearRenderTargetView(D3D::renderTargetView.Get(), clearColor);

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->IASetInputLayout(nullptr);

    // Shaders
    D3D::deviceContext->VSSetShader(D3D::FullScreen_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PostProcess_PS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(12, 1, D3D::hdrSRV.GetAddressOf());

    // Render
    D3D::deviceContext.Get()->Draw(3, 0);

    // SRV hazard 방지
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
}

bool App::InitRenderPipeLine()
{
    // IBL Textures Load
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/skybox_cubemapDiffuseHDR.dds", nullptr, &IBL_IrradianceMap1);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/skybox_cubemapSpecularHDR.dds", nullptr, &IBL_SpecularEnvMap1);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/skybox_cubemapBrdf.dds", nullptr, &IBL_BRDF_LUT1);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/indoorDiffuseHDR.dds", nullptr, &IBL_IrradianceMap2);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/indoorSpecularHDR.dds", nullptr, &IBL_SpecularEnvMap2);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/indoorBrdf.dds", nullptr, &IBL_BRDF_LUT2);

    return true;
}

void App::UninitRenderPipeLine()
{
    skybox1.UninitRenderPipeLine();
}

bool App::InitGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(D3D::device.Get(), D3D::deviceContext.Get());

    return true;
}

void App::UninitGUI()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void App::RenderGUI()
{
    // GUI
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Inspector
    ImGui::Begin("Inspertor", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Text("[Light]");
    ImGui::SliderFloat3("Direction", &light.direction.x, -1.0f, 1.0f, "%.2f");
    ImGui::ColorEdit3("Color", &light.color.x);
    ImGui::SliderFloat("Direct Intensity", &light.directIntensity, 0.0f, 10.0f);
    ImGui::SliderFloat("Indirect Intensity", &light.indirectIntensity, 0.0f, 10.0f);

    ImGui::Text("");
    ImGui::Text("[Material]");
    ImGui::SliderFloat("Metallic Factor", &metallicFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness Factor", &roughnessFactor, 0.0f, 1.0f);
    ImGui::Checkbox("BaseColor Override", &useBaseColorOverride);
    ImGui::ColorEdit3("BaseColor", &baseColorOverride.x);

    ImGui::Checkbox("Metallic Override", &useMetallicOverride);
    ImGui::SliderFloat("Metallic", &metallicOverride, 0.0f, 1.0f);
    ImGui::Checkbox("Roughness Override", &useRoughnessOverride);
    ImGui::SliderFloat("Roughness", &roughnessOverride, 0.0f, 1.0f);

    ImGui::Text("");
    ImGui::Text("[Transform]");
    ImGui::InputFloat3("position", &character->position.x);
    ImGui::SliderAngle("Pitch", &character->rotation.x, 0.0f, 360.0f);
    ImGui::SliderAngle("Yaw", &character->rotation.y, 0.0f, 360.0f);
    ImGui::SliderAngle("Roll", &character->rotation.z, 0.0f, 360.0f);
    ImGui::InputFloat3("scale", &character->scale.x);
    ImGui::End();

    // Camera
    ImGui::Begin("[Camera]");
    ImGui::SliderFloat("Near", &camera.Near, 0.01f, 10000.0f);
    ImGui::SliderFloat("Far", &camera.Far, 0.01f, 10000.0f);

    ImGui::SliderFloat("FOV", &fovDeg, 20.0f, 90.0f);

    camera.FovY = XMConvertToRadians(fovDeg);
    camera.FovY = std::clamp(camera.FovY, 0.3f, 1.7f);
    ImGui::InputFloat("Move Speec", &camera.moveSpeed, 0.0f, 0.0f, "%.3f");
    ImGui::End();

    // Shadow
    ImGui::Begin("[Shadow]");
    ImGui::Text("[Shadow Frustum]");
    ImGui::SliderFloat("Near", &shadowNear, 0.01f, 10000.0f);
    ImGui::SliderFloat("Far", &shadowFar, 0.01f, 10000.0f);
    ImGui::InputFloat("Width", &shadowWidth);
    ImGui::InputFloat("Height", &shadowHeight);

    ImGui::Text("");
    ImGui::Text("[Shadow Light Pos]");
    ImGui::SliderFloat("lookPointDist", &lookPointDist, 1.f, 5000.0f);
    ImGui::SliderFloat("shadowLightDist", &shadowLightDist, 1.f, 5000.0f);
    ImGui::End();

    // IBL
    ImGui::Begin("[IBL]");
    ImGui::Checkbox("use IBL", &useIBL);

    ImGui::Text("");
    ImGui::Text("[Skybox]");
    ImGui::Combo("Skybox Mode", &currentSkybox, skyboxes, IM_ARRAYSIZE(skyboxes));
    ImGui::End();

    // PostProcess
    ImGui::Begin("[PostProcess]");
    ImGui::Checkbox("use Gamma", &useGamma);

    ImGui::End();

    // Memory
    ImGui::Begin("[Memory Debugger]");
    ImGui::Text("[T] Trim");
    ImGui::Text("%ls", memory_debugger.GetMemoryUsageWstring().c_str());
    ImGui::End();


    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// Frustum Debug Draw
void App::FrustumDebugDraw(const Matrix& frustumView, const Matrix& frustumProj,
    const Matrix& renderView, const Matrix& renderProj, FXMVECTOR color)
{
    // Frustum Create
    BoundingFrustum frustum{};
    BoundingFrustum::CreateFromMatrix(frustum, frustumProj); // view space 기준
    Matrix invFrustumView = frustumView.Invert();
    frustum.Transform(frustum, invFrustumView);        // view -> world

    // Effect Update (render 기준은 항상 main camera)
    m_effect->SetWorld(Matrix::Identity);
    m_effect->SetView(renderView);
    m_effect->SetProjection(renderProj);
    m_effect->Apply(D3D::deviceContext.Get());

    // Stage Setting
    D3D::deviceContext.Get()->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    D3D::deviceContext.Get()->OMSetDepthStencilState(m_states->DepthNone(), 0);
    D3D::deviceContext.Get()->RSSetState(m_states->CullNone());
    D3D::deviceContext.Get()->IASetInputLayout(m_layout.Get());

    // Draw
    m_batch->Begin();
    Draw(m_batch.get(), frustum, color);
    m_batch->End();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK App::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    return __super::WindowProc(hWnd, message, wParam, lParam);
}