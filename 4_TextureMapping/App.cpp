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

// Vertex Structure
struct Vertex
{
	Vector3 position;		 // vertex 위치
	Vector3 normal;			 // vertex 법선
	Vector2 texcoord;		 // uv 좌표

	Vertex(Vector3 position, Vector3 normal, Vector2 uv)
		: position(position), normal(normal), texcoord(uv) {}
};

// ConstantBuffer
struct alignas(16) ConstantBuffer
{
	Matrix world;					// world 행렬
	Matrix view;		 			// view 행렬
	Matrix projection;				// projection 행렬

	Vector4 lightDirection;         // directional light의 방향 벡터
	Vector4 lightColor;				// directional light 색상
};

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
	time = Time::GetTotalTime();

	// world update
	// cube
	Matrix t1 = XMMatrixTranslationFromVector(cube.position);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(cube.rotation.x, cube.rotation.y, cube.rotation.z);
	Matrix r1 = XMMatrixRotationQuaternion(q);
	cube.world = r1 * t1;

	// view update
	camera.GetViewMatrix(view);
}

void App::OnRender()
{
	// RTV clear
	D3DBase::deviceContext->OMSetRenderTargets(1, D3DBase::renderTargetView.GetAddressOf(), depthStencilView);
	D3DBase::deviceContext->ClearRenderTargetView(D3DBase::renderTargetView.Get(), clearColor);

	// death buffer clear
	D3DBase::deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// render pipeline bind (stage setting)
	D3DBase::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3DBase::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferStride, &vertexBufferOffset);
	D3DBase::deviceContext->IASetInputLayout(inputLayout);
	D3DBase::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	D3DBase::deviceContext->VSSetShader(vertexShader, NULL, 0);
	D3DBase::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	D3DBase::deviceContext->PSSetShader(pixelShader, NULL, 0);
	D3DBase::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
	D3DBase::deviceContext->PSSetShaderResources(0, 1, &textureRV);
	D3DBase::deviceContext->PSSetSamplers(0, 1, &samplerState);

	// render
	// cube
	ConstantBuffer constBuffer;
	constBuffer.world = XMMatrixTranspose(cube.world);
	constBuffer.view = XMMatrixTranspose(view);
	constBuffer.projection = XMMatrixTranspose(projection);
	constBuffer.lightDirection = light.direction;
	constBuffer.lightColor = light.color;
	D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &constBuffer, 0, 0);
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);

	// GUI
	RenderGUI();

	// present
	D3DBase::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
	// IA - vertex buffer create
	// Vertex가 normal벡터 정보를 가져야하므로 정육면체의 각 면마다의 vertex 정보를 넣어주어야 한다.
	Vertex vertices[] =
	{
		// Top (+Y)
		Vertex({-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}),
		Vertex({ 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}),
		Vertex({ 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}),
		Vertex({-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}),

		// Bottom (-Y)
		Vertex({-1.0f, -1.0f, -1.0f}, {0.0f,-1.0f, 0.0f}, {0.0f, 0.0f}),
		Vertex({ 1.0f, -1.0f, -1.0f}, {0.0f,-1.0f, 0.0f}, {1.0f, 0.0f}),
		Vertex({ 1.0f, -1.0f,  1.0f}, {0.0f,-1.0f, 0.0f}, {1.0f, 1.0f}),
		Vertex({-1.0f, -1.0f,  1.0f}, {0.0f,-1.0f, 0.0f}, {0.0f, 1.0f}),

		// Left (-X)
		Vertex({-1.0f, -1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
		Vertex({-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
		Vertex({-1.0f,  1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}),
		Vertex({-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),

		// Right (+X)
		Vertex({ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
		Vertex({ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
		Vertex({ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}),
		Vertex({ 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),

		// Back (-Z)
		Vertex({-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}),
		Vertex({ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}),
		Vertex({ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}),
		Vertex({-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}),

		// Front (+Z)
		Vertex({-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}),
		Vertex({ 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}),
		Vertex({ 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}),
		Vertex({-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f})
	};


	D3D11_BUFFER_DESC vertexBuffer_Desc = {};
	vertexBuffer_Desc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);		// buffer size
	vertexBuffer_Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;					// bind 용도
	vertexBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;							// buffer 사용 용도	

	D3D11_SUBRESOURCE_DATA vertexBuffer_Data = {};						    // vertex data
	vertexBuffer_Data.pSysMem = vertices;
	vertexBufferStride = sizeof(Vertex);
	vertexBufferOffset = 0;

	HR_T(D3DBase::device->CreateBuffer(&vertexBuffer_Desc, &vertexBuffer_Data, &vertexBuffer));


	// IA - index buffer create
	// 삼각형 그리는 순서(CW : 시계방향)
	WORD indices[] =
	{
		3,1,0,	  2,1,3,		// top
		6,4,5,	  7,4,6,		// bottom
		11,9,8,	  10,9,11,		// left
		14,12,13, 15,12,14,		// right
		19,17,16, 18,17,19,     // back
		22,20,21, 23,20,22		// front
	};

	D3D11_BUFFER_DESC indexBuffer_Desc = {};
	indexBuffer_Desc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);		// buffer size
	indexBuffer_Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;			    // bind 용도		
	indexBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;						// buffer 사용 용도

	D3D11_SUBRESOURCE_DATA indexBuffer_Data = {};						// index data
	indexBuffer_Data.pSysMem = indices;
	indexCount = ARRAYSIZE(indices);

	HR_T(D3DBase::device->CreateBuffer(&indexBuffer_Desc, &indexBuffer_Data, &indexBuffer));


	// IA - input layout create
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D10Blob* vertexShaderBuffer = nullptr;		// vs mapping
	HR_T(CompileShaderFromFile(L"VertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(D3DBase::device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout));

	// VS - vertex shader create
	HR_T(D3DBase::device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader));
	SAFE_RELEASE(vertexShaderBuffer);

	// PS - pixel shader create
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"PixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(D3DBase::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader));
	SAFE_RELEASE(pixelShaderBuffer);

	// PS - texture load
	HR_T(CreateDDSTextureFromFile(D3DBase::device.Get(), L"../Resource/seafloor.dds", nullptr, &textureRV));

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

	// OM - depth stencil view create
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = screenWidth;
	descDepth.Height = screenHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> pTextureDepthStencil;
	HR_T(D3DBase::device->CreateTexture2D(&descDepth, nullptr, pTextureDepthStencil.GetAddressOf()));

	// create
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(D3DBase::device->CreateDepthStencilView(pTextureDepthStencil.Get(), &descDSV, &depthStencilView));

	// constant buffer create (vs, ps에 전달할 사용할 행렬 data)
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3DBase::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

	// Object Init
	cube.Init();
	cube.rotation = { 0, 45, 0 };

	// Matrix Init
	// view init
	camera.GetViewMatrix(view);

	// projection init 
	projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

	return true;
}

void App::UninitRenderPipeLine()
{
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(constantBuffer);
	SAFE_RELEASE(inputLayout);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
	SAFE_RELEASE(depthStencilView);
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

	ImGui::SliderAngle("x", &light.direction.x, 0.0f, 360.0f);
	ImGui::SliderAngle("y", &light.direction.y, 0.0f, 360.0f);
	ImGui::SliderAngle("z", &light.direction.z, 0.0f, 360.0f);
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
