#pragma once
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using Microsoft::WRL::ComPtr;

// DirectX 3D 렌더 파이프라인 필수 객체 초기화 클래스입니다.

class D3D
{
public:
	static ComPtr<ID3D11Device>		       device;
	static ComPtr<ID3D11DeviceContext>     deviceContext;
	static ComPtr<IDXGISwapChain>		   swapChain;
	static ComPtr<ID3D11RenderTargetView>  renderTargetView;
	static ComPtr<ID3D11DepthStencilView>  depthStencilView;
	static ComPtr<ID3D11SamplerState>	   samplerState;

	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
	static void UnInit();
};

