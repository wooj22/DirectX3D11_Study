#include "D3DBase.h"
#include "../WinBase/Helper.h"

// static member init
ComPtr<ID3D11Device>		   D3DBase::device = nullptr;
ComPtr<ID3D11DeviceContext>    D3DBase::deviceContext = nullptr;
ComPtr<IDXGISwapChain>		   D3DBase::swapChain = nullptr;
ComPtr<ID3D11RenderTargetView> D3DBase::renderTargetView = nullptr;

bool D3DBase::Init(HWND& hWnd, int screenWidth, int screenHeight)
{
	// swap chain setup struct
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = hWnd;
	swapDesc.Windowed = true;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = screenWidth;
	swapDesc.BufferDesc.Height = screenHeight;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	// deviec create debug flag
	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// create device, device context, swap chain
	HR_T(D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		creationFlags,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&swapDesc,
		&swapChain,
		&device,
		NULL,
		&deviceContext));

	// create RTV					// TODO :: 여기 RTV이 null임. 일단 라이브러리 사용 x
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));	// backbuffer get
	HR_T(device->CreateRenderTargetView(pBackBufferTexture, NULL, renderTargetView.GetAddressOf()));	    // RTV create
	SAFE_RELEASE(pBackBufferTexture);													    // RTV에서 backbuffer texture 참조중 (메모리 관리)
	ID3D11RenderTargetView* rtv = renderTargetView.Get();
	deviceContext->OMSetRenderTargets(1, &rtv, nullptr);						// render targetview  binding

	// viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);	// viewport binding

	return true;
}

void D3DBase::UnInit()
{
	renderTargetView.Reset();
	deviceContext.Reset();
	swapChain.Reset();
	device.Reset();
}