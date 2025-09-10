#include "App.h"
#include "../WinBase/Helper.h"
#include <DirectXMath.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
using namespace DirectX;
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

// Constant Buffer 상수 버퍼
// VS에 전달할 행렬 데이터로 맵핑할 cbuffer와 정렬 규칙 확인 필수!
struct alignas(16) ConstantBuffer
{
	Matrix world;			// flaot 4x4, 64Byte
	Matrix view;		 	// flaot 4x4, 64Byte  
	Matrix projection;     	// flaot 4x4, 64Byte
};

// Main process
bool App::OnInit()
{
	if (!D3DBase::Init(hWnd,screenWidth,screenHeight)) return false;
	if (!InitRenderPipeLine()) return false;
	return true;
}

void App::OnUninit()
{
	UninitRenderPipeLine();
	D3DBase::UnInit();
	CheckDXGIDebug();
}

void App::OnUpdate()
{
	time = Time::GetTotalTime();

	// world update
	// cube 1
	cube1_matrix = XMMatrixRotationY(-time);

	// cube 2
	Matrix translate = XMMatrixTranslation(3.0f, 0.0f, 0.0f);   // cube1로 부터 떨어진 거리
	Matrix spin = XMMatrixRotationY(time * 3);					// 자전
	Matrix scale = XMMatrixScaling(0.5f, 0.5f, 0.5f);

	// cube1 기준으로 변환 적용
	cube2_matrix = scale * spin * translate * cube1_matrix;
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
	ConstantBuffer cube1_constBuffer;
	cube1_constBuffer.world = XMMatrixTranspose(cube1_matrix);
	cube1_constBuffer.view = XMMatrixTranspose(viewMatrix);
	cube1_constBuffer.projection = XMMatrixTranspose(projectionMatrix);
	D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cube1_constBuffer, 0, 0);
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);

	// cube 2
	ConstantBuffer cube2_constBuffer;
	cube2_constBuffer.world = XMMatrixTranspose(cube2_matrix);
	cube2_constBuffer.view = XMMatrixTranspose(viewMatrix);
	cube2_constBuffer.projection = XMMatrixTranspose(projectionMatrix);
	D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cube2_constBuffer, 0, 0);
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);

	// present
	D3DBase::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
	// IA - vertex buffer create
	Vertex vertices[] =
	{
		// index buffer를 사용하므로 cube정점 8개 data만 넣으면 된다.
		Vertex(Vector3(0,0,0), Vector4(1, 0, 0, 1.0f)),   
		Vertex(Vector3(1,0,0), Vector4(0, 1, 0, 1.0f)),  
		Vertex(Vector3(0,1,0), Vector4(0, 0, 1, 1.0f)),  
		Vertex(Vector3(1,1,0), Vector4(1, 1, 0, 1.0f)),  
		Vertex(Vector3(0,0,1), Vector4(1, 0, 1, 1.0f)),   
		Vertex(Vector3(1,0,1), Vector4(0, 1, 1, 1.0f)),   
		Vertex(Vector3(0,1,1), Vector4(1, 0.5f, 0, 1.0f)),
		Vertex(Vector3(1,1,1), Vector4(1, 1, 1, 1.0f)),   
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
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
	// update에서 계속 udpate되기 때문에 초기화만 해둔다.
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3DBase::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

	

	cube1_matrix = XMMatrixIdentity();
	cube2_matrix = XMMatrixIdentity();

	cube1_matrix = XMMatrixTranslation(0.5f, 0.5f, 0.5f);
	cube2_matrix = XMMatrixTranslation(0.5f, 0.5f, 0.5f);

	XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	viewMatrix = XMMatrixLookAtLH(eye, at, up);

	projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, screenWidth / (FLOAT)screenHeight, 0.01f, 100.0f);

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

