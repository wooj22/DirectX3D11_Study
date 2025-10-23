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
	if (!D3D::Init(hWnd, screenWidth, screenHeight)) return false;
	if (!InitRenderPipeLine()) return false;
	if (!InitGUI()) return false;
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
	// world udpate
	character->Update();
	zelda->Update();
	tree->Update();

	// view update
	camera.GetViewMatrix(view);
}

void App::OnRender()
{
	// RTV clear
	D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), D3D::depthStencilView.Get());
	D3D::deviceContext->ClearRenderTargetView(D3D::renderTargetView.Get(), clearColor);

	// death buffer clear
	D3D::deviceContext->ClearDepthStencilView(D3D::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// render pipeline stage setting
	D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D::deviceContext->IASetInputLayout(inputLayout);
	D3D::deviceContext->VSSetShader(vertexShader, NULL, 0);
	D3D::deviceContext->PSSetShader(pixelShader, NULL, 0);
	D3D::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	D3D::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
	D3D::deviceContext->PSSetSamplers(0, 1, D3D::samplerState.GetAddressOf());

	ConstantBuffer cb;
	cb.view = XMMatrixTranspose(view);
	cb.projection = XMMatrixTranspose(projection);
	cb.lightDirection = light.direction;
	cb.lightColor = light.color;
	cb.indirectLight = light.indirectLight;
	cb.directLight = light.directLight;
	cb.ambientHighlight = ambientHighlight;
	cb.diffuseHighlight = diffuseHighlight;
	cb.specularHighlight = specularHighlight;
	cb.shininess = shininess;
	cb.cameraPos = camera.position;

	// render
	character->Render(constantBuffer, cb);
	zelda->Render(constantBuffer, cb);
	tree->Render(constantBuffer, cb);

	// GUI
	RenderGUI();

	// present
	D3D::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
	// model init
	character = ModelLoder::LoadStaticMesh("../Resource/Character.fbx");
	zelda = ModelLoder::LoadStaticMesh("../Resource/zeldaPosed001.fbx");
	tree = ModelLoder::LoadStaticMesh("../Resource/Tree.fbx");

	//zelda->SetPosition({5,0,0});
	//tree->SetPosition({ -5,0,0 });

	// IA - input layout create
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 48,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D10Blob* vertexShaderBuffer = nullptr;		// vs mapping
	HR_T(CompileShaderFromFile(L"VS_Basic.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(D3D::device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout));

	// VS - vertex shader create
	HR_T(D3D::device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader));
	SAFE_RELEASE(vertexShaderBuffer);

	// PS - pixel shader create
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"PS_Basic.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader));
	SAFE_RELEASE(pixelShaderBuffer);

	// Constant Buffer create
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3D::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

	// Matrix Init
	// view init
	camera.position.z = -50;
	camera.Far = 1000.0f;
	camera.moveSpeed = 200.f;
	camera.GetViewMatrix(view);

	// projection init 
	projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

	return true;
}

void App::UninitRenderPipeLine()
{
	
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

	ImGui::Text("Models");
	ImGui::Text("Character");
	ImGui::InputFloat3("position", &character->position.x);
	ImGui::SliderAngle("Pitch", &character->rotation.x, 0.0f, 360.0f);
	ImGui::SliderAngle("Yaw", &character->rotation.y, 0.0f, 360.0f);
	ImGui::SliderAngle("Roll", &character->rotation.z, 0.0f, 360.0f);
	ImGui::InputFloat3("scale", &character->scale.x);	

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
