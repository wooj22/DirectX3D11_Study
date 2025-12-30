#pragma once
#include <d3d11.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <wrl/client.h>
#include <vector>

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
struct BloomCB;

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
    static ComPtr<ID3D11Texture2D>           backbufferTex;
	static ComPtr<ID3D11RenderTargetView>    renderTargetView;      // LDR (final)
    static ComPtr<ID3D11Texture2D>           depthStencilTexture;
	static ComPtr<ID3D11DepthStencilView>    depthStencilView;

    // viewport
    static D3D11_VIEWPORT viewport_screen;
    static D3D11_VIEWPORT viewport_shadowMap;

    // HDR
    static ComPtr<ID3D11Texture2D>           sceneHDRTex;
    static ComPtr<ID3D11RenderTargetView>    sceneHDRRTV;          // HDR RTV
    static ComPtr<ID3D11ShaderResourceView>  sceneHDRSRV;          // HDR SRV

    // Shadow
    static ComPtr<ID3D11Texture2D>           shadowMap;
    static ComPtr<ID3D11DepthStencilView>    shadowDSV;
    static ComPtr<ID3D11ShaderResourceView>  shadowSRV;            // ShadowMap Texture
    static ComPtr<ID3D11SamplerState>        shadowSamplerState;   // clmap

    // Deferred - G-buffer (write: RTV, read: SRV)
    static ComPtr<ID3D11Texture2D>           positionTex;
    static ComPtr<ID3D11Texture2D>           albedoTex;
    static ComPtr<ID3D11Texture2D>           normalTex;
    static ComPtr<ID3D11Texture2D>           metalRoughTex;
    static ComPtr<ID3D11Texture2D>           emissiveTex;

    static ComPtr<ID3D11RenderTargetView>    positionRTV;
    static ComPtr<ID3D11RenderTargetView>    albedoRTV;
    static ComPtr<ID3D11RenderTargetView>    normalRTV;
    static ComPtr<ID3D11RenderTargetView>    metalRoughRTV;
    static ComPtr<ID3D11RenderTargetView>    emissiveRTV;

    static ComPtr<ID3D11ShaderResourceView>  positionSRV;
    static ComPtr<ID3D11ShaderResourceView>  albedoSRV;
    static ComPtr<ID3D11ShaderResourceView>  normalSRV;
    static ComPtr<ID3D11ShaderResourceView>  metalRoughSRV;
    static ComPtr<ID3D11ShaderResourceView>  emissiveSRV;
    static ComPtr<ID3D11ShaderResourceView>  depthSRV;

    // Bloom
    static UINT bloomW;
    static UINT bloomH;
    static UINT bloomMipCount;
    static ComPtr<ID3D11Texture2D>           bloomATex;
    static ComPtr<ID3D11Texture2D>           bloomBTex;
    static ComPtr<ID3D11ShaderResourceView>  bloomASRV;
    static ComPtr<ID3D11ShaderResourceView>  bloomBSRV;
    static std::vector<ComPtr<ID3D11RenderTargetView>> bloomARTVs;
    static std::vector<ComPtr<ID3D11RenderTargetView>> bloomBRTVs;

    static ComPtr<ID3D11Texture2D>           accumATex;
    static ComPtr<ID3D11Texture2D>           accumBTex;
    static ComPtr<ID3D11ShaderResourceView>  accumASRV;
    static ComPtr<ID3D11ShaderResourceView>  accumBSRV;
    static std::vector<ComPtr<ID3D11RenderTargetView>> accumARTVs;
    static std::vector<ComPtr<ID3D11RenderTargetView>> accumBRTVs;

    // 옵션
    static ComPtr<ID3D11DepthStencilState>   defualtDSS;               // test + write
	static ComPtr<ID3D11DepthStencilState>   wirteoffDSS;              // write off
    static ComPtr<ID3D11DepthStencilState>   disableDSS;               // test + write off
    static ComPtr<ID3D11RasterizerState>     cullfrontRS;              // cullmode = front
	static ComPtr<ID3D11SamplerState>	     linearSamplerState;       // linear    
	static ComPtr<ID3D11SamplerState>	     linearClamSamplerState;   // linear + clamp    
	static ComPtr<ID3D11BlendState>          alphaBlendState;          // alpha

    // Vertex Shader
    static ComPtr<ID3D11VertexShader> VS_BaseLit_Static;
    static ComPtr<ID3D11VertexShader> VS_BaseLit_Skinned;
    static ComPtr<ID3D11VertexShader> VS_Skybox;
    static ComPtr<ID3D11VertexShader> VS_Skinned_OutLine;
    static ComPtr<ID3D11VertexShader> VS_ShadowDepth_Skinned;
    static ComPtr<ID3D11VertexShader> VS_ShadowDepth_Static;
    static ComPtr<ID3D11VertexShader> VS_FullScreen;

    // Pixel Shader
    static ComPtr<ID3D11PixelShader> PS_BlinnPhong;
    static ComPtr<ID3D11PixelShader> PS_PBR;
    static ComPtr<ID3D11PixelShader> PS_BlinnPhongToon;
    static ComPtr<ID3D11PixelShader> PS_Skybox;
    static ComPtr<ID3D11PixelShader> PS_OutLine;
    static ComPtr<ID3D11PixelShader> PS_PostProcess;
    static ComPtr<ID3D11PixelShader> PS_ShadowDepth;
    static ComPtr<ID3D11PixelShader> PS_BloomPrefilter;
    static ComPtr<ID3D11PixelShader> PS_BloomDownsampleBlur;
    static ComPtr<ID3D11PixelShader> PS_BloomUpsampleCombine;
    static ComPtr<ID3D11PixelShader> PS_Gbuffer;
    static ComPtr<ID3D11PixelShader> PS_DeferredLighting;

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
    static ComPtr<ID3D11Buffer> bloomBuffer;

    static TransformCB     transformCBData;
    static LightingCB      lightingCBData;
    static MaterialCB      materialCBData;
    static OffsetMatrixCB  offsetCBData;
    static PoseMatrixCB    poseCBData;
    static OutLineCB       outlineCBData;
    static DebugCB         debugCBData;
    static PostProcessCB   postprocessCBData;
    static ScreenFxCB      screenFxCBData;
    static BloomCB         bloomCBData;


    //--------------------------------
	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
	static void UnInit();

private:
    static bool CreateHDRResource(int screenWidth, int screenHeight);
    static bool CreateShadowMapResource();
    static bool CreateDeferredResource(int screenWidth, int screenHeight);
    static bool CreateBloomResource(int screenWidth, int screenHeight);
    static bool CreateShader();
    static bool CreateConstantBuffer();

public:
    // Texture, RTV, SRV Create Utils
    static bool CreateRTTex_RTV_SRV(int w, int h, DXGI_FORMAT fomat,
        ID3D11Texture2D** outTex, ID3D11RenderTargetView** outRTV, ID3D11ShaderResourceView** outSRV);

    // Bloom Utils -------------------------
    // mip 해상도 구하기 (baseW/baseH는 bloom base 해상도)
    static void GetMipSize(UINT baseW, UINT baseH, UINT mip, UINT& outW, UINT& outH);

    // mip 기준 texel size 구하기
    static void GetMipTexelSize(UINT baseW, UINT baseH, UINT mip, float& outTx, float& outTy);

    // viewport 설정
    static void SetViewport(UINT width, UINT height);

    // bloom mip용 viewport 설정
    static void SetViewportForMip(UINT baseW, UINT baseH, UINT mip);
};

