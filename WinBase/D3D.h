#pragma once
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using Microsoft::WRL::ComPtr;

/*
    여러 프로젝트에서 공통적으로 사용될 D3D 객체들을 초기화합니다.
    InputLayout, ConstantBuffer, Shader, 기타 옵션 객체 등은 각 프로젝트에서 정의합니다.
*/

class D3D
{
public:
    // 필수 객체
	static ComPtr<ID3D11Device>		         device;
	static ComPtr<ID3D11DeviceContext>       deviceContext;
	static ComPtr<IDXGISwapChain>		     swapChain;
	static ComPtr<ID3D11RenderTargetView>    renderTargetView;
	static ComPtr<ID3D11DepthStencilView>    depthStencilView;

    // 옵션 객체
	static ComPtr<ID3D11DepthStencilState>   depthStencilState;		// write off
    static ComPtr <ID3D11RasterizerState>    rasterizerState;       // cullmode = front
	static ComPtr<ID3D11SamplerState>	     samplerState;
	static ComPtr<ID3D11BlendState>          blendState;			// alpha

	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
	static void UnInit();
};

