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
struct DebugCB;
struct PostProcessCB;
struct ScreenFxCB;

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
	static ComPtr<ID3D11RenderTargetView>    renderTargetView;      // LDR
	static ComPtr<ID3D11DepthStencilView>    depthStencilView;

    // viewport
    static D3D11_VIEWPORT viewport_screen;
    static D3D11_VIEWPORT viewport_shadowMap;

    // HDR
    static ComPtr<ID3D11Texture2D>           hdrTexture;
    static ComPtr<ID3D11RenderTargetView>    hdrRTV;         // HDR RTV
    static ComPtr<ID3D11ShaderResourceView>  hdrSRV;         // HDR SRV

    // Shadow
    static ComPtr<ID3D11Texture2D>           shadowMap;
    static ComPtr<ID3D11DepthStencilView>    shadowDSV;
    static ComPtr<ID3D11ShaderResourceView>  shadowSRV;             // ShadowMap Texture
    static ComPtr<ID3D11SamplerState>        shadowSamplerState;    // clmap

    // 옵션
	static ComPtr<ID3D11DepthStencilState>   wirteoffDSS;              // write off
    static ComPtr <ID3D11RasterizerState>    cullfrontRS;              // cullmode = front
	static ComPtr<ID3D11SamplerState>	     linearSamplerState;       // linear    
	static ComPtr<ID3D11SamplerState>	     linearClamSamplerState;   // linear + clamp    
	static ComPtr<ID3D11BlendState>          alphaBlendState;          // alpha

    // Shader
    static ComPtr<ID3D11VertexShader> BaseLit_Static_VS;
    static ComPtr<ID3D11VertexShader> BaseLit_Skinned_VS;
    static ComPtr<ID3D11VertexShader> Skybox_VS;
    static ComPtr<ID3D11VertexShader> Skinned_OutLine_VS;
    static ComPtr<ID3D11VertexShader> ShadowDepth_Skinned_VS;
    static ComPtr<ID3D11VertexShader> ShadowDepth_Static_VS;
    static ComPtr<ID3D11VertexShader> FullScreen_VS;

    static ComPtr<ID3D11PixelShader> BlinnPhong_PS;
    static ComPtr<ID3D11PixelShader> PBR_PS;
    static ComPtr<ID3D11PixelShader> BlinnPhongToon_PS;
    static ComPtr<ID3D11PixelShader> Skybox_PS;
    static ComPtr<ID3D11PixelShader> OutLine_PS;
    static ComPtr<ID3D11PixelShader> PostProcess_PS;
    static ComPtr<ID3D11PixelShader> ShadowDepth_PS;

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
    static ComPtr<ID3D11Buffer> debugBuffer;
    static ComPtr<ID3D11Buffer> postprocessBuffer;
    static ComPtr<ID3D11Buffer> screenFxBuffer;

    static TransformCB     transformCBData;
    static LightingCB      lightingCBData;
    static MaterialCB      materialCBData;
    static OffsetMatrixCB  offsetCBData;
    static PoseMatrixCB    poseCBData;
    static OutLineCB       outlineCBData;
    static DebugCB         debugCBData;
    static PostProcessCB   postprocessCBData;
    static ScreenFxCB        screenFxCBData;


    //--------------------------------
	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
    static bool CreateShader();
    static bool CreateConstantBuffer();
	static void UnInit();
};

