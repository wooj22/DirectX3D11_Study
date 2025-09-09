#include "App.h"
#include "../WinBase/Helper.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")
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
bool App::OnInit()
{
	if (!D3DBase::Init(hWnd,screenWidth,screenHeight)) return false;
	if (!InitRenderPipeLine()) return false;
	return true;
}

void App::OnUninit()
{
	D3DBase::UnInit();
	UninitRenderPipeLine();
}

void App::OnUpdate()
{

}

void App::OnRender()
{
	// RTV clear
	D3DBase::deviceContext->OMSetRenderTargets(1, D3DBase::renderTargetView.GetAddressOf(), NULL);
	D3DBase::deviceContext->ClearRenderTargetView(D3DBase::renderTargetView.Get(), backGroundColor);

	// render pipeline bind (stage setting)
	D3DBase::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3DBase::deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexBufferStride, &vertexBufferOffset);
	D3DBase::deviceContext->IASetInputLayout(inputLayout);
	D3DBase::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	D3DBase::deviceContext->VSSetShader(vertexShader, NULL, 0);
	D3DBase::deviceContext->PSSetShader(pixelShader, NULL, 0);

	// render
	D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);

	// present
	D3DBase::swapChain->Present(1, 0);
}

bool App::InitRenderPipeLine()
{
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

	HR_T(D3DBase::device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer));
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

	HR_T(D3DBase::device->CreateBuffer(&ibDesc, &ibData, &indexBuffer));


	// IA - input layout create
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

