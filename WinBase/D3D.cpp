#include "D3D.h"
#include "../WinBase/Helper.h"

// static member init
ComPtr<ID3D11Device>		    D3D::device = nullptr;
ComPtr<ID3D11DeviceContext>     D3D::deviceContext = nullptr;
ComPtr<IDXGISwapChain>		    D3D::swapChain = nullptr;
ComPtr<ID3D11RenderTargetView>  D3D::renderTargetView = nullptr;
ComPtr<ID3D11DepthStencilView>  D3D::depthStencilView = nullptr;

bool D3D::Init(HWND& hWnd, int screenWidth, int screenHeight)
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

	// create RTV					
	ID3D11Texture2D* pBackBufferTexture = nullptr;
	HR_T(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture));					// backbuffer get
	HR_T(device->CreateRenderTargetView(pBackBufferTexture, NULL, renderTargetView.GetAddressOf()));	    // RTV create
	SAFE_RELEASE(pBackBufferTexture);															            // RTV에서 backbuffer texture 참조중 (메모리 관리)

	ID3D11RenderTargetView* rtv = renderTargetView.Get();
	deviceContext->OMSetRenderTargets(1, &rtv, nullptr);	// render targetview  binding

	// viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)screenWidth;
	viewport.Height = (float)screenHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);	// viewport binding

	// create depth stencil view 
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = screenWidth;
	descDepth.Height = screenHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> pTextureDepthStencil;
	HR_T(device->CreateTexture2D(&descDepth, nullptr, pTextureDepthStencil.GetAddressOf()));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	HR_T(device->CreateDepthStencilView(pTextureDepthStencil.Get(), &descDSV, depthStencilView.GetAddressOf()));

	return true;
}

void D3D::UnInit()
{
	// 스마트 포인터이므로 알아서 해제됨
	renderTargetView.Reset();
	deviceContext.Reset();
	swapChain.Reset();
	device.Reset();
}