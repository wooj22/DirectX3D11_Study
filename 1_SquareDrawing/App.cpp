#include "App.h"
#include "../WinBase/Helper.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

#define USE_FLIPMODE 1

// Vertex Structure
struct Vertex
{
	Vector3 position;		
	Vector4 color;			

	Vertex(float x, float y, float z) : position(x, y, z) {}
	Vertex(Vector3 position) : position(position) {}
	Vertex(Vector3 position, Vector4 color) : position(position), color(color) {}
};

// Main process
App::App()
{
	
}

App::~App()
{

}

bool App::OnInit()
{
	if (!InitD3D()) return false;
	if (!InitRenderPipeLine()) return false;
	return true;
}

void App::OnUninit()
{
	UninitD3D();
	UninitRenderPipeLine();
}

void App::OnUpdate()
{

}

void App::OnRender()
{
	// RTV clear
	deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);
	deviceContext->ClearRenderTargetView(renderTargetView, backGroundColor);

	// render pipeline bind (stage setting)
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferStride, &vertexBufferOffset);
	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	deviceContext->VSSetShader(vertexShader, NULL, 0);
	deviceContext->PSSetShader(pixelShader, NULL, 0);

	// render
	deviceContext->DrawIndexed(indexCount, 0, 0);

	// present
	swapChain->Present(1, 0);
}

// D3D
bool App::InitD3D()
{
	HRESULT hr = 0;

	// swap chain setup struct
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = hWnd;
	swapDesc.Windowed = true;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = screenWidth;
	swapDesc.BufferDesc.Height = screenHeight;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	// deviec create debug flag
	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// create device, device context, swap chain
	HR_T(D3D11CreateDeviceAndSwapChain(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		creationFlags, 
		NULL, 
		NULL,
		D3D11_SDK_VERSION, 
		&swapDesc, 
		&swapChain, 
		&device, 
		NULL, 
		&deviceContext));

	// create RTV
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));	// backbuffer get
	HR_T(device->CreateRenderTargetView(pBackBufferTexture, NULL, &renderTargetView));	    // RTV create
	SAFE_RELEASE(pBackBufferTexture);													    // RTV에서 backbuffer texture 참조중 (메모리 관리)
	deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);							// render targetview  binding

	// viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);	// viewport binding

	return true;
}

void App::UninitD3D()
{
	SAFE_RELEASE(renderTargetView);
	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(device);
}

bool App::InitRenderPipeLine()
{
	HRESULT hr = 0;

	// IA - vertex buffer create
	Vertex vertices[] =
	{
		Vertex(Vector3(-0.5f,  0.5f, 0.5f), Vector4(0, 0, 0, 1.0f)),
		Vertex(Vector3(0.5f,  0.5f, 0.5f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
		Vertex(Vector3(-0.5f, -0.5f, 0.5f), Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
		Vertex(Vector3(0.5f, -0.5f, 0.5f), Vector4(0, 0, 0, 1.0f))
	};

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);	// buffer size
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;				// bind 용도
	vbDesc.Usage = D3D11_USAGE_DEFAULT;							// buffer 사용 용도	

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;								    // vertex data

	HR_T(device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer));
	vertexBufferStride = sizeof(Vertex);
	vertexBufferOffset = 0;


	// IA - index buffer create
	WORD indices[] =
	{
		0, 1, 2,
		2, 1, 3
	};
	indexCount = ARRAYSIZE(indices);

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);	// buffer size
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;			    // bind 용도		
	ibDesc.Usage = D3D11_USAGE_DEFAULT;						// buffer 사용 용도

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;								// index data

	HR_T(device->CreateBuffer(&ibDesc, &ibData, &indexBuffer));


	// IA - input layout create
	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D10Blob* vertexShaderBuffer = nullptr;		// vs mapping
	HR_T(CompileShaderFromFile(L"VertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer));
	HR_T(device->CreateInputLayout(layout, ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout));


	// VS - vertex shader create
	HR_T(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &vertexShader));
	SAFE_RELEASE(vertexShaderBuffer);


	// PS - pixel shader create
	ID3D10Blob* pixelShaderBuffer = nullptr;
	HR_T(CompileShaderFromFile(L"PixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer));
	HR_T(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &pixelShader));
	SAFE_RELEASE(pixelShaderBuffer);

	return true;
}

void App::UninitRenderPipeLine()
{
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(inputLayout);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
}

