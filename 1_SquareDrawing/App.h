#pragma once
#include "../WinBase/WinApp.h"
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <directxtk/simplemath.h>
#include <wrl/client.h>

// index buffer�� Ȱ���ؼ� �簢���� �׸��� ������Ʈ�Դϴ�.

class App : public WinApp
{
public:
	// d3d base
	ID3D11Device*			device = nullptr;
	ID3D11DeviceContext*	deviceContext = nullptr;
	IDXGISwapChain*			swapChain = nullptr;	
	ID3D11RenderTargetView* renderTargetView = nullptr;	

	// rendering pipeline
	ID3D11Buffer*		 vertexBuffer = nullptr;	
	ID3D11Buffer*		 indexBuffer = nullptr;			
	ID3D11InputLayout*   inputLayout = nullptr;		
	ID3D11VertexShader*  vertexShader = nullptr;		
	ID3D11PixelShader*   pixelShader = nullptr;			

	// vertex info
	UINT vertexBufferStride = 0;				// ���ؽ� �ϳ��� ũ��
	UINT vertexBufferOffset = 0;				// ���ؽ� ������ ������
	UINT indexCount = 0;						// �ε��� ����

	// else
	float backGroundColor[4] = { 0, 0, 0, 1.0f };

public:
	// main process
	App();
	~App() override;
	virtual bool OnInit() override;
	virtual void OnUninit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;

	// d3d base
	bool InitD3D();
	void UninitD3D();

	// vertex, shader
	bool InitRenderPipeLine();
	void UninitRenderPipeLine();
};

