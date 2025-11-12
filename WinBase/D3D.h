#pragma once
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"dxgi.lib")

using Microsoft::WRL::ComPtr;

struct TransformCB;
struct LightingCB;
struct MaterialCB;
struct OffsetMatrixCB;
struct PoseMatrixCB;
struct OutLineCB;

/*
*   Direct3D 리소스 초기화용 정적 헬퍼 클래스
    여러 프로젝트에서 공통적으로 사용될 D3D 객체들을 초기화합니다.
*/

class D3D
{
public:
    // 필수
	static ComPtr<ID3D11Device>		         device;
	static ComPtr<ID3D11DeviceContext>       deviceContext;
	static ComPtr<IDXGISwapChain>		     swapChain;
	static ComPtr<ID3D11RenderTargetView>    renderTargetView;
	static ComPtr<ID3D11DepthStencilView>    depthStencilView;

    // viewport
    static D3D11_VIEWPORT viewport_screen;
    static D3D11_VIEWPORT viewport_shadowMap;

    // Shadow
    static ComPtr<ID3D11DepthStencilView>    shadowDSV;
    static ComPtr<ID3D11ShaderResourceView>  shadowSRV;
    static ComPtr<ID3D11SamplerState>        shadowSamplerState;

    // 옵션
	static ComPtr<ID3D11DepthStencilState>   depthStencilState;		// write off
    static ComPtr <ID3D11RasterizerState>    rasterizerState;       // cullmode = front
	static ComPtr<ID3D11SamplerState>	     samplerState;
	static ComPtr<ID3D11BlendState>          blendState;			// alpha

    // Shader
    static ComPtr<ID3D11VertexShader> BaseLit_Static_VS;
    static ComPtr<ID3D11VertexShader> BaseLit_Skinned_VS;
    static ComPtr<ID3D11VertexShader> Skybox_VS;
    static ComPtr<ID3D11VertexShader> Skinned_OutLine_VS;
    static ComPtr<ID3D11VertexShader> ShadowDepth_Skinned_VS;
    static ComPtr<ID3D11VertexShader> ShadowDepth_Static_VS;
    static ComPtr<ID3D11PixelShader> BlinnPhong_PS;
    static ComPtr<ID3D11PixelShader> BlinnPhongToon_PS;
    static ComPtr<ID3D11PixelShader> Skybox_PS;
    static ComPtr<ID3D11PixelShader> OutLine_PS;

    // InputLayout
    static ComPtr<ID3D11InputLayout> inputLayout_Vertex;
    static ComPtr<ID3D11InputLayout> inputLayout_BoneWeightVertex;
    static ComPtr<ID3D11InputLayout> inputLayout_Skybox;

    // ConstantBuffer
    static ComPtr<ID3D11Buffer> transformBuffer;
    static ComPtr<ID3D11Buffer> lightingBuffer;
    static ComPtr<ID3D11Buffer> materialBuffer;
    static ComPtr<ID3D11Buffer> offsetMatrixBuffer;
    static ComPtr<ID3D11Buffer> poseMatrixBuffer;
    static ComPtr<ID3D11Buffer> outlineBuffer;

    static TransformCB     transformCBData;
    static LightingCB      lightingCBData;
    static MaterialCB      materialCBData;
    static OffsetMatrixCB  offsetCBData;
    static PoseMatrixCB    poseCBData;
    static OutLineCB       outlineCBData;


    //--------------------------------
	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
    static bool CreateShader();
    static bool CreateConstantBuffer();
	static void UnInit();
};

