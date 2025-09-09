#pragma once
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using Microsoft::WRL::ComPtr;

// DirectX 3D ���̺귯���� ����ϱ� ���� �ʱ�ȭ ���� Ŭ�����Դϴ�.
class D3DBase
{
public:
	static ComPtr<ID3D11Device>		       device;
	static ComPtr<ID3D11DeviceContext>     deviceContext;
	static ComPtr<IDXGISwapChain>		   swapChain;
	static ComPtr<ID3D11RenderTargetView>  renderTargetView;

	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
	static void UnInit();
};
