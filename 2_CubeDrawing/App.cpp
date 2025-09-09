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

// Constant Buffer ��� ����
// VS�� ������ ��� �����ͷ� ������ cbuffer�� ���� ��Ģ Ȯ�� �ʼ�!
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

	// cube 1 rotation
	cube1_matrix - XMMatrixRotationY(-time);

	// cube 2 rotation (������ �������� ȸ�� + �����Ÿ� �̵��Ͽ� ȸ��)
	XMMATRIX spin = XMMatrixRotationZ(-time);
	XMMATRIX orbit = XMMatrixRotationY(-time * 2.0f);
	XMMATRIX translate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX scale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	cube2_matrix = scale * spin * translate * orbit;
}

void App::OnRender()
{
	// RTV clear
	D3DBase::deviceContext->OMSetRenderTargets(1, D3DBase::renderTargetView.GetAddressOf(), NULL);
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
	// D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);
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
		// index buffer�� ����ϹǷ� cube���� 8�� data�� ������ �ȴ�.
		Vertex(Vector3(0,0,0), Vector4(1, 0, 0, 1.0f)),
		Vertex(Vector3(1,0,0), Vector4(0, 0, 0, 1.0f)),
		Vertex(Vector3(0,1,0), Vector4(0, 0, 0, 1.0f)),
		Vertex(Vector3(1,1,0), Vector4(0, 1, 0, 1.0f)),
		Vertex(Vector3(0,0,1), Vector4(0, 1, 0, 1.0f)),
		Vertex(Vector3(1,0,1), Vector4(0, 0, 0, 1.0f)),
		Vertex(Vector3(0,1,1), Vector4(0, 0, 0, 1.0f)),
		Vertex(Vector3(1,1,1), Vector4(0, 0, 1, 1.0f)),
	};

	D3D11_BUFFER_DESC vertexBuffer_Desc = {};
	vertexBuffer_Desc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);		// buffer size
	vertexBuffer_Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;					// bind �뵵
	vertexBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;							// buffer ��� �뵵	

	D3D11_SUBRESOURCE_DATA vertexBuffer_Data = {};						    // vertex data
	vertexBuffer_Data.pSysMem = vertices;				 
	vertexBufferStride = sizeof(Vertex);
	vertexBufferOffset = 0;

	HR_T(D3DBase::device->CreateBuffer(&vertexBuffer_Desc, &vertexBuffer_Data, &vertexBuffer));
	

	// IA - index buffer create
	WORD indices[] =
	{
		// �ո� (Z = 0)
	0, 1, 2,   1, 3, 2,
	// �޸� (Z = 1)
	4, 6, 5,   5, 6, 7,
	// ���ʸ� (X = 0)
	0, 2, 4,   4, 2, 6,
	// �����ʸ� (X = 1)
	1, 5, 3,   5, 7, 3,
	// ���� (Y = 1)
	2, 3, 6,   3, 7, 6,
	// �Ʒ��� (Y = 0)
	0, 4, 1,   1, 4, 5
	};
	
	D3D11_BUFFER_DESC indexBuffer_Desc = {};
	indexBuffer_Desc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);		// buffer size
	indexBuffer_Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;			    // bind �뵵		
	indexBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;						// buffer ��� �뵵

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


	// constant buffer create (vs�� ������ ����� ��� data)
	// update���� ��� udpate�Ǳ� ������ �ʱ�ȭ�� �صд�.
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3DBase::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

	cube1_matrix = XMMatrixIdentity();
	cube2_matrix = XMMatrixIdentity();

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

