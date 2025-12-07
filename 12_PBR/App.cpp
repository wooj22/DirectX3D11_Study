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
    debugger.Init();
    skybox.InitRenderPipeLine();

    // model init
    character = new RigidModel();
    AssetManager::Instance().LoadRigidModelAsset(character, "../Resource/char.fbx");

    // view init
    camera.position = { 0, 80, -300 };
    camera.Far = 1000.0f;
    camera.moveSpeed = 300.f;
    camera.GetViewMatrix(view);

    // light init
    light.direction = { 0,0,1 };
    light.color = { 1.0f, 0.9608f, 0.8980f, 1.0f };

    // projection init 
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    debugger.CheakMemoryUsage();
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
    character->Update();
    camera.GetViewMatrix(view);

    // Memory Cheak
    debugger.CheakMemoryUsage();

    // Trim
    if (Input::GetKeyDown('T'))
    {
        debugger.Trim();
    }
}

void App::OnRender()
{
    // RTV clear
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->ClearRenderTargetView(D3D::renderTargetView.Get(), clearColor);

    // death buffer clear
    D3D::deviceContext->ClearDepthStencilView(D3D::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // --------------------- Stage Setting -----------------------
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->PSSetSamplers(0, 1, D3D::samplerState.GetAddressOf());
    D3D::deviceContext->OMSetBlendState(D3D::blendState.Get(), blendFactor, sampleMask);

    // constant buffer
    D3D::deviceContext->VSSetConstantBuffers(0, 1, D3D::transformBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(0, 1, D3D::transformBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(1, 1, D3D::lightingBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(1, 1, D3D::lightingBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(2, 1, D3D::materialBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(6, 1, D3D::debugBuffer.GetAddressOf());

    // skybox render 
    skybox.Render(view, projection);

    // buffer data
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::lightingCBData.lightDirection = light.direction;
    D3D::lightingCBData.lightColor = light.color;
    D3D::lightingCBData.cameraPos = camera.position;
    D3D::debugCBData.metallicFactor = metallicFactor;
    D3D::debugCBData.roughnessFactor = roughnessFactor;
    D3D::debugCBData.metallicOverride = metallicOverride;
    D3D::debugCBData.roughnessOverride = roughnessOverride;
    D3D::debugCBData.useMetallicOverride = useMetallicOverride ? 1 : 0;
    D3D::debugCBData.useRoughnessOverride = useRoughnessOverride ? 1 : 0;
    D3D::deviceContext->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::debugBuffer.Get(), 0, nullptr, &D3D::debugCBData, 0, 0);

    // Mesh Render-------------------------------------
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());

    // rigid model
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Static_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PBR_PS.Get(), NULL, 0);
    character->Render();

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
    skybox.UninitRenderPipeLine();
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
    ImGui::Text("Light");
    ImGui::SliderFloat3("Direction", &light.direction.x, -1.0f, 1.0f, "%.2f");
    ImGui::ColorEdit3("Color", &light.color.x);

    ImGui::Text("Material");
    ImGui::SliderFloat("Metallic Factor", &metallicFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness Factor", &roughnessFactor, 0.0f, 1.0f);
    ImGui::Checkbox("Metallic Override", &useMetallicOverride);
    ImGui::SliderFloat("Metallic", &metallicOverride, 0.0f, 1.0f);
    ImGui::Checkbox("Roughness Override", &useRoughnessOverride);
    ImGui::SliderFloat("Roughness", &roughnessOverride, 0.0f, 1.0f);

    ImGui::Text("Models");
    ImGui::InputFloat3("position", &character->position.x);
    ImGui::SliderAngle("Pitch", &character->rotation.x, 0.0f, 360.0f);
    ImGui::SliderAngle("Yaw", &character->rotation.y, 0.0f, 360.0f);
    ImGui::SliderAngle("Roll", &character->rotation.z, 0.0f, 360.0f);
    ImGui::InputFloat3("scale", &character->scale.x);

    ImGui::End();

    ImGui::Begin("Memory Debugger");
    ImGui::Text("[T] Trim");
    ImGui::Text("%ls", debugger.GetMemoryUsageWstring().c_str());
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
