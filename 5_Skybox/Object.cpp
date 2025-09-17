#include "Object.h"
#include "../WinBase/Helper.h"
#include "DirectionalLight.h"
#include <Directxtk/DDSTextureLoader.h>

using namespace DirectX;

void Object::InitRenderPipeLine()
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

	// constant buffer create (vs, ps에 전달할 사용할 행렬 data)
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3DBase::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

	// Object Init
	InitTransform();
	rotation = { 0, 45, 0 };
}

void Object::Render(Matrix& view, Matrix&projection, DirectionalLight& light)
{
	// render pipeline stage setting
	D3DBase::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferStride, &vertexBufferOffset);
	D3DBase::deviceContext->IASetInputLayout(inputLayout);
	D3DBase::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	D3DBase::deviceContext->VSSetShader(vertexShader, NULL, 0);
	D3DBase::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	D3DBase::deviceContext->PSSetShader(pixelShader, NULL, 0);
	D3DBase::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
	D3DBase::deviceContext->PSSetShaderResources(0, 1, &textureRV);

	// render
	ConstantBuffer constBuffer;
	constBuffer.world = XMMatrixTranspose(world);
	constBuffer.view = XMMatrixTranspose(view);
	constBuffer.projection = XMMatrixTranspose(projection);
	constBuffer.lightDirection = light.direction;
	constBuffer.lightColor = light.color;
	D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &constBuffer, 0, 0);
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);
}

void Object::UninitRenderPipeLine()
{
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(constantBuffer);
	SAFE_RELEASE(inputLayout);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
}