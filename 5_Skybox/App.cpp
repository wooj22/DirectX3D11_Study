#include "App.h"
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
	if (!D3DBase::Init(hWnd, screenWidth, screenHeight)) return false;
	if (!InitRenderPipeLine()) return false;
	if (!InitGUI()) return false;
	return true;
}

void App::OnUninit()
{
	UninitGUI();
	UninitRenderPipeLine();
	D3DBase::UnInit();
	CheckDXGIDebug();
}

void App::OnUpdate()
{
	float time = Time::GetTotalTime();

	// world update
	cube.Update();

	// view update
	camera.GetViewMatrix(view);
	//projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);
}

void App::OnRender()
{
	// RTV clear
	D3DBase::deviceContext->OMSetRenderTargets(1, D3DBase::renderTargetView.GetAddressOf(), D3DBase::depthStencilView.Get());
	D3DBase::deviceContext->ClearRenderTargetView(D3DBase::renderTargetView.Get(), clearColor);

	// death buffer clear
	D3DBase::deviceContext->ClearDepthStencilView(D3DBase::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// render pipeline stage setting
	D3DBase::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3DBase::deviceContext->PSSetSamplers(0, 1, &samplerState);

	// render
	skybox1.Render(view, projection);
	cube.Render(view, projection, light);

	// GUI
	RenderGUI();

	// present
	D3DBase::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
	cube.InitRenderPipeLine();
	skybox1.InitRenderPipeLine();
	
	// PS - smapler state create
	D3D11_SAMPLER_DESC sample_Desc = {};
	sample_Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;			// 상하좌우 텍셀 보간
	sample_Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;				// 0~1 범위를 벗어난 uv는 소수 부분만 사용
	sample_Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sample_Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sample_Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sample_Desc.MinLOD = 0;
	sample_Desc.MaxLOD = D3D11_FLOAT32_MAX;
	HR_T(D3DBase::device->CreateSamplerState(&sample_Desc, &samplerState));

	// Matrix Init
	// view init
	camera.GetViewMatrix(view);

	// projection init 
	projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

	return true;
}

void App::UninitRenderPipeLine()
{
	cube.UninitRenderPipeLine();
	skybox1.UninitRenderPipeLine();
	SAFE_RELEASE(samplerState);
}

bool App::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(D3DBase::device.Get(), D3DBase::deviceContext.Get());

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
	ImGui::Text("Directional Light");
	ImGui::InputFloat3("Direction(r)", &light.direction.x);

	ImGui::SliderAngle("x", &light.direction.x, -180.0f, 180.0f);
	ImGui::SliderAngle("y", &light.direction.y, -180.0f, 180.0f);
	ImGui::SliderAngle("z", &light.direction.z, -180.0f, 180.0f);
	ImGui::InputFloat3("Color", &light.color.x);

	ImGui::Text("Cube");
	ImGui::SliderAngle("Pitch", &cube.rotation.x, 0.0f, 360.0f);
	ImGui::SliderAngle("Yaw", &cube.rotation.y, 0.0f, 360.0f);
	ImGui::SliderAngle("Roll", &cube.rotation.z, 0.0f, 360.0f);

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
