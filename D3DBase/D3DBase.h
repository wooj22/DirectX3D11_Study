#pragma once
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using Microsoft::WRL::ComPtr;

// TODO :: 삭제 (WinBase 라이브러리로 그래픽 클래스들 옮김)
// 레거시 프로젝트때문에 냅둠
// DirectX 3D 렌더 파이프라인 필수 객체 초기화 클래스입니다.
class D3DBase
{
public:
	static ComPtr<ID3D11Device>		       device;
	static ComPtr<ID3D11DeviceContext>     deviceContext;
	static ComPtr<IDXGISwapChain>		   swapChain;
	static ComPtr<ID3D11RenderTargetView>  renderTargetView;
	static ComPtr<ID3D11DepthStencilView>  depthStencilView;

	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
	static void UnInit();
};
