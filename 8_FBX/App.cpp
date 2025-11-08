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

	// model init
	character = ModelLoader::LoadStaticMesh("../Resource/Character.fbx");
	zelda = ModelLoader::LoadStaticMesh("../Resource/zeldaPosed001.fbx");
	tree = ModelLoader::LoadStaticMesh("../Resource/Tree.fbx");
	boxHuman = ModelLoader::LoadRigidMesh("../Resource/BoxHuman.fbx");
    skinningTest = ModelLoader::LoadSkeletalMesh("../Resource/SkinningTest.fbx");

	character->SetPosition({ -100, 0, 0 });
	zelda->SetPosition({ 0, 0, 0 });
	tree->SetPosition({ 100, 0, 0 });
	tree->SetScale({ 100,100,100 });
	boxHuman->SetPosition({ 200,0,0 });
	boxHuman->SetScale({ 0.2,0.2,0.2 });
    skinningTest->SetPosition({ 300, 0,0 });

	// view init
	camera.position = { 70, 50, -200 };
	camera.Far = 1000.0f;
	camera.moveSpeed = 300.f;
	camera.GetViewMatrix(view);

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
	// world udpate
	character->Update();
	zelda->Update();
	tree->Update();
	boxHuman->Update();
    skinningTest->Update();

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
	D3D::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    D3D::deviceContext->VSSetConstantBuffers(1, 1, &offsetMatrixCB);
    D3D::deviceContext->VSSetConstantBuffers(2, 1, &poseMatrixCB);
	D3D::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
	D3D::deviceContext->PSSetSamplers(0, 1, D3D::samplerState.GetAddressOf());
	D3D::deviceContext->OMSetBlendState(D3D::blendState.Get(), blendFactor, sampleMask);

	// constant buffer
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

    OffsetMatrixCB boneOffsetCB;
    PoseMatrixCB bonePoseCB;

	// render
	// 불투명 모델
	D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->IASetInputLayout(inputLayout);
    D3D::deviceContext->VSSetShader(VS_Basic, NULL, 0);
    D3D::deviceContext->PSSetShader(PS_Toon, NULL, 0);
    zelda->Render(constantBuffer, cb);

    D3D::deviceContext->PSSetShader(PS_Basic, NULL, 0);
	character->Render(constantBuffer, cb);
	boxHuman->Render(constantBuffer, cb);

    D3D::deviceContext->VSSetShader(VS_Skinning, NULL, 0);
    D3D::deviceContext->IASetInputLayout(inputLayout_weight);
    skinningTest->Render(constantBuffer, offsetMatrixCB, poseMatrixCB, cb, boneOffsetCB, bonePoseCB);

	// 투명 모델
	// 만약 모델이 여러개 있다면 Back to Front 순서 렌더 (카메라에서 먼 것부터 렌더링되도록 정렬하여 렌더링)
    D3D::deviceContext->OMSetDepthStencilState(D3D::depthStencilState.Get(), 0);
    D3D::deviceContext->IASetInputLayout(inputLayout);
    D3D::deviceContext->VSSetShader(VS_Basic, NULL, 0);

	tree->Render(constantBuffer, cb);

	// GUI
	RenderGUI();

	// present
	D3D::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
	// IA - input layout create
    // basic
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 48,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D10Blob* vertexShaderBuffer = nullptr;		// vs mapping
	HR_T(CompileShaderFromFile(L"VS_Basic.hlsl", "main", "vs_5_0", &vertexShaderBuffer));
	HR_T(D3D::device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout));

    // skinned
    D3D11_INPUT_ELEMENT_DESC layout2[] =
    {   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
        { "POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT"     , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BITANGENT"   , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT     , 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_INDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ID3D10Blob* vertexShaderBuffer2 = nullptr;		// vs mapping
    HR_T(CompileShaderFromFile(L"VS_Skinning.hlsl", "main", "vs_5_0", &vertexShaderBuffer2));
    HR_T(D3D::device->CreateInputLayout(layout2, ARRAYSIZE(layout2),
        vertexShaderBuffer2->GetBufferPointer(), vertexShaderBuffer2->GetBufferSize(), &inputLayout_weight));


	// VS - vertex shader create
    // basic
	HR_T(D3D::device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &VS_Basic));
	SAFE_RELEASE(vertexShaderBuffer);

    // skinned
    HR_T(D3D::device->CreateVertexShader(vertexShaderBuffer2->GetBufferPointer(),
        vertexShaderBuffer2->GetBufferSize(), NULL, &VS_Skinning));
    SAFE_RELEASE(vertexShaderBuffer2);


	// PS - pixel shader create
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"PS_Basic.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
	HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &PS_Basic));

    HR_T(CompileShaderFromFile(L"PS_Toon.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
    HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
        pixelShaderBuffer->GetBufferSize(), NULL, &PS_Toon));
    SAFE_RELEASE(pixelShaderBuffer);

	// Constant Buffer create
    // basic
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3D::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

    // offsetMatrixCB
    D3D11_BUFFER_DESC constBuffer_Desc2 = {};
    constBuffer_Desc2.Usage = D3D11_USAGE_DEFAULT;
    constBuffer_Desc2.ByteWidth = sizeof(OffsetMatrixCB);
    constBuffer_Desc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBuffer_Desc2.CPUAccessFlags = 0;
    HR_T(D3D::device->CreateBuffer(&constBuffer_Desc2, nullptr, &offsetMatrixCB));

    // poseMatrixCB
    D3D11_BUFFER_DESC constBuffer_Desc3 = {};
    constBuffer_Desc3.Usage = D3D11_USAGE_DEFAULT;
    constBuffer_Desc3.ByteWidth = sizeof(PoseMatrixCB);
    constBuffer_Desc3.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBuffer_Desc3.CPUAccessFlags = 0;
    HR_T(D3D::device->CreateBuffer(&constBuffer_Desc3, nullptr, &poseMatrixCB));

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
	ImGui::InputFloat3("position", &skinningTest->position.x);
	ImGui::SliderAngle("Pitch", &skinningTest->rotation.x, 0.0f, 360.0f);
	ImGui::SliderAngle("Yaw", &skinningTest->rotation.y, 0.0f, 360.0f);
	ImGui::SliderAngle("Roll", &skinningTest->rotation.z, 0.0f, 360.0f);
	ImGui::InputFloat3("scale", &skinningTest->scale.x);

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
