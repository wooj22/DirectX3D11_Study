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

// Main process
bool App::OnInit()
{
    if (!D3D::Init(hWnd, screenWidth, screenHeight)) return false;
    if (!InitRenderPipeLine()) return false;
    if (!InitGUI()) return false;
    memory_debugger.Init();
    skybox1.InitRenderPipeLine(L"../Resource/skybox_cubmap.dds");

    // model init
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

    // view init
    camera.position = { 0, 80, -300 };
    camera.Far = 1000.0f;
    camera.moveSpeed = 300.f;
    camera.GetViewMatrix(view);

    // light init
    light.direction = { 0,-0.5,1 };
    light.color = { 1.0f, 0.9608f, 0.8980f, 1.0f };
    light.directIntensity = 5.0f;

    // LDR
    D3D::postprocessCBData.isHDR = 0;
    D3D::postprocessCBData.defalutGamma = 1.7;

    // projection init 
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    memory_debugger.CheakMemoryUsage();
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
    // model update(local, model matrix)
    character->Update();
    girl->Update();
    enemy->Update();
    camera.GetViewMatrix(view);

    // light update
    Vector3 sceneCenter = camera.position + camera.GetForward() * 300;
    Vector3 lightPos = sceneCenter - light.direction * 200;
    lightView = XMMatrixLookAtLH(lightPos, sceneCenter, { 0.0f, 0.0f, -1.0f });
    lightProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);
    lightProjection = XMMatrixOrthographicLH(screenWidth, screenHeight, camera.Near, camera.Far);

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
    // clear
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->ClearRenderTargetView(D3D::renderTargetView.Get(), clearColor);

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

    // Skybox render  --------------------------------
    skybox1.Render(view, projection);

    // buffer data -----------------------------------
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::transformCBData.shadowView = XMMatrixTranspose(lightView);
    D3D::transformCBData.shadowProjection = XMMatrixTranspose(lightProjection);

    D3D::lightingCBData.lightDirection = light.direction;
    D3D::lightingCBData.lightColor = light.color;
    D3D::lightingCBData.directIntensity = light.directIntensity;
    D3D::lightingCBData.cameraPos = camera.position;

    D3D::debugCBData.metallicFactor = metallicFactor;
    D3D::debugCBData.roughnessFactor = roughnessFactor;
    D3D::debugCBData.baseColorOverride = baseColorOverride;
    D3D::debugCBData.metallicOverride = metallicOverride;
    D3D::debugCBData.roughnessOverride = roughnessOverride;
    D3D::debugCBData.useBaseColorOverride = useBaseColorOverride ? 1 : 0;
    D3D::debugCBData.useMetallicOverride = useMetallicOverride ? 1 : 0;
    D3D::debugCBData.useRoughnessOverride = useRoughnessOverride ? 1 : 0;

    D3D::postprocessCBData.useDefalutGamma = usedefalutGamma ? 1 : 0;

    D3D::deviceContext->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::debugBuffer.Get(), 0, nullptr, &D3D::debugCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::postprocessBuffer.Get(), 0, nullptr, &D3D::postprocessCBData, 0, 0);


    // 1. Depth Only Pass -------------------------------------
    // static, rigid model
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_shadowMap);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(0, nullptr, D3D::shadowDSV.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->ClearDepthStencilView(D3D::shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::ShadowDepth_Static_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(nullptr, NULL, 0);

    zelda->Render();
    character->Render();

    // skeletal model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::ShadowDepth_Skinned_VS.Get(), NULL, 0);
    girl->Render();
    enemy->Render();


    // 2. PBR Mesh Render Pass -------------------------------------
    // static, rigid model
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    //D3D::deviceContext->OMSetDepthStencilState(D3D::depthStencilState.Get(), 0);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Static_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PBR_PS.Get(), NULL, 0); 
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    floor->Render();
    zelda->Render();
    character->Render();

    // skeletal model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Skinned_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    girl->Render();
    enemy->Render();

    // 다음 shadowpass에서 SRV를 DSV로 다시 쓰기 위해 연결 해제
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(6, 1, nullSRV);

    // GUI
    RenderGUI();

    // present
    D3D::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
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

    ImGui::Begin("Inspertor", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Text("[Light]");
    ImGui::SliderFloat3("Direction", &light.direction.x, -1.0f, 1.0f, "%.2f");
    ImGui::ColorEdit3("Color", &light.color.x);
    ImGui::SliderFloat("Intensity", &light.directIntensity, 0.0f, 10.0f);

    ImGui::Text("[Material]");
    ImGui::SliderFloat("Metallic Factor", &metallicFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness Factor", &roughnessFactor, 0.0f, 1.0f);
    ImGui::Checkbox("BaseColor Override", &useBaseColorOverride);
    ImGui::ColorEdit3("BaseColor", &baseColorOverride.x);

    ImGui::Checkbox("Metallic Override", &useMetallicOverride);
    ImGui::SliderFloat("Metallic", &metallicOverride, 0.0f, 1.0f);
    ImGui::Checkbox("Roughness Override", &useRoughnessOverride);
    ImGui::SliderFloat("Roughness", &roughnessOverride, 0.0f, 1.0f);

    ImGui::Text("[Models]");
    ImGui::InputFloat3("position", &character->position.x);
    ImGui::SliderAngle("Pitch", &character->rotation.x, 0.0f, 360.0f);
    ImGui::SliderAngle("Yaw", &character->rotation.y, 0.0f, 360.0f);
    ImGui::SliderAngle("Roll", &character->rotation.z, 0.0f, 360.0f);
    ImGui::InputFloat3("scale", &character->scale.x);

    ImGui::End();

    // Gamma
    ImGui::Begin("[Gamma]");
    ImGui::Checkbox("useGamma", &usedefalutGamma);
    ImGui::SliderFloat("Gamma", &D3D::postprocessCBData.defalutGamma, 0.f, 5.0f);
    ImGui::End();

    ImGui::Begin("[Memory Debugger]");
    ImGui::Text("[T] Trim");
    ImGui::Text("%ls", memory_debugger.GetMemoryUsageWstring().c_str());
    ImGui::End();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK App::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    return __super::WindowProc(hWnd, message, wParam, lParam);
}
