#include "App.h"
#include "../WinBase/Helper.h"
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define USE_FLIPMODE 1

// Vertex Structure
struct Vertex
{
	Vector3 position;    // vertex 위치
	//Vector3 normal;      // vertex 법선

	Vertex(Vector3 position) : position(position) {}
	//Vertex(Vector3 position, Vector3 normal) : position(position), normal(normal) {}
};

// 정점 쉐이더에 전달할 상수 버퍼
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
	// cube 1
	Matrix t1 = XMMatrixTranslationFromVector(cube1.position);
	Matrix r1 = XMMatrixRotationY(-time);
	cube1.world = r1 * t1;

	// cube 2
	Matrix t2 = XMMatrixTranslationFromVector(cube2.position);
	Matrix r2 = XMMatrixRotationY(time * 3);
	Matrix s2 = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	cube2.world = s2 * r2 * t2 * cube1.world;
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

	// render
	// cube 1
	ConstantBuffer cube_constBuffer;
	cube_constBuffer.world = XMMatrixTranspose(cube1.world);
	cube_constBuffer.view = XMMatrixTranspose(view);
	cube_constBuffer.projection = XMMatrixTranspose(projection);
	D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cube_constBuffer, 0, 0);
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);

	// cube 2
	cube_constBuffer.world = XMMatrixTranspose(cube2.world);
	D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cube_constBuffer, 0, 0);
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);

	// GUI
	RenderGUI();

	// present
	D3DBase::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
	// IA - vertex buffer create
	Vertex vertices[] =
	{
		// index buffer를 사용하므로 cube정점 8개 data만 넣으면 된다.
		Vertex(Vector3(0,0,0)),
		Vertex(Vector3(1,0,0)),
		Vertex(Vector3(0,1,0)),
		Vertex(Vector3(1,1,0)),
		Vertex(Vector3(0,0,1)),
		Vertex(Vector3(1,0,1)),
		Vertex(Vector3(0,1,1)),
		Vertex(Vector3(1,1,1)),
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
	// 삼각형 정점 순서(CCW : 반시계방향, 앞면)
	// 만약 삼각형 그리는 순서를 시계방향(CW)로 그리면 뒷면이 그려질 수 있음
	WORD indices[] =
	{
		0,2,1, 2,3,1,		// front
		4,5,6, 5,7,6,		// back
		0,4,2, 4,6,2,		// left
		1,3,5, 3,7,5,		// right
		2,6,3, 6,7,3,		// up
		0,1,4, 1,5,4		// down
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

	// constant buffer create (vs에 전달할 사용할 행렬 data)
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3DBase::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

	// Object Init
	cube1.Init();
	cube2.Init();
	cube2.position = { 3, 0, 0 };

	// Camera Init
	

	// Matrix Init
	// view init - camera 역행렬 (view행렬)
	view = XMMatrixLookAtLH(camera.eye, camera.at, camera.up);

	// projection init 
	// XMMatrixPerspectiveFovLH(Fov, AspectRatio, NearZ, FarZ)
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
	ImGui::Text("Cube Positions");
	ImGui::InputFloat3("Cube1", &cube1.position.x);
	ImGui::InputFloat3("Cube2", &cube2.position.x);

	ImGui::Text("Camera Settings");
	ImGui::InputFloat3("Eye", &camera.eye.x);
	ImGui::InputFloat3("At", &camera.at.x);
	ImGui::InputFloat3("Up", &camera.up.x);
	ImGui::SliderAngle("FOV Y", &camera.FovY, 30.0f, 120.0f);
	ImGui::InputFloat("Near Plane", &camera.Near);
	ImGui::InputFloat("Far Plane", &camera.Far);

	// matrix udpate
	cube1.world = XMMatrixTranslationFromVector(cube1.position);
	cube2.world = XMMatrixTranslationFromVector(cube2.position);
	view = XMMatrixLookAtLH(camera.eye, camera.at, camera.up);
	projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (float)screenHeight, camera.Near, camera.Far);

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
