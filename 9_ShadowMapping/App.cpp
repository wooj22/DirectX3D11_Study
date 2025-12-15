#include "App.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Helper.h"
#include "../WinBase/Camera.h"
#include "../WinBase/AssetManager.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>

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
    skybox1.InitRenderPipeLine(L"../Resource/skybox_cubmap.dds");

    // model init
    warrior = new SkeletalModel();
    AssetManager::Instance().LoadSkeletalModelAsset(warrior, "../Resource/Girl.fbx");

    character = new SkeletalModel();
    AssetManager::Instance().LoadSkeletalModelAsset(character, "../Resource/Enemy.fbx");
    character->SetPosition({ 200, 0,0 });

    zelda = new StaticModel();
    AssetManager::Instance().LoadStaticModelAsset(zelda, "../Resource/zeldaPosed001.fbx");

    tree = new StaticModel();
    AssetManager::Instance().LoadStaticModelAsset(tree, "../Resource/Tree.fbx");

    boxHuman = new RigidModel();
    AssetManager::Instance().LoadRigidModelAsset(boxHuman, "../Resource/BoxHuman.fbx");
    zelda->SetPosition({ -150, 0,0 });
    boxHuman->SetPosition({ -320, 0,0 });
    boxHuman->SetScale({ 0.2,0.2,0.2 });
    tree->SetPosition({ -450, 0,0 });
    tree->SetScale({ 100,100,100 });

    plane = new StaticModel();
    ModelLoader::LoadStaticMesh(plane, "../Resource/Plane.fbx");
    plane->SetPosition({ 0,-5,0 });
    plane->SetScale({ 0.4,1,0.2 });

    // view init
    camera.position = { -200, 100, -500 };
    camera.Far = 1000.0f;
    camera.moveSpeed = 300.f;
    camera.GetViewMatrix(view);

    // light init
    light.color = { 1.0f, 0.9608f, 0.8980f, 1.0f };
    light.indirectIntensity = 0.3f;

    // projection init 
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, 0.1f, 2000.0f);

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
    character->Update();
    zelda->Update();
    boxHuman->Update();
    tree->Update();
    plane->Update();
    camera.GetViewMatrix(view);

    // Light view, projection
    Vector3 sceneCenter = camera.position + camera.GetForward() * 300;
    Vector3 lightPos = sceneCenter - light.direction * 200;
    lightView = XMMatrixLookAtLH(lightPos, sceneCenter, { 0.0f, 0.0f, -1.0f });
    lightProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);
    lightProjection = XMMatrixOrthographicLH(screenWidth, screenHeight, camera.Near, camera.Far);
}

void App::OnRender()
{
    // clear
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->ClearRenderTargetView(D3D::renderTargetView.Get(), clearColor);
    D3D::deviceContext->ClearDepthStencilView(D3D::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    

    // --------------------- stage setting -----------------------
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());
    D3D::deviceContext->PSSetSamplers(1, 1, D3D::shadowSamplerState.GetAddressOf());
    D3D::deviceContext->OMSetBlendState(D3D::alphaBlendState.Get(), blendFactor, sampleMask);

    // constant buffer
    D3D::deviceContext->VSSetConstantBuffers(0, 1, D3D::transformBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(1, 1, D3D::lightingBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(2, 1, D3D::materialBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(3, 1, D3D::offsetMatrixBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(4, 1, D3D::poseMatrixBuffer.GetAddressOf());

    // skybox render 
    skybox1.Render(view, projection);

    // buffer data
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::transformCBData.shadowView = XMMatrixTranspose(lightView);
    D3D::transformCBData.shadowProjection = XMMatrixTranspose(lightProjection);

    D3D::lightingCBData.lightDirection = light.direction;
    D3D::lightingCBData.lightColor = light.color;
    D3D::lightingCBData.indirectIntensity = light.indirectIntensity;
    D3D::lightingCBData.directIntensity = light.directIntensity;
    D3D::lightingCBData.ambientHighlight = ambientHighlight;
    D3D::lightingCBData.diffuseHighlight = diffuseHighlight;
    D3D::lightingCBData.specularHighlight = specularHighlight;
    D3D::lightingCBData.shininess = shininess;
    D3D::lightingCBData.cameraPos = camera.position;
    D3D::deviceContext->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);


    // Model Render
    // 1. Depth Only Pass
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_shadowMap);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(0, nullptr, D3D::shadowDSV.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->ClearDepthStencilView(D3D::shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    D3D::deviceContext->VSSetShader(D3D::ShadowDepth_Skinned_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(nullptr, nullptr, 0);
    warrior->Render();
    character->Render();

    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::ShadowDepth_Static_VS.Get(), NULL, 0);
    zelda->Render();
    boxHuman->Render();
    plane->Render();
    tree->Render();

    // 2. Render Pass
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Skinned_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::BlinnPhong_PS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    warrior->Render();
    character->Render();

    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::BaseLit_Static_VS.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    zelda->Render();
    boxHuman->Render();
    plane->Render();

    D3D::deviceContext->OMSetDepthStencilState(D3D::wirteoffDSS.Get(), 0);
    tree->Render();

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
    ImGui::Text("Light");
    ImGui::SliderFloat3("Direction", &light.direction.x, -1.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("indirectLight", &light.indirectIntensity, 0.0f, 50.0f, "%.2f");
    ImGui::SliderFloat("directLight", &light.directIntensity, 0.0f, 50.0f, "%.2f");
    ImGui::ColorEdit3("Color", &light.color.x);

    ImGui::SliderFloat("ambientHighlight", &ambientHighlight, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("diffuseHighlight", &diffuseHighlight, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("specularHighlight", &specularHighlight, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("shininess", &shininess, 0.0f, 3000.0f, "%.2f");

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
