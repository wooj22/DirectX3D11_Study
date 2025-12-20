#include "D3D.h"
#include "Helper.h"
#include "Structures.hpp"
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "dxguid.lib") 

// static member init
ComPtr<ID3D11Device>		      D3D::device = nullptr;
ComPtr<ID3D11DeviceContext>       D3D::deviceContext = nullptr;
ComPtr<IDXGISwapChain>		      D3D::swapChain = nullptr;
ComPtr<ID3D11RenderTargetView>    D3D::renderTargetView = nullptr;
ComPtr<ID3D11DepthStencilView>    D3D::depthStencilView = nullptr;

D3D11_VIEWPORT D3D::viewport_screen = {};
D3D11_VIEWPORT D3D::viewport_shadowMap = {};

ComPtr<ID3D11Texture2D>           D3D::hdrTexture = nullptr;
ComPtr<ID3D11RenderTargetView>    D3D::hdrRTV = nullptr;   
ComPtr<ID3D11ShaderResourceView>  D3D::hdrSRV = nullptr;   

ComPtr<ID3D11Texture2D>           D3D::shadowMap = nullptr;
ComPtr<ID3D11DepthStencilView>    D3D::shadowDSV = nullptr;
ComPtr<ID3D11ShaderResourceView>  D3D::shadowSRV = nullptr;
ComPtr<ID3D11SamplerState>        D3D::shadowSamplerState = nullptr;

ComPtr<ID3D11DepthStencilState>   D3D::wirteoffDSS = nullptr;
ComPtr<ID3D11RasterizerState>     D3D::cullfrontRS = nullptr;
ComPtr<ID3D11SamplerState>        D3D::linearSamplerState = nullptr;
ComPtr<ID3D11SamplerState>        D3D::linearClamSamplerState = nullptr;
ComPtr<ID3D11BlendState>          D3D::alphaBlendState = nullptr;
                                  
ComPtr<ID3D11VertexShader>        D3D::BaseLit_Static_VS = nullptr;
ComPtr<ID3D11VertexShader>        D3D::BaseLit_Skinned_VS = nullptr;
ComPtr<ID3D11VertexShader>        D3D::Skybox_VS = nullptr;
ComPtr<ID3D11VertexShader>        D3D::Skinned_OutLine_VS = nullptr;
ComPtr<ID3D11VertexShader>        D3D::ShadowDepth_Skinned_VS = nullptr;
ComPtr<ID3D11VertexShader>        D3D::ShadowDepth_Static_VS = nullptr;
ComPtr<ID3D11VertexShader>        D3D::FullScreen_VS = nullptr;

ComPtr<ID3D11PixelShader>         D3D::BlinnPhong_PS = nullptr;
ComPtr<ID3D11PixelShader>         D3D::PBR_PS = nullptr;
ComPtr<ID3D11PixelShader>         D3D::BlinnPhongToon_PS = nullptr;
ComPtr<ID3D11PixelShader>         D3D::Skybox_PS = nullptr;
ComPtr<ID3D11PixelShader>         D3D::OutLine_PS = nullptr;
ComPtr<ID3D11PixelShader>         D3D::PostProcess_PS = nullptr;
ComPtr<ID3D11PixelShader>         D3D::ShadowDepth_PS = nullptr;
                     
ComPtr<ID3D11InputLayout>         D3D::inputLayout_Vertex = nullptr;
ComPtr<ID3D11InputLayout>         D3D::inputLayout_BoneWeightVertex = nullptr;
ComPtr<ID3D11InputLayout>         D3D::inputLayout_Skybox = nullptr;

ComPtr<ID3D11Buffer>              D3D::transformBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::lightingBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::materialBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::offsetMatrixBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::poseMatrixBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::outlineBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::debugBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::postprocessBuffer = nullptr;
ComPtr<ID3D11Buffer>              D3D::screenFxBuffer = nullptr;

TransformCB        D3D::transformCBData;
LightingCB         D3D::lightingCBData;
MaterialCB         D3D::materialCBData;
OffsetMatrixCB     D3D::offsetCBData;
PoseMatrixCB       D3D::poseCBData;
OutLineCB          D3D::outlineCBData;
DebugCB            D3D::debugCBData;
PostProcessCB      D3D::postprocessCBData;
ScreenFxCB           D3D::screenFxCBData;


bool D3D::Init(HWND& hWnd, int screenWidth, int screenHeight)
{
	// swap chain setup struct
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = hWnd;
	swapDesc.Windowed = true;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // UNORM : PS에서 감마 인코딩 필요, UNORM_SRGB : PS 자동 감마 적용
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
	viewport_screen = {};
	viewport_screen.TopLeftX = 0;
	viewport_screen.TopLeftY = 0;
	viewport_screen.Width = (float)screenWidth;
	viewport_screen.Height = (float)screenHeight;
	viewport_screen.MinDepth = 0.0f;
	viewport_screen.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport_screen);	// viewport binding

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


    // create HDR RTV, SRV
    {
        // texture
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = screenWidth;
        texDesc.Height = screenHeight;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;       // HDR 포멧
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, hdrTexture.GetAddressOf());
        if (FAILED(hr)) { OutputDebugStringA("FAILED Create HDR Texture"); }

        // RTV
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;       // HDR 포멧
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        hr = device->CreateRenderTargetView(hdrTexture.Get(), &rtvDesc, hdrRTV.GetAddressOf());
        if (FAILED(hr)) { OutputDebugStringA("FAILED Create HDR RTV"); }

        // SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;       // HDR 포멧
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        hr = device->CreateShaderResourceView(hdrTexture.Get(), &srvDesc, hdrSRV.GetAddressOf());
        if(FAILED(hr)) { OutputDebugStringA("FAILED Create HDR SRV"); }
    }


    // create shadowDSV, shadowSRV
    {
        // viewport
        viewport_shadowMap = {};
        viewport_shadowMap.TopLeftX = 0;
        viewport_shadowMap.TopLeftY = 0;
        viewport_shadowMap.Width = (float)8192;
        viewport_shadowMap.Height = (float)8192;
        viewport_shadowMap.MinDepth = 0.0f;
        viewport_shadowMap.MaxDepth = 1.0f;

        // texture2D
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = 8192;
        texDesc.Height = 8192;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R32_TYPELESS;        // DSV와 SRV가 TYPELESS 텍스처 공유
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL |    // 깊이값 기록 용도
                            D3D11_BIND_SHADER_RESOURCE;   // 셰이더에서 텍스처 슬롯에 설정할 용도
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;

        
        HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, shadowMap.GetAddressOf());
        if (FAILED(hr)) { OutputDebugStringA("FAILED Create ShadowMapTexture"); }

        // DSV
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        hr = device->CreateDepthStencilView(shadowMap.Get(), &dsvDesc, shadowDSV.GetAddressOf());
        if (FAILED(hr)) { OutputDebugStringA("FAILED Create Shadow Depth Stencil View"); }

        // SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        hr = device->CreateShaderResourceView(shadowMap.Get(), &srvDesc, shadowSRV.GetAddressOf());
        if (FAILED(hr)) { OutputDebugStringA("FAILED Create Shadow Shader Resource View"); }

        // Sampler State
        D3D11_SAMPLER_DESC sampDesc = {};
        sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        HR_T(device->CreateSamplerState(&sampDesc, shadowSamplerState.GetAddressOf()));
    }

	// create depth stencil state (alpha, skybox)
    {
        D3D11_DEPTH_STENCIL_DESC dsDesc = {};
        dsDesc.DepthEnable = TRUE;                              // 깊이 테스트 o  
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;    // 버퍼 기록 x
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        dsDesc.StencilEnable = FALSE;

        HR_T(device->CreateDepthStencilState(&dsDesc, wirteoffDSS.GetAddressOf()));
    }

    // create rasterizer state (skybox 큐브의 안쪽이 그려지도록 cull mode front)
    {
        D3D11_RASTERIZER_DESC rsDesc = {};
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.CullMode = D3D11_CULL_FRONT;
        rsDesc.DepthClipEnable = TRUE;
        HR_T(device->CreateRasterizerState(&rsDesc, cullfrontRS.GetAddressOf()));
    }

	// create smapler state
    {
        D3D11_SAMPLER_DESC sample_Desc = {};
        sample_Desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;			// 상하좌우 텍셀 보간
        sample_Desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;				// 0~1 범위를 벗어난 uv는 소수 부분만 사용
        sample_Desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_Desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_Desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sample_Desc.MinLOD = 0;
        sample_Desc.MaxLOD = D3D11_FLOAT32_MAX;
        HR_T(device->CreateSamplerState(&sample_Desc, linearSamplerState.GetAddressOf()));
    }

    // create smapler state 
    {
        D3D11_SAMPLER_DESC sampDesc = {};
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampDesc.MipLODBias = 0.0f;
        sampDesc.MaxAnisotropy = 1;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDesc.MinLOD = 0.0f;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        HR_T(device->CreateSamplerState(&sampDesc, linearClamSamplerState.GetAddressOf()));
    }

	// create blend state
    {
        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        D3D::device->CreateBlendState(&blendDesc, alphaBlendState.GetAddressOf());
    }

    if (!CreateShader()) return false;
    if (!CreateConstantBuffer()) return false;

	return true;
}

bool D3D::CreateShader()
{
    //---------------------------
    // 1. Skybox
    {
        // InputLayout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        ID3D10Blob* vertexShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/Skybox_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer));
        HR_T(device->CreateInputLayout(layout, ARRAYSIZE(layout),
            vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout_Skybox));

        // VS
        device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &Skybox_VS);
        vertexShaderBuffer->Release();

        // PS
        ID3D10Blob* pixelShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/Skybox_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
        HR_T(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &Skybox_PS));
    }

    //---------------------------
    // 2. Static Mesh
    {
        // Input Layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 48,  D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        ID3D10Blob* vertexShaderBuffer = nullptr;		// vs mapping
        HR_T(CompileShaderFromFile(L"../WinBase/BaseLit_Static_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer));
        HR_T(device->CreateInputLayout(layout, ARRAYSIZE(layout),
            vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout_Vertex));

        // VS
        HR_T(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
            vertexShaderBuffer->GetBufferSize(), NULL, &BaseLit_Static_VS));
        SAFE_RELEASE(vertexShaderBuffer);

        // ShadowDepth_VS
        ID3D10Blob* vertexShaderBuffer2 = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/ShadowDepth_Static_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer2));
        HR_T(device->CreateVertexShader(vertexShaderBuffer2->GetBufferPointer(),
            vertexShaderBuffer2->GetBufferSize(), NULL, &ShadowDepth_Static_VS));
        SAFE_RELEASE(vertexShaderBuffer2);
    }

    //---------------------------
    // 3. Skeletal Mesh
    {
        // Input Layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {   // SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate	
            { "POSITION"    , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL"      , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT"     , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BITANGENT"   , 0, DXGI_FORMAT_R32G32B32_FLOAT  , 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD"    , 0, DXGI_FORMAT_R32G32_FLOAT     , 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BONE_INDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        ID3D10Blob* vertexShaderBuffer = nullptr;		// vs mapping
        HR_T(CompileShaderFromFile(L"../WinBase/BaseLit_Skinned_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer));
        HR_T(device->CreateInputLayout(layout, ARRAYSIZE(layout),
            vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &inputLayout_BoneWeightVertex));

        // VS
        HR_T(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
            vertexShaderBuffer->GetBufferSize(), NULL, &BaseLit_Skinned_VS));
        SAFE_RELEASE(vertexShaderBuffer);

        // Skinned OutLine VS
        ID3D10Blob* vertexShaderBuffer2 = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/Skinned_OutLine_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer2));
        HR_T(device->CreateVertexShader(vertexShaderBuffer2->GetBufferPointer(),
            vertexShaderBuffer2->GetBufferSize(), NULL, &Skinned_OutLine_VS));
        SAFE_RELEASE(vertexShaderBuffer2);

        // ShadowDepth_VS
        ID3D10Blob* vertexShaderBuffer3 = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/ShadowDepth_Skinned_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer3));
        HR_T(device->CreateVertexShader(vertexShaderBuffer3->GetBufferPointer(),
            vertexShaderBuffer3->GetBufferSize(), NULL, &ShadowDepth_Skinned_VS));
        SAFE_RELEASE(vertexShaderBuffer3);
    }

    //---------------------------
    // Full Screen VS
    {
        ID3D10Blob* vertexShaderBuffer = nullptr;		// vs mapping
        HR_T(CompileShaderFromFile(L"../WinBase/Fullscreen_VS.hlsl", "main", "vs_5_0", &vertexShaderBuffer));
        HR_T(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
            vertexShaderBuffer->GetBufferSize(), NULL, &FullScreen_VS));
        SAFE_RELEASE(vertexShaderBuffer);
    }
  

    //---------------------------
    // BlinnPhong PS
    {
        ID3D10Blob* pixelShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/BlinnPhong_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
        HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
            pixelShaderBuffer->GetBufferSize(), NULL, &BlinnPhong_PS));
        SAFE_RELEASE(pixelShaderBuffer);
    }


    //---------------------------
    // PBR (+IBL) PS
    {
        ID3D10Blob* pixelShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/PBR_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
        HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
            pixelShaderBuffer->GetBufferSize(), NULL, &PBR_PS));
        SAFE_RELEASE(pixelShaderBuffer);
    }


    //---------------------------
    // BlinnPhong Toon PS
    {
        ID3D10Blob* pixelShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/BlinnPhongToon_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
        HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
            pixelShaderBuffer->GetBufferSize(), NULL, &BlinnPhongToon_PS));
        SAFE_RELEASE(pixelShaderBuffer);
    }


    //---------------------------
    // OutLine PS
    {
        ID3D10Blob* pixelShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/OutLine_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
        HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
            pixelShaderBuffer->GetBufferSize(), NULL, &OutLine_PS));
        SAFE_RELEASE(pixelShaderBuffer);
    }

    //---------------------------
    // PostProcess PS
    {
        ID3D10Blob* pixelShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/PostProcess_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
        HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
            pixelShaderBuffer->GetBufferSize(), NULL, &PostProcess_PS));
        SAFE_RELEASE(pixelShaderBuffer);
    }

    //---------------------------
    // ShadowDepth PS
    {
        ID3D10Blob* pixelShaderBuffer = nullptr;
        HR_T(CompileShaderFromFile(L"../WinBase/ShadowDepth_PS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));
        HR_T(D3D::device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
            pixelShaderBuffer->GetBufferSize(), NULL, &ShadowDepth_PS));
        SAFE_RELEASE(pixelShaderBuffer);
    }

    return true;
}

bool D3D::CreateConstantBuffer()
{
    // 1. TransformCB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(TransformCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &transformBuffer));
    }

    // 2. LightingCB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(LightingCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &lightingBuffer));
    }

    // 3. MaterialCB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(MaterialCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &materialBuffer));
    }

    // 4. OffsetMatrixCB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(OffsetMatrixCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &offsetMatrixBuffer));
    }

    // 5. PoseMatrixCB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(PoseMatrixCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &poseMatrixBuffer));
    }

    // 6. OutLineCB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(OutLineCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &outlineBuffer));
    }

    // 7. Dubug CB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(DebugCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &debugBuffer));
    }

    // 8. PostProcess CB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(PostProcessCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &postprocessBuffer));
    }

    // 9. Post Ex CB
    {
        D3D11_BUFFER_DESC constBuffer_Desc = {};
        constBuffer_Desc.Usage = D3D11_USAGE_DEFAULT;
        constBuffer_Desc.ByteWidth = sizeof(ScreenFxCB);
        constBuffer_Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constBuffer_Desc.CPUAccessFlags = 0;
        HR_T(device->CreateBuffer(&constBuffer_Desc, nullptr, &screenFxBuffer));
    }

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