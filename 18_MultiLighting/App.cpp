#include "App.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Helper.h"
#include "../WinBase/Camera.h"
#include "../WinBase/AssetManager.h"
#include <d3dcompiler.h>
#include <Directxtk/DDSTextureLoader.h>
#include <iostream>

#pragma comment (lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib, "dxguid.lib") 
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define USE_FLIPMODE 1

// sky box GUI cheaker
static int currentSkybox = 3;
const char* skyboxes[] = { "OutDoor", "InDoor", "BlueSky", "RedSky" };


bool App::OnInit()
{
    if (!D3D::Init(hWnd, screenWidth, screenHeight)) return false;
    if (!InitResource()) return false;
    if (!InitGUI()) return false;

    lightRenderer = new LightRenderer();
    shadowRenderer = new ShadowRenderer();
    geometryRenderer = new GeometryRenderer();
    bloomRenderer = new BloomRenderer();
    postRenderer = new PostProcessRenderer();

    return true;
}

void App::OnUninit()
{
    delete lightRenderer;
    delete geometryRenderer;
    delete shadowRenderer;
    delete bloomRenderer;
    delete postRenderer;

    UninitGUI();
    UninitRenderPipeLine();
    D3D::UnInit();
    CheckDXGIDebug();
}

void App::OnUpdate()
{
    // View, Projection
    camera.GetViewMatrix(view);
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    // Shadow View, Projection
    Vector3 lightDir;
    for (auto& light : lights)
    {
        if (light.isSunLight)
        {
            lightDir = light.direction;     // sunlight는 하나밖에 없음
            break;
        }
    }
    shadowCamera.Udate(camera, lightDir, shadowOrthoDesc);

    // Local, Model, World
    for (auto& m : static_models) m->Update();
    for (auto& m : rigid_models) m->Update();
    for (auto& m : skeletal_models) m->Update();
    
    // Memory Cheak
    memory_debugger.CheakMemoryUsage();

    // Trim
    if (Input::GetKeyDown('T'))
        memory_debugger.Trim();
}

void App::OnRender()
{
    // Defualt Stage Setting
    DefualtStageSetting();         // binding + CB udpate

    // Render Pass
    ShadowMapPass();        // Shadow Map
    GeometryPass();         // G-Buffer
    StencilPass();          // Light Volume Stencil
    LightingPass();         // Shadow + Lighting
    SkyBoxRender();         // Skybox
    BloomProcess();         // Bloom Prefilter -> DownSample -> UpSample
    PostProcess();          // ToneMapping + PostProcess + ScreenFx

    // Debug Draw
    if (frustumON)
    {
        //FrustumDebugDraw(view, projection, view, projection, Colors::FloralWhite);
        FrustumDebugDraw(shadowCamera.GetView(), shadowCamera.GetProjection(),
            view, projection, Colors::GreenYellow);
    }

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
    D3D::transformCBData.shadowView = XMMatrixTranspose(shadowCamera.GetView());
    D3D::transformCBData.shadowProjection = XMMatrixTranspose(shadowCamera.GetProjection());
    D3D::transformCBData.cameraPos = camera.position;
    XMMATRIX invVP = XMMatrixInverse(nullptr, view * projection);
    D3D::transformCBData.invViewProjection = XMMatrixTranspose(invVP);
    D3D::transformCBData.screenSize = { (float)screenWidth,(float)screenHeight };

    D3D::materialCBData.useBaseColorOverride = useBaseColorOverride ? 1 : 0;
    D3D::materialCBData.useEmissiveOverride = useEmissiveOverride ? 1 : 0;
    D3D::materialCBData.useMetallicOverride = useMetallicOverride ? 1 : 0;
    D3D::materialCBData.useRoughnessOverride = useRoughnessOverride ? 1 : 0;

    D3D::debugCBData.lightVolumeON = lightVolumeON ? 1 : 0;

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

    D3D::deviceContext->UpdateSubresource(D3D::debugBuffer.Get(), 0, nullptr, &D3D::debugCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::postprocessBuffer.Get(), 0, nullptr, &D3D::postprocessCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::screenFxBuffer.Get(), 0, nullptr, &D3D::screenFxCBData, 0, 0);
    D3D::deviceContext->UpdateSubresource(D3D::bloomBuffer.Get(), 0, nullptr, &D3D::bloomCBData, 0, 0);
}

////////////////////////////////////////////////////////////////////
/////////////           Rendering Pass                //////////////
////////////////////////////////////////////////////////////////////

// [ Shadow Map Pass ]
void App::ShadowMapPass()
{
    Matrix view = shadowCamera.GetView();
    Matrix projection = shadowCamera.GetProjection();
    shadowRenderer->ShadowMapPass(view, projection, static_models, rigid_models, skeletal_models);
}

// [ Geometry Pass ]
// G-Buffer에 라이팅에 필요한 정보 기록 (albedo, normal, metallic/roughness, emissive, position)
void App::GeometryPass()
{
    geometryRenderer->GeometryPass(view, projection, static_models, rigid_models, skeletal_models);
}


// [ Stencil Pass ]
//  Lighting Volume을 그리며 Stencil Buffer에 라이팅 연산 영역 마크
//  라이팅 연산 영역이란 ? 라이팅 볼륨 안의 픽셀중 G-Buffer의 깊이값보다 가까운 픽셀
//  RTV는 바인딩 하지 않고 Stecnil Buffer만 사용한다.
void App::StencilPass()
{
    lightRenderer->StencilPass(lights, camera);
}


// [ Lighting Pass ]
//  G-Buffer를 샘플링하여 라이팅 계산
//  Directional : Full Screen Quad
//  Point, Spot : Light Volume + ★Stencil Test★
void App::LightingPass()
{
    // CB
    D3D::lightingCBData.useIBL = useIBL ? 1 : 0;

    // SRV
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

    // Renderer Pass Call
    lightRenderer->LightingPass(lights, camera);
}

// [ Skybox Render ]
// Deferred 렌더링에서 스카이박스는 Lighting Pass 이후의 
// 비어있는 픽셀에 기록한다. (Depth Test)
void App::SkyBoxRender()
{
    // RTV, DSV
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);
    D3D::deviceContext->OMSetRenderTargets(1, D3D::sceneHDRRTV.GetAddressOf(), D3D::depthStencilView.Get());
    D3D::deviceContext->OMSetDepthStencilState(D3D::depthTestOnlyDSS.Get(), 0);

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
}

// [ BloomProcess Pass ]
// Prefilter -> DownSample+Blur -> UpSample+Combine
void App::BloomProcess()
{
    bloomRenderer->BloomPass();
}

// [ PostProcess Pass ]
// ToneMapping(LDR) + PostProcess
// Tone Mapping 패스는 화면을 덮는 FullScreen 사각형을 그리면서,
// HDR SRV를 샘플링해 색을 계산하고, 그 결과를 BackBuffer에 기록하는 단계
void App::PostProcess()
{
    postRenderer->PostProcessPass();
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////



bool App::InitResource()
{
    // skybox
    {
        skybox1.InitRenderPipeLine(L"../Resource/Skybox/skybox_cubmap.dds");
        skybox2.InitRenderPipeLine(L"../Resource/Skybox/indoorEnvHDR.dds");
        skybox3.InitRenderPipeLine(L"../Resource/Skybox/blueskyEnvHDR.dds");
        skybox4.InitRenderPipeLine(L"../Resource/Skybox/redskyEnvHDR.dds");
    }

    // model
    {
        StaticModel* floor = AssetManager::Instance().LoadStaticModelAsset("../Resource/Plane.fbx");
        StaticModel* tree = AssetManager::Instance().LoadStaticModelAsset("../Resource/Tree.fbx");
        StaticModel* zelda = AssetManager::Instance().LoadStaticModelAsset("../Resource/zeldaPosed001.fbx");
        RigidModel* character = AssetManager::Instance().LoadRigidModelAsset("../Resource/char.fbx");
        SkeletalModel* girl = AssetManager::Instance().LoadSkeletalModelAsset("../Resource/Girl.fbx");
        SkeletalModel* enemy = AssetManager::Instance().LoadSkeletalModelAsset("../Resource/Enemy.fbx");

        vector<StaticModel*> spheres;
        vector<StaticModel*> torus;

        for (int i = 0; i < 10; i++)
        {
            auto model = AssetManager::Instance().LoadStaticModelAsset("../Resource/sphere.fbx");
            spheres.push_back(model);
            static_models.push_back(model);
            spheres[i]->SetPosition({ -900 + i * 200.0f, 50.0f, 1000 });
            spheres[i]->SetScale({ 0.85,0.85,0.85 });
        }

        for (int i = 0; i < 10; i++)
        {
            auto model = AssetManager::Instance().LoadStaticModelAsset("../Resource/Torus.fbx");
            torus.push_back(model);
            static_models.push_back(model);
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

        // render용 배열에 추가
        static_models.push_back(floor);
        static_models.push_back(tree);
        static_models.push_back(zelda);
        rigid_models.push_back(character);
        skeletal_models.push_back(girl);
        skeletal_models.push_back(enemy);
    }

    // light
    {
        D3D::lightingCBData.indirectIntensity = 0.05f;
        D3D::lightingCBData.useIBL = 1;

        Light directionalLight(LightType::Directional, true);
        directionalLight.direction = { -0.3f,-0.5, 1 };
        directionalLight.color = { 1.0f, 0.9608f, 0.8980f };
        directionalLight.intensity = 0.1f;
        lights.push_back(directionalLight);

        directionalLight.direction = { 0.3f,-0.5, 1 };
        lights.push_back(directionalLight);

        Light pointLight(LightType::Point);
        pointLight.position = { -500,10,0 };
        pointLight.color = { 1.0f, 0.0f, 0.0f };
        pointLight.intensity = 1000.0f;
        pointLight.range = 50.0f;
        for (int i = 0; i < 3; i++)
        {
            pointLight.position.x += 300;
            lights.push_back(pointLight);
        }

        Light spotLight(LightType::Spot);
        spotLight.position = { -650, 100, 0 };
        spotLight.direction = { 0,-1, 0 };
        spotLight.color = { 0.0f, 0.0f, 1.0f };
        spotLight.intensity = 1000.0f;
        spotLight.range = 200.0f;
        spotLight.innerAngle = 10.0f;
        spotLight.outerAngle = 20.0f;
        for (int i = 0; i < 3; i++)
        {
            spotLight.position.x += 300;
            lights.push_back(spotLight);
        }
    }

    // view maxtrix
    camera.position = { 0, 80, -300 };
    camera.moveSpeed = 300.f;
    camera.GetViewMatrix(view);

    // projection matrix 
    projection = XMMatrixPerspectiveFovLH(camera.FovY, screenWidth / (FLOAT)screenHeight, camera.Near, camera.Far);

    // debug settup
    useBaseColorOverride = 1;
    useMetallicOverride = 1;
    useRoughnessOverride = 1;
    D3D::materialCBData.metallicOverride = 0.0f;
    D3D::materialCBData.roughnessOverride = 1.0f;
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

    // IBL Textures Load
    {
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
    }

    return true;
}

void App::UninitRenderPipeLine()
{
    skybox1.UninitRenderPipeLine();
    skybox2.UninitRenderPipeLine();
    skybox3.UninitRenderPipeLine();
    skybox4.UninitRenderPipeLine();
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

    // Light
    {
        ImGui::Begin("[Light]");
        auto GetCountForType = [&](LightType type) -> size_t
            {
                size_t c = 0;
                for (auto& l : lights) if (l.type == type) c++;
                return c;
            };

        const size_t dirCount = GetCountForType(LightType::Directional);
        const size_t pointCount = GetCountForType(LightType::Point);
        const size_t spotCount = GetCountForType(LightType::Spot);
        
        ImGui::Text("Directional Count : %zu", dirCount);
        ImGui::Text("Point Count       : %zu", pointCount);
        ImGui::Text("Spot Count        : %zu", spotCount);

        if(lights.empty())
        {
            ImGui::Text("No Lights Available");
            ImGui::End();
            return;
        }

        ImGui::Checkbox("LightVolume Debug ON", &lightVolumeON);

        // --- selection state (static: keep selection across frames) ---
        static int uiType = 0;          // 0:Dir, 1:Point, 2:Spot
        static int uiIndexInType = 0;   // index inside the selected type list

        const char* typeLabels[] = { "Directional", "Point", "Spot" };
        ImGui::Text("Select");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        ImGui::Combo("##LightType", &uiType, typeLabels, IM_ARRAYSIZE(typeLabels));

        LightType selectedType = LightType::Directional;
        if (uiType == 1) selectedType = LightType::Point;
        else if (uiType == 2) selectedType = LightType::Spot;

        // --- build index map for selected type ---
        std::vector<int> indices;
        indices.reserve(lights.size());
        for (int i = 0; i < (int)lights.size(); ++i)
            if (lights[i].type == selectedType)
                indices.push_back(i);

        if (indices.empty())
        {
            ImGui::TextDisabled("No lights of selected type.");
            ImGui::End();
            return;
        }

        // clamp selection if counts changed
        if (uiIndexInType < 0) uiIndexInType = 0;
        if (uiIndexInType >= (int)indices.size()) uiIndexInType = (int)indices.size() - 1;

        // pick current light (global index)
        int curLightIndex = indices[uiIndexInType];
        Light& cur = lights[curLightIndex];

        // --- select which light within this type ---
        ImGui::SetNextItemWidth(250.0f);
        ImGui::SliderInt("Current Light", &uiIndexInType, 0, (int)indices.size() - 1);

        ImGui::Text("Global Index : %d", curLightIndex);
        ImGui::Separator();

        // --- common props ---
        ImGui::ColorEdit3("Color", &cur.color.x);
        ImGui::InputFloat("Intensity", &cur.intensity, 0.0f, 1000.0f, "%.2f");

        // --- per-type props ---
        if (cur.type == LightType::Directional)
        {
            ImGui::SliderFloat3("Direction", &cur.direction.x, -1.0f, 1.0f, "%.2f");
        }
        else if (cur.type == LightType::Point)
        {
            ImGui::SliderFloat3("Position", &cur.position.x, -500.0f, 500.0f, "%.2f");
            ImGui::SliderFloat("Range", &cur.range, 0.1f, 500.0f, "%.2f");
        }
        else if (cur.type == LightType::Spot)
        {
            ImGui::SliderFloat3("Position", &cur.position.x, -500.0f, 500.0f, "%.2f");
            ImGui::SliderFloat3("Direction", &cur.direction.x, -1.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Range", &cur.range, 0.1f, 500.0f, "%.2f");

            // degree
            ImGui::SliderFloat("Inner Angle", &cur.innerAngle, 0.0f, 89.0f, "%.1f deg");
            ImGui::SliderFloat("Outer Angle", &cur.outerAngle, 0.0f, 89.0f, "%.1f deg");

            // inner <= outer
            if (cur.innerAngle > cur.outerAngle)
                cur.innerAngle = cur.outerAngle;
        }

        ImGui::Separator();
        ImGui::End();
    }

    // Model
    {
        ImGui::Begin("Model", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);
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
        ImGui::InputFloat3("position", &skeletal_models[0]->position.x);
        ImGui::SliderAngle("Pitch", &skeletal_models[0]->rotation.x, 0.0f, 360.0f);
        ImGui::SliderAngle("Yaw", &skeletal_models[0]->rotation.y, 0.0f, 360.0f);
        ImGui::SliderAngle("Roll", &skeletal_models[0]->rotation.z, 0.0f, 360.0f);
        ImGui::InputFloat3("scale", &skeletal_models[0]->scale.x);
        ImGui::End();
    }

    // Camera
    {
        ImGui::Begin("[Camera]");
        ImGui::SliderFloat("Near", &camera.Near, 0.01f, 10000.0f);
        ImGui::SliderFloat("Far", &camera.Far, 0.01f, 10000.0f);

        ImGui::SliderFloat("FOV", &fovDeg, 20.0f, 90.0f);

        camera.FovY = XMConvertToRadians(fovDeg);
        camera.FovY = std::clamp(camera.FovY, 0.3f, 1.7f);
        ImGui::InputFloat("Move Speec", &camera.moveSpeed, 0.0f, 0.0f, "%.3f");
        ImGui::End();
    }

    // Shadow
    {
        ImGui::Begin("[Shadow]");
        ImGui::Text("[Shadow Frustum]");
        ImGui::Checkbox("Frustum Debug ON", &frustumON);
        ImGui::SliderFloat("Near", &shadowOrthoDesc.shadowNear, 0.01f, 10000.0f);
        ImGui::SliderFloat("Far", &shadowOrthoDesc.shadowFar, 0.01f, 10000.0f);
        ImGui::InputFloat("Width", &shadowOrthoDesc.shadowWidth);
        ImGui::InputFloat("Height", &shadowOrthoDesc.shadowHeight);

        ImGui::Text("");
        ImGui::Text("[Shadow Light Pos]");
        ImGui::SliderFloat("lookPointDist", &shadowOrthoDesc.lookPointDist, 1.f, 5000.0f);
        ImGui::SliderFloat("shadowLightDist", &shadowOrthoDesc.shadowLightDist, 1.f, 5000.0f);
        ImGui::End();
    }

    // IBL
    {
        ImGui::Begin("[IBL]");
        ImGui::Checkbox("use IBL", &useIBL);
        ImGui::SliderFloat("Indirect Intensity", &D3D::lightingCBData.indirectIntensity, 0.0f, 10.0f);

        ImGui::Text("");
        ImGui::Text("[Skybox]");
        ImGui::Combo("Skybox Mode", &currentSkybox, skyboxes, IM_ARRAYSIZE(skyboxes));
        ImGui::End();
    }

    // PostProcess
    {
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
        ImGui::SliderFloat("Bloom Intensity", &D3D::bloomCBData.bloom_intensity, 0.0f, 10.0f, "%.2f");
        ImGui::SliderFloat("Clamp", &D3D::bloomCBData.bloom_clamp, 0.0f, 20.0f, "%.2f");
        ImGui::SliderFloat("Scatter", &D3D::bloomCBData.bloom_scatter, 0.0f, 1.0f, "%.2f");
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
    }

    // Screen Space Effect
    {
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
    }

    // G-Buffer
    {
        ImGui::Begin("[G-Buffer]");
        const ImVec2 size(screenWidth / 10, screenHeight / 10);

        if (ImGui::BeginTable("GBufferTable", 2,
            ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit))
        {
            auto Cell = [&](const char* label, ID3D11ShaderResourceView* srv)
                {
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(label);

                    if (srv)
                        ImGui::Image((ImTextureID)srv, size);
                    else
                        ImGui::Dummy(size);
                };

            // Row 1
            Cell("Position", D3D::positionSRV.Get());
            Cell("Albedo", D3D::albedoSRV.Get());

            // Row 2
            Cell("Normal", D3D::normalSRV.Get());
            Cell("Metal/Rough", D3D::metalRoughSRV.Get());

            // Row 3
            Cell("Emissive", D3D::emissiveSRV.Get());
            Cell("Depth", D3D::depthSRV.Get());

            ImGui::EndTable();
        }
        ImGui::End();
    }

    // Memory
    {
        ImGui::Begin("[Memory Debugger]");
        ImGui::Text("[T] Trim");
        ImGui::Text("%ls", memory_debugger.GetMemoryUsageWstring().c_str());
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

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

    // UnBind
    const float blendFactor[4] = { 0,0,0,0 };
    D3D::deviceContext.Get()->OMSetBlendState(nullptr, blendFactor, 0xFFFFFFFF);
    D3D::deviceContext.Get()->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext.Get()->RSSetState(nullptr);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK App::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    return __super::WindowProc(hWnd, message, wParam, lParam);
}