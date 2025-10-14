#include "Cube.h"
#include "../WinBase/Helper.h"
#include "../WinBase/Camera.h"
#include "DirectionalLight.hpp"
#include "Material.hpp"
#include <Directxtk/DDSTextureLoader.h>

#include <string>
#include <sstream>
#include <Windows.h>

using namespace DirectX;

void Cube::InitRenderPipeLine()
{
	// IA - vertex buffer create
	// Vertex가 normal벡터 정보를 가져야하므로 정육면체의 각 면마다의 vertex 정보를 넣어주어야 한다.
	Cube_Vertex vertices[] =
	{
		// position, tangent, bitangent, normal, uv
		// Top (+Y)
		Cube_Vertex({-1,  1, -1}, {1,0,0}, {0,0,-1}, {0,1,0}, {0,0}),
		Cube_Vertex({ 1,  1, -1}, {1,0,0}, {0,0,-1}, {0,1,0}, {1,0}),
		Cube_Vertex({ 1,  1,  1}, {1,0,0}, {0,0,-1}, {0,1,0}, {1,1}),
		Cube_Vertex({-1,  1,  1}, {1,0,0}, {0,0,-1}, {0,1,0}, {0,1}),

		// Bottom (-Y)
		Cube_Vertex({-1,-1,-1}, {1,0,0}, {0,0,1}, {0,-1,0}, {0,0}),
		Cube_Vertex({ 1,-1,-1}, {1,0,0}, {0,0,1}, {0,-1,0}, {1,0}),
		Cube_Vertex({ 1,-1, 1}, {1,0,0}, {0,0,1}, {0,-1,0}, {1,1}),
		Cube_Vertex({-1,-1, 1}, {1,0,0}, {0,0,1}, {0,-1,0}, {0,1}),

		// Left (-X)
		Cube_Vertex({-1,-1, 1}, {0,0,-1}, {0,1,0}, {-1,0,0}, {0,0}),
		Cube_Vertex({-1,-1,-1}, {0,0,-1}, {0,1,0}, {-1,0,0}, {1,0}),
		Cube_Vertex({-1, 1,-1}, {0,0,-1}, {0,1,0}, {-1,0,0}, {1,1}),
		Cube_Vertex({-1, 1, 1}, {0,0,-1}, {0,1,0}, {-1,0,0}, {0,1}),

		// Right (+X)
		Cube_Vertex({ 1,-1, 1}, {0,0,1}, {0,1,0}, {1,0,0}, {0,0}),
		Cube_Vertex({ 1,-1,-1}, {0,0,1}, {0,1,0}, {1,0,0}, {1,0}),
		Cube_Vertex({ 1, 1,-1}, {0,0,1}, {0,1,0}, {1,0,0}, {1,1}),
		Cube_Vertex({ 1, 1, 1}, {0,0,1}, {0,1,0}, {1,0,0}, {0,1}),

		// Front (-Z)
		Cube_Vertex({-1,-1,-1}, {1,0,0}, {0,1,0}, {0,0,-1}, {0,0}),
		Cube_Vertex({ 1,-1,-1}, {1,0,0}, {0,1,0}, {0,0,-1}, {1,0}),
		Cube_Vertex({ 1, 1,-1}, {1,0,0}, {0,1,0}, {0,0,-1}, {1,1}),
		Cube_Vertex({-1, 1,-1}, {1,0,0}, {0,1,0}, {0,0,-1}, {0,1}),

		// Back (+Z)
		Cube_Vertex({-1,-1, 1}, {-1,0,0}, {0,1,0}, {0,0,1}, {0,0}),
		Cube_Vertex({ 1,-1, 1}, {-1,0,0}, {0,1,0}, {0,0,1}, {1,0}),
		Cube_Vertex({ 1, 1, 1}, {-1,0,0}, {0,1,0}, {0,0,1}, {1,1}),
		Cube_Vertex({-1, 1, 1}, {-1,0,0}, {0,1,0}, {0,0,1}, {0,1}),
	};

	D3D11_BUFFER_DESC vertexBuffer_Desc = {};
	vertexBuffer_Desc.ByteWidth = sizeof(Cube_Vertex) * ARRAYSIZE(vertices);
	vertexBuffer_Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBuffer_Data = {};
	vertexBuffer_Data.pSysMem = vertices;
	vertexBufferStride = sizeof(Cube_Vertex);
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
		19,17,16, 18,17,19,     // front
		22,20,21, 23,20,22		// back
	};

	D3D11_BUFFER_DESC indexBuffer_Desc = {};
	indexBuffer_Desc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	indexBuffer_Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBuffer_Data = {};
	indexBuffer_Data.pSysMem = indices;
	indexCount = ARRAYSIZE(indices);

	HR_T(D3DBase::device->CreateBuffer(&indexBuffer_Desc, &indexBuffer_Data, &indexBuffer));


	// IA - input layout create
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 48,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

	// Texture load
	HR_T(CreateTextureFromFile(D3DBase::device.Get(), L"../Resource/Bricks059_1K-JPG_Color.jpg", &diffuseTRV));
	HR_T(CreateTextureFromFile(D3DBase::device.Get(), L"../Resource/Bricks059_1K-JPG_NormalDX.jpg", &normalTRV));
	HR_T(CreateTextureFromFile(D3DBase::device.Get(), L"../Resource/Bricks059_Specular.png", &specualrTRV));

	// Constant Buffer create
	D3D11_BUFFER_DESC constBuffer_Desc = {};
	constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
	constBuffer_Desc.ByteWidth = sizeof(ConstantBuffer);
	constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBuffer_Desc.CPUAccessFlags = 0;
	HR_T(D3DBase::device->CreateBuffer(&constBuffer_Desc, nullptr, &constantBuffer));

	// Object Init
	InitTransform();
	scale = { 100,100, };
}

void Cube::Update()
{
	// world update
	Matrix t1 = XMMatrixTranslationFromVector(position);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	Matrix r1 = XMMatrixRotationQuaternion(q);
	Matrix s1 = XMMatrixScalingFromVector(scale);
	world = s1 * r1 * t1;
}

void Cube::Render(Matrix& view, Matrix& projection, Camera& camera, DirectionalLight& light, Material& material)
{
	// render pipeline stage setting
	D3DBase::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferStride, &vertexBufferOffset);
	D3DBase::deviceContext->IASetInputLayout(inputLayout);
	D3DBase::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	D3DBase::deviceContext->VSSetShader(vertexShader, NULL, 0);
	D3DBase::deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	D3DBase::deviceContext->PSSetShader(pixelShader, NULL, 0);
	D3DBase::deviceContext->PSSetConstantBuffers(0, 1, &constantBuffer);
	D3DBase::deviceContext->PSSetShaderResources(0, 1, &diffuseTRV);
	D3DBase::deviceContext->PSSetShaderResources(1, 1, &normalTRV);
	D3DBase::deviceContext->PSSetShaderResources(2, 1, &specualrTRV);

	// render
	ConstantBuffer constBuffer;
	constBuffer.world = XMMatrixTranspose(world);
	constBuffer.view = XMMatrixTranspose(view);
	constBuffer.projection = XMMatrixTranspose(projection);
	constBuffer.lightDirection = light.direction;
	constBuffer.lightColor = light.color;
	constBuffer.indirectLight = light.indirectLight;
	constBuffer.directLight = light.directLight;
	constBuffer.ambientReflection = material.ambientReflection;
	constBuffer.diffuseReflection = material.diffuseReflection;
	constBuffer.specularReflection = material.specularReflection;
	constBuffer.shininess = material.shininess;
	constBuffer.cameraPos = camera.position;

	/*
		std::ostringstream oss;
		oss << "Camera Position: "
		<< camera.position.x << ", "
		<< camera.position.y << ", "
		<< camera.position.z << "\n";

		OutputDebugStringA(oss.str().c_str());
	*/

	D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &constBuffer, 0, 0);
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);
}

void Cube::UninitRenderPipeLine()
{
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(constantBuffer);
	SAFE_RELEASE(inputLayout);
	SAFE_RELEASE(vertexShader);
	SAFE_RELEASE(pixelShader);
}