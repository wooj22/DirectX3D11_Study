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
	ID3D11Device*			device = nullptr;				// 디바이스	
	ID3D11DeviceContext*	deviceContext = nullptr;		// 디바이스 컨텍스트
	IDXGISwapChain1*	    swapChain = nullptr;			// 스왑체인
	ID3D11RenderTargetView* renderTargetView = nullptr;		// 렌더 타겟 뷰

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

