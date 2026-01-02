#include "App.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Helper.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>
#include "../WinBase/Camera.h"
#include "../WinBase/AssetManager.h"


#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define USE_FLIPMODE 1

static int currentSkybox = 3;
const char* skyboxes[] = { "OutDoor", "InDoor", "BlueSky", "RedSky" };

// Main process
bool App::OnInit()
{
    if (!D3D::Init(hWnd, screenWidth, screenHeight)) return false;
    if (!InitRenderPipeLine()) return false;
    if (!InitGUI()) return false;

    // skybox
    skybox1.InitRenderPipeLine(L"../Resource/Skybox/skybox_cubmap.dds");
    skybox2.InitRenderPipeLine(L"../Resource/Skybox/indoorEnvHDR.dds");
    skybox3.InitRenderPipeLine(L"../Resource/Skybox/blueskyEnvHDR.dds");
    skybox4.InitRenderPipeLine(L"../Resource/Skybox/redskyEnvHDR.dds");

    /// model
    floor = AssetManager::Instance().LoadStaticModelAsset("../Resource/Plane.fbx");
    tree = AssetManager::Instance().LoadStaticModelAsset("../Resource/Tree.fbx");
    zelda = AssetManager::Instance().LoadStaticModelAsset("../Resource/zeldaPosed001.fbx");
    character = AssetManager::Instance().LoadRigidModelAsset("../Resource/char.fbx");
    girl = AssetManager::Instance().LoadSkeletalModelAsset("../Resource/Girl.fbx");
    enemy = AssetManager::Instance().LoadSkeletalModelAsset("../Resource/Enemy.fbx");

    for (int i = 0; i < 10; i++)
    {
        auto model = AssetManager::Instance().LoadStaticModelAsset("../Resource/sphere.fbx");
        spheres.push_back(model);
        spheres[i]->SetPosition({ -900 + i * 200.0f, 50.0f, 1000 });
        spheres[i]->SetScale({ 0.85,0.85,0.85 });
    }

    for (int i = 0; i < 10; i++)
    {
        auto model = AssetManager::Instance().LoadStaticModelAsset("../Resource/Torus.fbx");
        torus.push_back(model);
        torus[i]->SetPosition({ -900 + i * 200.0f, 50.0f, 500 });
        torus[i]->SetScale({ 0.7,0.7,0.7 });
    }

    floor->SetPosition({ 0,-5, 600 });
    floor->SetScale({ 0.5,0.3,0.5 });
    tree->SetPosition({ -150, 0, 130 });
    tree->SetScale({ 80, 80, 80 });
    zelda->SetPosition({ -180,0,0 });
    character->SetPosition({ -20,0,0 });
    girl->SetPosition({ 100,0,70 });
    enemy->SetPosition({ 250,0,20 });

    // light
    light.direction = { 0,-0.5,1 };
    light.color = { 1.0f, 0.9608f, 0.8980f, 1.0f };
    light.directIntensity = 2.0f;
    light.indirectIntensity = 0.2f;

    // view maxtrix
    camera.position = { 0, 80, -300 };
    camera.moveSpeed = 300.f;
    camera.GetViewMatrix(view);

    // projection matrix 
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    // debug settup
    D3D::lightingCBData.useIBL = 1;
    D3D::postprocessCBData.isHDR = 1;
    D3D::postprocessCBData.contrast = 1.0;
    D3D::postprocessCBData.saturation = 1.0;
    D3D::screenFxCBData.enableWaterDistortion = 1;

    // memory debugger
    memory_debugger.Init();

    // debug draw set up
    m_states = std::make_unique<CommonStates>(D3D::device.Get());
    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(D3D::deviceContext.Get());
    m_effect = std::make_unique<BasicEffect>((D3D::device.Get()));
    m_effect->SetVertexColorEnabled(true);
    m_effect->SetView(view);
    m_effect->SetProjection(projection);
    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        HR_T(D3D::device.Get()->CreateInputLayout(
            VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_layout.ReleaseAndGetAddressOf())
        );
    }

    return true;
}

void App::OnUninit()
{
    UninitGUI();
    UninitRenderPipeLine();
    D3D::UnInit();
    CheckDXGIDebug();
}

void App::OnUpdate()
{
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    // model update(local, model matrix)
    character->Update();
    girl->Update();
    enemy->Update();
    camera.GetViewMatrix(view);

    // light update
    Vector3 sceneCenter = camera.position + camera.GetForward() * lookPointDist;
    Vector3 lightPos = sceneCenter - light.direction * shadowLightDist;
    lightView = XMMatrixLookAtLH(lightPos, sceneCenter, Vector3::Up);
    lightProjection = XMMatrixOrthographicLH(shadowWidth, shadowHeight, shadowNear, shadowFar);

    // Memory Cheak
    memory_debugger.CheakMemoryUsage();

    // Trim
    if (Input::GetKeyDown('T'))
    {
        memory_debugger.Trim();
    }
}

void App::OnRender()
{
    // Stage Setting
    DefualtStageSetting();         // binding + CB udpate
        
    // Render
    SceneHDRRender();       // Gaometry + Lighting + Shadow
    BloomProcess();         // Bloom
    PostProcess();          // ToneMapping + PostProcess + ScreenFx

    // Debug Draw
    FrustumDebugDraw(view, projection, view, projection, Colors::FloralWhite);
    FrustumDebugDraw(lightView, lightProjection, view, projection, Colors::GreenYellow);

    // GUI
    RenderGUI();

    // Present
    D3D::swapChain->Present(1, 0);
}

// Stage Setting + CB update
void App::DefualtStageSetting()
{
    // Stage Setting
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());
    D3D::deviceContext->PSSetSamplers(1, 1, D3D::shadowSamplerState.GetAddressOf());
    D3D::deviceContext->PSSetSamplers(2, 1, D3D::linearClamSamplerState.GetAddressOf());
    D3D::deviceContext->OMSetBlendState(D3D::alphaBlendState.Get(), blendFactor, sampleMask);

    D3D::deviceContext->VSSetConstantBuffers(0, 1, D3D::transformBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(0, 1, D3D::transformBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(1, 1, D3D::lightingBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(1, 1, D3D::lightingBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(2, 1, D3D::materialBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(3, 1, D3D::offsetMatrixBuffer.GetAddressOf());
    D3D::deviceContext->VSSetConstantBuffers(4, 1, D3D::poseMatrixBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(6, 1, D3D::debugBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(7, 1, D3D::postprocessBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(8, 1, D3D::screenFxBuffer.GetAddressOf());
    D3D::deviceContext->PSSetConstantBuffers(9, 1, D3D::bloomBuffer.GetAddressOf());

    // CB Update
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::transformCBData.shadowView = XMMatrixTranspose(lightView);
    D3D::transformCBData.shadowProjection = XMMatrixTranspose(lightProjection);
    D3D::transformCBData.cameraPos = camera.position;
    D3D::transformCBData.screenSize = { (float)screenWidth,(float)screenHeight };

    D3D::lightingCBData.lightDirection = light.direction;
    D3D::lightingCBData.lightColor = light.color;
    D3D::lightingCBData.directIntensity = light.directIntensity;
    D3D::lightingCBData.indirectIntensity = light.indirectIntensity;
    D3D::lightingCBData.useIBL = useIBL ? 1 : 0;
    D3D::lightingCBData.isSunLight = true;

    D3D::materialCBData.useBaseColorOverride = useBaseColorOverride ? 1 : 0;
    D3D::materialCBData.useEmissiveOverride = useEmissiveOverride ? 1 : 0;
    D3D::materialCBData.useMetallicOverride = useMetallicOverride ? 1 : 0;
    D3D::materialCBData.useRoughnessOverride = useRoughnessOverride ? 1 : 0;

    D3D::postprocessCBData.useDefaultGamma = usedefalutGamma ? 1 : 0;
    D3D::postprocessCBData.useColorAdjustments = useColorAdjustments ? 1 : 0;
    D3D::postprocessCBData.useLGG = useLGG ? 1 : 0;
    D3D::postprocessCBData.useVignette = useVignette ? 1 : 0;
    D3D::postprocessCBData.useFilmGrain = useFilmGrain ? 1 : 0;
    D3D::postprocessCBData.useBloom = useBloom ? 1 : 0;

    D3D::postprocessCBData.useWhiteBalance = useWhiteBalance ? 1 : 0;
    D3D::postprocessCBData.useHueShift = useHueShift ? 1 : 0;
    D3D::postprocessCBData.useColorTint = useColorTint ? 1 : 0;
    D3D::postprocessCBData.useLift = useLift ? 1 : 0;
    D3D::postprocessCBData.useGamma = useGamma ? 1 : 0;
    D3D::postprocessCBData.useGain = useGain ? 1 : 0;

    D3D::screenFxCBData.time = Time::GetTotalTime();
    D3D::screenFxCBData.enableWaterDistortion = enableRipple == 1 ? 1 : 0;
    D3D::screenFxCBData.enablePlasmaOverlay = enablePlasmaOverlay == 1 ? 1 : 0;
    D3D::screenFxCBData.enableFilmGrain = enableFilmGrain == 1 ? 1 : 0;

    D3D::deviceContext->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::debugBuffer.Get(), 0, nullptr, &D3D::debugCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::postprocessBuffer.Get(), 0, nullptr, &D3D::postprocessCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::screenFxBuffer.Get(), 0, nullptr, &D3D::screenFxCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);
}

// [ Gaometry + Lighting + Shadow Pass ]
void App::SceneHDRRender()
{
    // Clear
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::sceneHDRRTV.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->ClearRenderTargetView(D3D::sceneHDRRTV.Get(), clearColor);

    // death buffer clear
    D3D::deviceContext->ClearDepthStencilView(D3D::depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Skybox Render  --------------------------------
    switch (currentSkybox)
    {
    case 0:
        skybox1.Render(view, projection);
        break;
    case 1:
        skybox2.Render(view, projection);
        break;
    case 2:
        skybox3.Render(view, projection);
        break;
    case 3:
        skybox4.Render(view, projection);
        break;
    }

    // Skyboc에서 view 행렬에 transform을 제거해서 다시 udpate
    D3D::transformCBData.view = XMMatrixTranspose(view);

    // 1. ShadowMap Pass  -------------------------------------
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_shadowMap);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(0, nullptr, D3D::shadowDSV.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->ClearDepthStencilView(D3D::shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Static, Rigid Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_ShadowDepth_Static.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PS_ShadowDepth.Get(), NULL, 0);    // alpha discard
    tree->Render();
    zelda->Render();
    character->Render();
    for (int i = 0; i < 10; i++)
    {
        spheres[i]->Render();
        torus[i]->Render();
    }

    // Skeletal Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_ShadowDepth_Skinned.Get(), NULL, 0);
    girl->Render();
    enemy->Render();


    // 2. Scene HDR Color Pass -------------------------------------
    // Static, Rigid Model
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);	// viewport binding
    D3D::deviceContext->OMSetRenderTargets(1, D3D::sceneHDRRTV.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Vertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_BaseLit_Static.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PS_PBR.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    switch (currentSkybox)
    {
    case 0:
        D3D::deviceContext->PSSetShaderResources(9, 1, &IBL_IrradianceMap1);
        D3D::deviceContext->PSSetShaderResources(10, 1, &IBL_SpecularEnvMap1);
        D3D::deviceContext->PSSetShaderResources(11, 1, &IBL_BRDF_LUT1);
        break;
    case 1:
        D3D::deviceContext->PSSetShaderResources(9, 1, &IBL_IrradianceMap2);
        D3D::deviceContext->PSSetShaderResources(10, 1, &IBL_SpecularEnvMap2);
        D3D::deviceContext->PSSetShaderResources(11, 1, &IBL_BRDF_LUT2);
        break;
    case 2:
        D3D::deviceContext->PSSetShaderResources(9, 1, &IBL_IrradianceMap3);
        D3D::deviceContext->PSSetShaderResources(10, 1, &IBL_SpecularEnvMap3);
        D3D::deviceContext->PSSetShaderResources(11, 1, &IBL_BRDF_LUT3);
        break;
    case 3:
        D3D::deviceContext->PSSetShaderResources(9, 1, &IBL_IrradianceMap4);
        D3D::deviceContext->PSSetShaderResources(10, 1, &IBL_SpecularEnvMap4);
        D3D::deviceContext->PSSetShaderResources(11, 1, &IBL_BRDF_LUT4);
        break;
    }

    for (int i = 0; i < 10; i++)
    {
        spheres[i]->Render();
        torus[i]->Render();
    }
    floor->Render();
    tree->Render();
    zelda->Render();
    character->Render();

    // Skeletal Model
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_BaseLit_Skinned.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    girl->Render();
    enemy->Render();

    // shadowmap SRV hazard 방지
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(6, 1, nullSRV);
}


// [ BloomProcess Pass ]
// Prefilter -> DownSample+Blur -> UpSample+Combine
void App::BloomProcess()
{
    // Full Screen VS setup
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->IASetInputLayout(nullptr);
    D3D::deviceContext->VSSetShader(D3D::VS_FullScreen.Get(), NULL, 0);

    // clear
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
    D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
    D3D::deviceContext->PSSetShaderResources(14, 1, nullSRV);

    ID3D11RenderTargetView* nullRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);


    // 1. Prefilter Pass ------------------------------------
    //  - HDR을 샘플링하여 BloomA의 mip0에 처리할 픽셀만 기록
    //  - sceneHDR read -> mip0 write
    {
        // View Port
        D3D::SetViewportForMip(D3D::bloomW, D3D::bloomH, 0);

        // output : A.mip0
        D3D::deviceContext->OMSetRenderTargets(1, D3D::bloomARTVs[0].GetAddressOf(), nullptr);
        D3D::deviceContext->ClearRenderTargetView(D3D::bloomARTVs[0].Get(), clearColor);

        // input : sceneHDR
        D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
        D3D::deviceContext->PSSetShaderResources(12, 1, D3D::sceneHDRSRV.GetAddressOf());

        // CB
        UINT w0, h0;
        D3D::GetMipSize(D3D::bloomW, D3D::bloomH, 0, w0, h0);
        D3D::bloomCBData.srcMip = 0;
        D3D::bloomCBData.srcTexelSize = DirectX::XMFLOAT2(1.0f / (float)w0, 1.0f / (float)h0);
        D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);

        // Draw Call
        D3D::deviceContext->PSSetShader(D3D::PS_BloomPrefilter.Get(), NULL, 0);
        D3D::deviceContext.Get()->Draw(3, 0);

        // cleanup
        D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
    }
    

    // 2. DownSample Blur Pass ------------------------------
    //  - BloomA mip0을 시작으로 Mip Chain 형성 + 블러 처리한다.
    //  - BloomA와 BloomB를 SRV와 RTV로 ping-pong하며 read & write 교대
    //  - mip(i-1) read -> mip(i) write
    {
        for (int i = 1; i < D3D::bloomMipCount; ++i)
        {
            // RTV UnBind
            D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);

            // View Port
            D3D::SetViewportForMip(D3D::bloomW, D3D::bloomH, i);

            // Ping Pong
            // i 홀수 : A(SRV) -> B(RTV)
            // i 짝수 : B(SRV) -> A(RTV)
            bool AtoB = (i % 2 != 0) ? true : false;
  
            // input : A or B mip(i-1)
            ID3D11ShaderResourceView* bloomSRV = AtoB ? D3D::bloomASRV.Get() : D3D::bloomBSRV.Get();
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
            D3D::deviceContext->PSSetShaderResources(13, 1, &bloomSRV);

            // output : A or B mip(i)
            ID3D11RenderTargetView* bloomRTV = AtoB ? D3D::bloomBRTVs[i].Get() : D3D::bloomARTVs[i].Get();
            D3D::deviceContext->OMSetRenderTargets(1, &bloomRTV, nullptr);
            D3D::deviceContext->ClearRenderTargetView(bloomRTV, clearColor);

            // CB
            UINT sw, sh;
            D3D::GetMipSize(D3D::bloomW, D3D::bloomH, i - 1, sw, sh);
            D3D::bloomCBData.srcTexelSize = DirectX::XMFLOAT2(1.0f / (float)sw, 1.0f / (float)sh);
            D3D::bloomCBData.srcMip = (float)(i - 1);
            D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);

            // Draw Call
            D3D::deviceContext->PSSetShader(D3D::PS_BloomDownsampleBlur.Get(), NULL, 0);
            D3D::deviceContext.Get()->Draw(3, 0);

            // cleanup
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
        }
    }

    // 3. UpSample Combine Pass -----------------------------
    //  - mipN에서 mip0으로 올라오며 업샘플 + 가산합성하여 최종 블룸 이미지를 도출한다.
    //  - DownSample 결과의 시작 누적(accum)은 last mip이 들어있는 텍스처에서 시작한다.
    //  - BloomA, BloomB : read only
    //  - AccumA, AccumB : small(accum) read, out write
    //  - mip(i, i+1) read -> mip(i)  write
    {
        // LastMip은 bloomMipCount-1이 홀수면 B, 짝수면 A에 있음 (downpass에서 핑퐁했기 때문에)
        int lastMipLevel = (int)D3D::bloomMipCount - 1;
        bool lastMipOnBloomB = (lastMipLevel % 2) != 0;

        // 첫 루프에서는 accum에 아직 누적 텍스처가 없으므로, bloom에서 big과 small 가져온다.
        bool accumOnB = false;  // bloomA를 small로 시작 (2개를 한번에 합할거라 순서 상관 x)

        for (int i = lastMipLevel - 1; i >= 0; --i)
        {
            // cleanup
            D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
            D3D::deviceContext->PSSetShaderResources(14, 1, nullSRV);

            // View Port
            D3D::SetViewportForMip(D3D::bloomW, D3D::bloomH, i);

            // 1) Big SRV : 더 큰 해상도의 mip이 있는 SRV
            ID3D11ShaderResourceView* bigSRV = nullptr;
            if (i == 0) bigSRV = D3D::bloomASRV.Get();      // prefilter 단계에서 mip0은 BloomA에 저장함
            else bigSRV = ((i % 2) != 0) ? D3D::bloomBSRV.Get() : D3D::bloomASRV.Get();

            // 2) Small SRV : 가산 누적된 mip이 있는 SRV (업샘플 소스)
            // 첫 루프에서는 accum에 아직 가산한 누적 텍스처가 없으므로, small = Bloom A or B의 last mip
            // 다음부터는 small = AccumA or AccumB의 mip(i+1)
            ID3D11ShaderResourceView* smallSRV = nullptr;
            if (i == lastMipLevel - 1)
                smallSRV = lastMipOnBloomB ? D3D::bloomBSRV.Get() : D3D::bloomASRV.Get();
            else
                smallSRV = accumOnB ? D3D::accumBSRV.Get() : D3D::accumASRV.Get();

            // 3) out RTV : 현재 패스에서 기록할 texture. (smallSRV와 겹치면 안됨!)
            const bool outOnB = !accumOnB;
            ID3D11RenderTargetView* outRTV = outOnB ? D3D::accumBRTVs[i].Get() : D3D::accumARTVs[i].Get();

            // SRV, RTV Bind
            D3D::deviceContext->PSSetShaderResources(13, 1, &bigSRV);
            D3D::deviceContext->PSSetShaderResources(14, 1, &smallSRV);
            D3D::deviceContext->OMSetRenderTargets(1, &outRTV, nullptr);

            // CB
            UINT wi, hi;
            D3D::GetMipSize(D3D::bloomW, D3D::bloomH, (UINT)i, wi, hi);
            D3D::bloomCBData.srcTexelSize = DirectX::XMFLOAT2(1.0f / (float)wi, 1.0f / (float)hi);
            D3D::bloomCBData.srcMip = (float)i;
            D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);

            // Draw Call
            D3D::deviceContext->PSSetShader(D3D::PS_BloomUpsampleCombine.Get(), NULL, 0);
            D3D::deviceContext->Draw(3, 0);

            // cleanup
            D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);
            D3D::deviceContext->PSSetShaderResources(14, 1, nullSRV);

            // 누적 위치 갱신
            accumOnB = outOnB;
        }

        // Final Bloom Texture
        // accumOnB가 가리키는 Accum 텍스처의 mip0 -> PostProcess에 활용
        finalBloomSRV = accumOnB ? D3D::accumBSRV.Get() : D3D::accumASRV.Get();
    }
}

// [ PostProcess Pass ]
// ToneMapping(LDR) + PostProcess
// Tone Mapping 패스는 화면을 덮는 FullScreen 사각형을 그리면서,
// HDR SRV를 샘플링해 색을 계산하고, 그 결과를 BackBuffer에 기록하는 단계
void App::PostProcess()
{
    // clear
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
    D3D::deviceContext->PSSetShaderResources(13, 1, nullSRV);

    ID3D11RenderTargetView* nullRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
    D3D::deviceContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, nullRTVs, nullptr);

    // view port
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);

    // RTV
    D3D::deviceContext->OMSetRenderTargets(1, D3D::renderTargetView.GetAddressOf(), nullptr);
    D3D::deviceContext->ClearRenderTargetView(D3D::renderTargetView.Get(), clearColor);

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext->IASetInputLayout(nullptr);

    // Shaders
    D3D::deviceContext->VSSetShader(D3D::VS_FullScreen.Get(), NULL, 0);
    D3D::deviceContext->PSSetShader(D3D::PS_PostProcess.Get(), NULL, 0);
    D3D::deviceContext->PSSetShaderResources(12, 1, D3D::sceneHDRSRV.GetAddressOf());
    D3D::deviceContext->PSSetShaderResources(13, 1, &finalBloomSRV);

    // Render
    D3D::deviceContext.Get()->Draw(3, 0);

    // SRV hazard 방지
    D3D::deviceContext->PSSetShaderResources(12, 1, nullSRV);
}

bool App::InitRenderPipeLine()
{
    // IBL Textures Load
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/skybox_cubemapDiffuseHDR.dds", nullptr, &IBL_IrradianceMap1);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/skybox_cubemapSpecularHDR.dds", nullptr, &IBL_SpecularEnvMap1);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/skybox_cubemapBrdf.dds", nullptr, &IBL_BRDF_LUT1);

    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/indoorDiffuseHDR.dds", nullptr, &IBL_IrradianceMap2);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/indoorSpecularHDR.dds", nullptr, &IBL_SpecularEnvMap2);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/indoorBrdf.dds", nullptr, &IBL_BRDF_LUT2);

    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/blueskyDiffuseHDR.dds", nullptr, &IBL_IrradianceMap3);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/blueskySpecularHDR.dds", nullptr, &IBL_SpecularEnvMap3);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/blueskyBrdf.dds", nullptr, &IBL_BRDF_LUT3);

    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/redskyDiffuseHDR.dds", nullptr, &IBL_IrradianceMap4);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/redskySpecularHDR.dds", nullptr, &IBL_SpecularEnvMap4);
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/IBL/redskyBrdf.dds", nullptr, &IBL_BRDF_LUT4);

    return true;
}

void App::UninitRenderPipeLine()
{
    skybox1.UninitRenderPipeLine();
}

bool App::InitGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(D3D::device.Get(), D3D::deviceContext.Get());

    return true;
}

void App::UninitGUI()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void App::RenderGUI()
{
    // GUI
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Inspector
    ImGui::Begin("Inspertor", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Text("[Light]");
    ImGui::SliderFloat3("Direction", &light.direction.x, -1.0f, 1.0f, "%.2f");
    ImGui::ColorEdit3("Color", &light.color.x);
    ImGui::SliderFloat("Direct Intensity", &light.directIntensity, 0.0f, 10.0f);
    ImGui::SliderFloat("Indirect Intensity", &light.indirectIntensity, 0.0f, 10.0f);

    ImGui::Text("");
    ImGui::Text("[Material]");
    ImGui::SliderFloat("Metallic Factor", &D3D::materialCBData.metallicFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Roughness Factor", &D3D::materialCBData.roughnessFactor, 0.0f, 1.0f);
    ImGui::SliderFloat("Emissve Factor", &D3D::materialCBData.emissiveFactor, 0.0f, 3.0f);

    ImGui::Checkbox("BaseColor Override", &useBaseColorOverride);
    ImGui::ColorEdit3("BaseColor", &D3D::materialCBData.baseColorOverride.x);
    ImGui::Checkbox("Emissve Override", &useEmissiveOverride);
    ImGui::ColorEdit3("Emissve", &D3D::materialCBData.emissiveOverride.x);
    ImGui::Checkbox("Metallic Override", &useMetallicOverride);
    ImGui::SliderFloat("Metallic", &D3D::materialCBData.metallicOverride, 0.0f, 1.0f);
    ImGui::Checkbox("Roughness Override", &useRoughnessOverride);
    ImGui::SliderFloat("Roughness", &D3D::materialCBData.roughnessOverride, 0.0f, 1.0f);

    ImGui::Text("");
    ImGui::Text("[Transform]");
    ImGui::InputFloat3("position", &character->position.x);
    ImGui::SliderAngle("Pitch", &character->rotation.x, 0.0f, 360.0f);
    ImGui::SliderAngle("Yaw", &character->rotation.y, 0.0f, 360.0f);
    ImGui::SliderAngle("Roll", &character->rotation.z, 0.0f, 360.0f);
    ImGui::InputFloat3("scale", &character->scale.x);
    ImGui::End();

    // Camera
    ImGui::Begin("[Camera]");
    ImGui::SliderFloat("Near", &camera.Near, 0.01f, 10000.0f);
    ImGui::SliderFloat("Far", &camera.Far, 0.01f, 10000.0f);

    ImGui::SliderFloat("FOV", &fovDeg, 20.0f, 90.0f);

    camera.FovY = XMConvertToRadians(fovDeg);
    camera.FovY = std::clamp(camera.FovY, 0.3f, 1.7f);
    ImGui::InputFloat("Move Speec", &camera.moveSpeed, 0.0f, 0.0f, "%.3f");
    ImGui::End();

    // Shadow
    ImGui::Begin("[Shadow]");
    ImGui::Text("[Shadow Frustum]");
    ImGui::SliderFloat("Near", &shadowNear, 0.01f, 10000.0f);
    ImGui::SliderFloat("Far", &shadowFar, 0.01f, 10000.0f);
    ImGui::InputFloat("Width", &shadowWidth);
    ImGui::InputFloat("Height", &shadowHeight);

    ImGui::Text("");
    ImGui::Text("[Shadow Light Pos]");
    ImGui::SliderFloat("lookPointDist", &lookPointDist, 1.f, 5000.0f);
    ImGui::SliderFloat("shadowLightDist", &shadowLightDist, 1.f, 5000.0f);
    ImGui::End();

    // IBL
    ImGui::Begin("[IBL]");
    ImGui::Checkbox("use IBL", &useIBL);

    ImGui::Text("");
    ImGui::Text("[Skybox]");
    ImGui::Combo("Skybox Mode", &currentSkybox, skyboxes, IM_ARRAYSIZE(skyboxes));
    ImGui::End();

    // PostProcess
    ImGui::Begin("[PostProcess]");
    ImGui::Text("Gamma (Linear->SRGB)");
    ImGui::Checkbox("use defalutGamma", &usedefalutGamma);
    ImGui::BeginDisabled(!usedefalutGamma);
    ImGui::SliderFloat("Gamma", &D3D::postprocessCBData.defaultGamma, 0.f, 5.0f);
    ImGui::EndDisabled();

    ImGui::Text("");
    ImGui::Text("[Color Adjustments]");
    ImGui::Checkbox("Enable Color Adjustments", &useColorAdjustments);
    ImGui::BeginDisabled(!useColorAdjustments);
    ImGui::SliderFloat("Exposure", &D3D::postprocessCBData.exposure, -5.f, 5.0f);
    ImGui::SliderFloat("Contrast", &D3D::postprocessCBData.contrast, 0.5f, 2.0f);
    ImGui::SliderFloat("Saturation", &D3D::postprocessCBData.saturation, 0.5f, 2.0f);

    ImGui::Checkbox("use HueShift", &useHueShift);
    ImGui::SliderAngle("HueShift", &D3D::postprocessCBData.hueShift, -180.0f, 180.0f);
    
    ImGui::Checkbox("use ColorTint", &useColorTint);
    ImGui::ColorEdit3("Color Tint", &D3D::postprocessCBData.colorTint.x);
    ImGui::SliderFloat("Strength", &D3D::postprocessCBData.colorTint_strength, 0, 1.0f);
    ImGui::EndDisabled();

    ImGui::Text("");
    ImGui::Text("[Bloom]");
    ImGui::Checkbox("Enable Bloom", &useBloom);
    ImGui::BeginDisabled(!useBloom);
    ImGui::SliderFloat("Threshold", &D3D::bloomCBData.bloom_threshold, 0.0f, 5.0f, "%.2f");
    ImGui::SliderFloat("Clamp", &D3D::bloomCBData.bloom_clamp, 0.0f, 20.0f, "%.2f");
    ImGui::SliderFloat("Scatter", &D3D::bloomCBData.bloom_scatter, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Bloom Intensity", &D3D::bloomCBData.bloom_intensity, 0.0f, 10.0f, "%.2f");
    ImGui::ColorEdit3("Bloom Tint", (float*)&D3D::bloomCBData.bloom_tint, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
    ImGui::EndDisabled();

    ImGui::Text("");
    ImGui::Text("[White Balance]");
    ImGui::Checkbox("Enable White Balance", &useWhiteBalance);
    ImGui::BeginDisabled(!useWhiteBalance);
    ImGui::SliderFloat("Temperature", &D3D::postprocessCBData.temperature, -1.f, 1.0f);
    ImGui::SliderFloat("Tint", &D3D::postprocessCBData.tint, -1.f, 1.0f);
    ImGui::EndDisabled();

    ImGui::Text("");
    ImGui::Text("[Lift / Gamma / Gain]");
    ImGui::Checkbox("Enable LGG", &useLGG);
    ImGui::BeginDisabled(!useLGG);
    ImGui::Checkbox("use Lift", &useLift);
    ImGui::SliderFloat3("Lift RGB", &D3D::postprocessCBData.lift.x, -1.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Lift Strength", &D3D::postprocessCBData.lift_strength, 0.0f, 1.0f, "%.3f");
    ImGui::Checkbox("use Gamma", &useGamma);
    ImGui::SliderFloat3("Gamma RGB", &D3D::postprocessCBData.gamma.x, -1.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Gamma Strength", &D3D::postprocessCBData.gamma_strength, 0.0f, 1.0f, "%.3f");
    ImGui::Checkbox("use Gain", &useGain);
    ImGui::SliderFloat3("Gain RGB", &D3D::postprocessCBData.gain.x, -1.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Gain Strength", &D3D::postprocessCBData.gain_strength, 0.0f, 1.0f, "%.3f");
    ImGui::EndDisabled();

    ImGui::Text("");
    ImGui::Text("[Vignette]");
    ImGui::Checkbox("Enable Vignette", &useVignette);
    ImGui::BeginDisabled(!useVignette);
    ImGui::SliderFloat("Vignette Intensity", &D3D::postprocessCBData.vignette_intensity, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Smoothness", &D3D::postprocessCBData.vignette_smoothness, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat2("Center", &D3D::postprocessCBData.vignetteCenter.x, 0.0f, 1.0f, "%.3f");
    ImGui::ColorEdit3("Vignette Color", &D3D::postprocessCBData.vignetteColor.x);
    ImGui::EndDisabled();

    ImGui::Text("");
    ImGui::Text("[Film Grain]");
    ImGui::Checkbox("Enable FilmGrain", &useFilmGrain);
    ImGui::BeginDisabled(!useFilmGrain);
    ImGui::SliderFloat("FilmGrain Intensity", &D3D::postprocessCBData.grain_intensity, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("Response", &D3D::postprocessCBData.grain_response, 0.0f, 1.0f, "%.3f");
    ImGui::SliderFloat("GrainScale", &D3D::postprocessCBData.grain_scale, 0.0f, 5.0f, "%.2f");
    ImGui::EndDisabled();
    ImGui::End();

    // Screen Space Effect
    ImGui::Begin("[Screen Effect]");
    ImGui::Text("Enable");
    ImGui::Checkbox("useRipple", &enableRipple);
    ImGui::Checkbox("usePlasmaOverlay", &enablePlasmaOverlay);
    ImGui::Checkbox("useFilmGrain", &enableFilmGrain);

    ImGui::Text("Pattern / Noise");
    ImGui::SliderFloat("Cell Scale", &D3D::screenFxCBData.cellScale, 0.1f, 10.0f);
    ImGui::SliderFloat("Random Intensity", &D3D::screenFxCBData.randomIntensity, 1000.0f, 60000.0f);
    ImGui::SliderFloat("Warp Strength", &D3D::screenFxCBData.warpStrength, 0.0f, 3.0f);
    ImGui::SliderFloat("distortion Strength", &D3D::screenFxCBData.distortionStrength, 0.002f, 0.008f);

    ImGui::Text("Plasma Overlay");
    ImGui::SliderFloat("Plasma Intensity", &D3D::screenFxCBData.plasmaIntensity, 0.0f, 1.5f);

    ImGui::Text("Film Grain");
    ImGui::SliderFloat("Grain Intensity", &D3D::screenFxCBData.grainIntensity, 0.0f, 0.15f);
    ImGui::End();

    // Memory
    ImGui::Begin("[Memory Debugger]");
    ImGui::Text("[T] Trim");
    ImGui::Text("%ls", memory_debugger.GetMemoryUsageWstring().c_str());
    ImGui::End();


    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// Frustum Debug Draw
void App::FrustumDebugDraw(const Matrix& frustumView, const Matrix& frustumProj,
    const Matrix& renderView, const Matrix& renderProj, FXMVECTOR color)
{
    // Frustum Create
    BoundingFrustum frustum{};
    BoundingFrustum::CreateFromMatrix(frustum, frustumProj); // view space 기준
    Matrix invFrustumView = frustumView.Invert();
    frustum.Transform(frustum, invFrustumView);        // view -> world

    // Effect Update (render 기준은 항상 main camera)
    m_effect->SetWorld(Matrix::Identity);
    m_effect->SetView(renderView);
    m_effect->SetProjection(renderProj);
    m_effect->Apply(D3D::deviceContext.Get());

    // Stage Setting
    D3D::deviceContext.Get()->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    D3D::deviceContext.Get()->OMSetDepthStencilState(m_states->DepthNone(), 0);
    D3D::deviceContext.Get()->RSSetState(m_states->CullNone());
    D3D::deviceContext.Get()->IASetInputLayout(m_layout.Get());

    // Draw
    m_batch->Begin();
    Draw(m_batch.get(), frustum, color);
    m_batch->End();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK App::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    return __super::WindowProc(hWnd, message, wParam, lParam);
}