#include "App.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Helper.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>
#include "../WinBase/Camera.h"

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
    skybox.InitRenderPipeLine();

    // model init
    warrior = new SkeletalModel();
    enemy = new SkeletalModel();
    ModelLoader::LoadSkeletalMesh(warrior, "../Resource/Girl.fbx");
    ModelLoader::LoadSkeletalMesh(enemy, "../Resource/Enemy.fbx");
    enemy->SetPosition({ 200, 0,0 });

    // view init
    camera.position = { 70, 80, -300 };
    camera.Far = 1000.0f;
    camera.moveSpeed = 300.f;
    camera.GetViewMatrix(view);

    // light init
    light.indirectLight = 0.1;
    light.directLight = 1.3;
    light.direction = { 0,0,1 };
    light.color = { 1.0f, 0.9608f, 0.8980f, 1.0f };

    // projection init 
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

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
    warrior->Update();
    enemy->Update();
    camera.GetViewMatrix(view);
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
    D3D::deviceContext->VSSetConstantBuffers(2, 1, D3D::materialBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(2, 1, D3D::materialBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(3, 1, D3D::offsetMatrixBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(4, 1, D3D::poseMatrixBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(5, 1, D3D::outlineBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(5, 1, D3D::outlineBuffer.GetAddressOf());

    // skybox render 
    skybox.Render(view, projection);

    // buffer data
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::lightingCBData.lightDirection = light.direction;
    D3D::lightingCBData.lightColor = light.color;
    D3D::lightingCBData.indirectLight = light.indirectLight;
    D3D::lightingCBData.directLight = light.directLight;
    D3D::lightingCBData.ambientHighlight = ambientHighlight;
    D3D::lightingCBData.diffuseHighlight = diffuseHighlight;
    D3D::lightingCBData.specularHighlight = specularHighlight;
    D3D::lightingCBData.shininess = shininess;
    D3D::lightingCBData.cameraPos = camera.position;
    D3D::deviceContext->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::outlineBuffer.Get(), 0, nullptr, &D3D::outlineCBData, 0, 0);

    // Skeletal Mesh Render-------------------------------------
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->PSSetShaderResources(4, 1, &diffuseRampTexture);
    D3D::deviceContext->PSSetShaderResources(5, 1, &specualrRampTexture);

    // [Model 1]
    // outline render
    D3D::deviceContext->VSSetShader(D3D::Skinned_OutLine_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::OutLine_PS.Get(), NULL, 0);
    D3D::deviceContext->RSSetState(D3D::rasterizerState.Get());
    warrior->Render();
    D3D::deviceContext->RSSetState(nullptr);

    // model render
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Skinned_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::BlinnPhongToon_PS.Get(), NULL, 0);
    warrior->Render();

    // [Model 2]
    // outline render
    D3D::deviceContext->VSSetShader(D3D::Skinned_OutLine_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::OutLine_PS.Get(), NULL, 0);
    D3D::deviceContext->RSSetState(D3D::rasterizerState.Get());
    enemy->Render();
    D3D::deviceContext->RSSetState(nullptr);

    // model render
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Skinned_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::BlinnPhongToon_PS.Get(), NULL, 0);
    enemy->Render();

    // GUI
    RenderGUI();

    // present
    D3D::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
    // RampTexture Create
    CreateTextureFromFile(D3D::device.Get(), L"../Resource/RampTexture.png", &diffuseRampTexture);
    CreateTextureFromFile(D3D::device.Get(), L"../Resource/SpecularRampTexture.png", &specualrRampTexture);

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
    ImGui::SliderFloat("indirectLight", &light.indirectLight, 0.0f, 50.0f, "%.2f");
    ImGui::SliderFloat("directLight", &light.directLight, 0.0f, 50.0f, "%.2f");
    ImGui::ColorEdit3("Color", &light.color.x);

    ImGui::SliderFloat("ambientHighlight", &ambientHighlight, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("diffuseHighlight", &diffuseHighlight, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("specularHighlight", &specularHighlight, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("shininess", &shininess, 0.0f, 3000.0f, "%.2f");

    ImGui::Text("OutLine");
    ImGui::SliderFloat("outlineThickness", &D3D::outlineCBData.outlineThickness, 0.0f, 2.0f, "%.2f");
    ImGui::ColorEdit3("outlineColoe", &D3D::outlineCBData.outlineColor.x);

    ImGui::Text("Models");
    ImGui::InputFloat3("position", &warrior->position.x);
    ImGui::SliderAngle("Pitch", &warrior->rotation.x, 0.0f, 360.0f);
    ImGui::SliderAngle("Yaw", &warrior->rotation.y, 0.0f, 360.0f);
    ImGui::SliderAngle("Roll", &warrior->rotation.z, 0.0f, 360.0f);
    ImGui::InputFloat3("scale", &warrior->scale.x);

    ImGui::End();
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
