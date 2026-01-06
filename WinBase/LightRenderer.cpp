#include "LightRenderer.h"
#include "Light.h"
#include "LightVolumeMesh.h"
#include "Camera.h"
#include "D3D.h"
#include "Structures.hpp"
using namespace std;

LightRenderer::LightRenderer()
{
    // light volume create
    sphereVolume = CreateLightVolumeSphere(D3D::device.Get(), 24, 16);
    coneVolume = CreateLightVolumeCone(D3D::device.Get(), 24, false);
}


// [ Stencil Pass ]
//  Lighting Volume을 그리며 Stencil Buffer에 라이팅 연산 영역 마크
//  라이팅 연산 영역이란 ? 라이팅 볼륨 안의 픽셀중 G-Buffer의 깊이값보다 가까운 픽셀
//  RTV는 바인딩 하지 않고 Stecnil Buffer만 사용한다.
void LightRenderer::StencilPass(const vector<Light>& lights, const Camera& camera)
{
    // RTV, DSV (Stencil)
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);
    D3D::deviceContext->OMSetRenderTargets(0, nullptr, D3D::depthStencilView.Get());
    D3D::deviceContext->ClearDepthStencilView(D3D::depthStencilView.Get(),
        D3D11_CLEAR_STENCIL, 1.0f, 0);  // Stencil만 0으로 초기화

    // DSS
    const UINT stencilRef = 1;          // Stencil Reference Value
    D3D::deviceContext->OMSetDepthStencilState(D3D::depthTestStencilWriteDSS.Get(), stencilRef);

    // RS (원래 outside는 cullBack인데, mesh가 뒤집혀있는듯?)
    D3D::deviceContext.Get()->RSSetState(D3D::cullfrontRS.Get());

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D::deviceContext.Get()->IASetInputLayout(D3D::inputLayout_Position.Get());

    // Shader
    D3D::deviceContext.Get()->VSSetShader(D3D::VS_LightVolume.Get(), nullptr, 0);
    D3D::deviceContext->PSSetShader(nullptr, nullptr, 0);   // PS x

    // Render
    for (const Light& light : lights)
    {
        if (light.type == LightType::Point)
        {
            sphereVolume->UpdateWolrd(light);
            sphereVolume->Draw(light, camera);
        }
        else if (light.type == LightType::Spot)
        {
            coneVolume->UpdateWolrd(light);
            coneVolume->Draw(light, camera);
        }
    }

    // clear
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->RSSetState(nullptr);
}


// [ Lighting Pass ]
//  G-Buffer를 샘플링하여 라이팅 계산
//  - Directional : Full Screen Quad
//  - Point, Spot : Light Volume + Stencil Test
void LightRenderer::LightingPass(const vector<Light>& lights, const Camera& camera)
{
    // RTV, DSV
    D3D::deviceContext->RSSetViewports(1, &D3D::viewport_screen);
    D3D::deviceContext->OMSetRenderTargets(1, D3D::sceneHDRRTV.GetAddressOf(), D3D::depthStencilReadOnlyView.Get());
    D3D::deviceContext->ClearRenderTargetView(D3D::sceneHDRRTV.Get(), clearColor);

    // IA
    D3D::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // PS
    D3D::deviceContext->PSSetShader(D3D::PS_DeferredLighting.Get(), NULL, 0);

    // Blend State (Additive)
    D3D::deviceContext->OMSetBlendState(D3D::additiveBlendState.Get(), nullptr, 0xffffffff);

    // Sampler
    D3D::deviceContext->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());
    D3D::deviceContext->PSSetSamplers(1, 1, D3D::shadowSamplerState.GetAddressOf());
    D3D::deviceContext->PSSetSamplers(2, 1, D3D::linearClamSamplerState.GetAddressOf());

    // SRV
    D3D::deviceContext->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    D3D::deviceContext->PSSetShaderResources(15, 1, D3D::albedoSRV.GetAddressOf());
    D3D::deviceContext->PSSetShaderResources(16, 1, D3D::normalSRV.GetAddressOf());
    D3D::deviceContext->PSSetShaderResources(17, 1, D3D::metalRoughSRV.GetAddressOf());
    D3D::deviceContext->PSSetShaderResources(18, 1, D3D::emissiveSRV.GetAddressOf());
    D3D::deviceContext->PSSetShaderResources(19, 1, D3D::depthSRV.GetAddressOf());

    // Render
    for (const Light& light : lights)
    {
        D3D::lightingCBData.lightType = static_cast<int>(light.type);
        D3D::lightingCBData.isSunLight = light.isSunLight;
        D3D::lightingCBData.lightColor = light.color;
        D3D::lightingCBData.directIntensity = light.intensity;
        D3D::lightingCBData.lightDirection = light.direction;
        D3D::lightingCBData.lightPos = light.position;
        D3D::lightingCBData.lightRange = light.range;
        D3D::lightingCBData.innerAngle = light.innerAngle;
        D3D::lightingCBData.outerAngle = light.outerAngle;
        D3D::deviceContext->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);

        // Light Volume 렌더링
        if (light.type == LightType::Directional)
        {
            // Stencil Test off
            D3D::deviceContext->OMSetDepthStencilState(D3D::disableDSS.Get(), 0);

            // RS
            D3D::deviceContext.Get()->RSSetState(nullptr);

            // Full Screen Quad
            D3D::deviceContext->IASetInputLayout(nullptr);
            D3D::deviceContext->VSSetShader(D3D::VS_FullScreen.Get(), NULL, 0);
            D3D::deviceContext.Get()->Draw(3, 0);
        }
        else
        {
            // Light Volume
            if (light.type == LightType::Point)
            {
                if (sphereVolume->IsInsidePointLight(camera.position, light.position, light.range))
                {
                    // Stencil Test off
                    D3D::deviceContext->OMSetDepthStencilState(D3D::disableDSS.Get(), 0);

                    // RS
                    D3D::deviceContext.Get()->RSSetState(nullptr);

                    // Full Screen Quad
                    D3D::deviceContext->IASetInputLayout(nullptr);
                    D3D::deviceContext->VSSetShader(D3D::VS_FullScreen.Get(), NULL, 0);
                    D3D::deviceContext.Get()->Draw(3, 0);
                }
                else
                {
                    // Stencil Test on
                    D3D::deviceContext->OMSetDepthStencilState(D3D::stencilTestOnlyDSS.Get(), 1);

                    // RS
                    D3D::deviceContext.Get()->RSSetState(D3D::cullfrontRS.Get());

                    // Light Volume
                    D3D::deviceContext.Get()->IASetInputLayout(D3D::inputLayout_Position.Get());
                    D3D::deviceContext.Get()->VSSetShader(D3D::VS_LightVolume.Get(), nullptr, 0);
                    sphereVolume->UpdateWolrd(light);
                    sphereVolume->Draw(light, camera);
                }
            }
            else if (light.type == LightType::Spot)
            {
                if (sphereVolume->IsInsideSpotLight(camera.position, light.position,
                    light.direction, light.range, light.outerAngle))
                {
                    // Stencil Test off
                    D3D::deviceContext->OMSetDepthStencilState(D3D::disableDSS.Get(), 0);

                    // RS
                    D3D::deviceContext.Get()->RSSetState(nullptr);

                    // Full Screen Quad
                    D3D::deviceContext->IASetInputLayout(nullptr);
                    D3D::deviceContext->VSSetShader(D3D::VS_FullScreen.Get(), NULL, 0);
                    D3D::deviceContext.Get()->Draw(3, 0);
                }
                else
                {
                    // Stencil Test on
                    D3D::deviceContext->OMSetDepthStencilState(D3D::stencilTestOnlyDSS.Get(), 1);

                    // RS
                    D3D::deviceContext.Get()->RSSetState(D3D::cullfrontRS.Get());

                    // Light Volum
                    D3D::deviceContext.Get()->IASetInputLayout(D3D::inputLayout_Position.Get());
                    D3D::deviceContext.Get()->VSSetShader(D3D::VS_LightVolume.Get(), nullptr, 0);
                    coneVolume->UpdateWolrd(light);
                    coneVolume->Draw(light, camera);
                }
            }
        }
    }

    // clear
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
    D3D::deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    D3D::deviceContext->RSSetState(nullptr);

    // RTV - SRV hazard 방지
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    D3D::deviceContext->PSSetShaderResources(6, 1, nullSRV);   // shadow
    D3D::deviceContext->PSSetShaderResources(9, 1, nullSRV);   // IBL irradiance
    D3D::deviceContext->PSSetShaderResources(10, 1, nullSRV);  // IBL spec
    D3D::deviceContext->PSSetShaderResources(11, 1, nullSRV);  // BRDF LUT
    D3D::deviceContext->PSSetShaderResources(15, 1, nullSRV);  // albedo
    D3D::deviceContext->PSSetShaderResources(16, 1, nullSRV);  // normal
    D3D::deviceContext->PSSetShaderResources(17, 1, nullSRV);  // metalRough
    D3D::deviceContext->PSSetShaderResources(18, 1, nullSRV);  // emissive
    D3D::deviceContext->PSSetShaderResources(19, 1, nullSRV);  // depth
}