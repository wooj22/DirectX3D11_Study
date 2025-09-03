#pragma once
#include "../WinBase/WinApp.h"
#include <d3d11.h>
#include <directxtk/simplemath.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

class App : public WinApp
{
public:
	// rendering
	ID3D11Device*			device = nullptr;				// ����̽�	
	ID3D11DeviceContext*	deviceContext = nullptr;		// ����̽� ���ؽ�Ʈ
	IDXGISwapChain1*	    swapChain = nullptr;			// ����ü��
	ID3D11RenderTargetView* renderTargetView = nullptr;		// ���� Ÿ�� ��

public:
	// main process
	App();
	~App() override;

	// override
	virtual bool OnInit() override;
	virtual void OnUninit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;

	// d3d
	bool InitD3D();
	void UninitD3D();
};

