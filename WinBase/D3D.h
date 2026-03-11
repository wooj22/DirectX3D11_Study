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
struct FrameCB;
struct EffectCB;
struct DecalCB;


/*
*   Direct3D 리소스 초기화용 정적 헬퍼 클래스
    여러 프로젝트에서 공통적으로 사용될 D3D 객체들을 초기화합니다.
*/

class D3D
{
public:
    // DX11 Base
	inline static ComPtr<ID3D11Device>		        device;
	inline static ComPtr<ID3D11DeviceContext>       deviceContext;
	inline static ComPtr<IDXGISwapChain>		    swapChain;
    inline static ComPtr<ID3D11Texture2D>           backbufferTex;
	inline static ComPtr<ID3D11RenderTargetView>    backbufferRTV;          // SDR (final)
    inline static ComPtr<ID3D11Texture2D>           depthStencilTexture;
	inline static ComPtr<ID3D11DepthStencilView>    depthStencilView;
    inline static ComPtr<ID3D11DepthStencilView>    depthStencilReadOnlyView;  // 다중라이팅 안하면 필수 아님

    // viewport
    inline static D3D11_VIEWPORT viewport_screen;
    inline static D3D11_VIEWPORT viewport_shadowMap;

    // DSS
    inline static ComPtr<ID3D11DepthStencilState>   defualtDSS;                 // depth test on + write on
    inline static ComPtr<ID3D11DepthStencilState>   depthTestOnlyDSS;           // depth test only
    inline static ComPtr<ID3D11DepthStencilState>   depthTestStencilWriteDSS;   // depth test only / stencil write on (stencil test ALWAYS)
    inline static ComPtr<ID3D11DepthStencilState>   stencilTestOnlyDSS;         // stencil test only
    inline static ComPtr<ID3D11DepthStencilState>   disableDSS;                 // all disable

    // RS
    inline static ComPtr<ID3D11RasterizerState>     cullfrontRS;              // cullmode = front : 앞면 버리고 뒷면만 남김
    inline static ComPtr<ID3D11RasterizerState>     cullNoneRS;               // 컬링 안하고 모든 영역을 그림

    // Sampler State
    inline static ComPtr<ID3D11SamplerState>	    linearSamplerState;       // linear    
    inline static ComPtr<ID3D11SamplerState>	    linearClamSamplerState;   // linear + clamp    
    inline static ComPtr<ID3D11SamplerState>        shadowSamplerState;       // clmap

    // Blend State
    inline static ComPtr<ID3D11BlendState>          alphaBlendState;          // alpha
    inline static ComPtr<ID3D11BlendState>          additiveBlendState;       // additive (multiple light)

    // HDR
    inline static ComPtr<ID3D11Texture2D>           sceneHDRTex;
    inline static ComPtr<ID3D11RenderTargetView>    sceneHDRRTV;          // HDR RTV
    inline static ComPtr<ID3D11ShaderResourceView>  sceneHDRSRV;          // HDR SRV

    // Shadow Resource
    inline static ComPtr<ID3D11Texture2D>           shadowMap;
    inline static ComPtr<ID3D11DepthStencilView>    shadowDSV;
    inline static ComPtr<ID3D11ShaderResourceView>  shadowSRV;            // ShadowMap Texture

    // G-buffer (write: RTV, read: SRV)
    inline static ComPtr<ID3D11Texture2D>           positionTex;    // 사용 x
    inline static ComPtr<ID3D11Texture2D>           albedoTex;
    inline static ComPtr<ID3D11Texture2D>           normalTex;
    inline static ComPtr<ID3D11Texture2D>           metalRoughTex;
    inline static ComPtr<ID3D11Texture2D>           emissiveTex;

    inline static ComPtr<ID3D11RenderTargetView>    positionRTV;    // 사용 x
    inline static ComPtr<ID3D11RenderTargetView>    albedoRTV;
    inline static ComPtr<ID3D11RenderTargetView>    normalRTV;
    inline static ComPtr<ID3D11RenderTargetView>    metalRoughRTV;
    inline static ComPtr<ID3D11RenderTargetView>    emissiveRTV;

    inline static ComPtr<ID3D11ShaderResourceView>  positionSRV;    // 사용 x
    inline static ComPtr<ID3D11ShaderResourceView>  albedoSRV;
    inline static ComPtr<ID3D11ShaderResourceView>  normalSRV;
    inline static ComPtr<ID3D11ShaderResourceView>  metalRoughSRV;
    inline static ComPtr<ID3D11ShaderResourceView>  emissiveSRV;
    inline static ComPtr<ID3D11ShaderResourceView>  depthSRV;

    // Bloom
    inline static ComPtr<ID3D11ShaderResourceView>  finalBloomSRV;     // 최종 Bloom SRV
    static UINT bloomW;
    static UINT bloomH;
    static UINT bloomMipCount;
    inline static ComPtr<ID3D11Texture2D>           bloomATex;
    inline static ComPtr<ID3D11Texture2D>           bloomBTex;
    inline static ComPtr<ID3D11ShaderResourceView>  bloomASRV;
    inline static ComPtr<ID3D11ShaderResourceView>  bloomBSRV;
    inline static std::vector<ComPtr<ID3D11RenderTargetView>> bloomARTVs;
    inline static std::vector<ComPtr<ID3D11RenderTargetView>> bloomBRTVs;

    inline static ComPtr<ID3D11Texture2D>           accumATex;
    inline static ComPtr<ID3D11Texture2D>           accumBTex;
    inline static ComPtr<ID3D11ShaderResourceView>  accumASRV;
    inline static ComPtr<ID3D11ShaderResourceView>  accumBSRV;
    inline static std::vector<ComPtr<ID3D11RenderTargetView>> accumARTVs;
    inline static std::vector<ComPtr<ID3D11RenderTargetView>> accumBRTVs;


    // InputLayout
    inline static ComPtr<ID3D11InputLayout> inputLayout_RigidVertex;
    inline static ComPtr<ID3D11InputLayout> inputLayout_BoneWeightVertex;
    inline static ComPtr<ID3D11InputLayout> inputLayout_Position;
    inline static ComPtr<ID3D11InputLayout> inputLayout_Particle;


    // Vertex Shader
    inline static ComPtr<ID3D11VertexShader> VS_ShadowDepth_Rigid;
    inline static ComPtr<ID3D11VertexShader> VS_ShadowDepth_Skeletal;
    inline static ComPtr<ID3D11VertexShader> VS_BaseLit_Rigid;
    inline static ComPtr<ID3D11VertexShader> VS_BaseLit_Skeletal;
    inline static ComPtr<ID3D11VertexShader> VS_Skybox;
    inline static ComPtr<ID3D11VertexShader> VS_FullScreen;
    inline static ComPtr<ID3D11VertexShader> VS_LightVolume;
    inline static ComPtr<ID3D11VertexShader> VS_Effect;
    inline static ComPtr<ID3D11VertexShader> VS_Skinned_OutLine;
    

    // Pixel Shader
    inline static ComPtr<ID3D11PixelShader> PS_BlinnPhong;
    inline static ComPtr<ID3D11PixelShader> PS_BlinnPhongToon;

    inline static ComPtr<ID3D11PixelShader> PS_ShadowDepth;
    inline static ComPtr<ID3D11PixelShader> PS_Gbuffer;
    inline static ComPtr<ID3D11PixelShader> PS_DeferredLighting;
    inline static ComPtr<ID3D11PixelShader> PS_ForwardLighting;
    inline static ComPtr<ID3D11PixelShader> PS_PBR;
    inline static ComPtr<ID3D11PixelShader> PS_Skybox;
    inline static ComPtr<ID3D11PixelShader> PS_Effect;
    inline static ComPtr<ID3D11PixelShader> PS_BloomPrefilter;
    inline static ComPtr<ID3D11PixelShader> PS_BloomDownsampleBlur;
    inline static ComPtr<ID3D11PixelShader> PS_BloomUpsampleCombine;
    inline static ComPtr<ID3D11PixelShader> PS_PostProcess;
    inline static ComPtr<ID3D11PixelShader> PS_OutLine;


    // ConstantBuffer
    inline static ComPtr<ID3D11Buffer> transformBuffer;
    inline static ComPtr<ID3D11Buffer> lightingBuffer;
    inline static ComPtr<ID3D11Buffer> materialBuffer;
    inline static ComPtr<ID3D11Buffer> offsetMatrixBuffer;
    inline static ComPtr<ID3D11Buffer> poseMatrixBuffer;
    inline static ComPtr<ID3D11Buffer> outlineBuffer;
    inline static ComPtr<ID3D11Buffer> debugBuffer;
    inline static ComPtr<ID3D11Buffer> postprocessBuffer;
    inline static ComPtr<ID3D11Buffer> screenFxBuffer;
    inline static ComPtr<ID3D11Buffer> bloomBuffer;
    inline static ComPtr<ID3D11Buffer> frameBuffer;
    inline static ComPtr<ID3D11Buffer> effectBuffer;
    inline static ComPtr<ID3D11Buffer> decalBuffer;

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
    static FrameCB         frameCBData;
    static EffectCB        effectCBData;
    static DecalCB         decalCBData;


    //--------------------------------
	static bool Init(HWND& hWnd, int screenWidth, int screenHeight);
	static void UnInit();

private:
    static bool CreateDX11BaseResource(HWND& hWnd, int screenWidth, int screenHeight);

    static bool CreateDSS();
    static bool CreateRS();
    static bool CreateSampler();
    static bool CreateBS();

    static bool CreateHDRResource(int screenWidth, int screenHeight);
    static bool CreateShadowMapResource();
    static bool CreateGbufferResource(int screenWidth, int screenHeight);
    static bool CreateBloomResource(int screenWidth, int screenHeight);
    static bool CreateInputLayoutShader();
    static bool CreateCB();

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

