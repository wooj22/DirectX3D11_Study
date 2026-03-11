#include "ForwardTransparentRenderer.h"
#include "D3D.h"
#include "Structures.hpp"
#include "StaticModel.h"
#include "RigidModel.h"
#include "SkeletalModel.h"
#include "Light.h"
#include "Environment.hpp"
#include "Camera.h"

using namespace std;

void ForwardTransparentRenderer::ForwardTransparentPass(const Matrix& view, const Matrix& projection, const vector<StaticModel*>& static_models, const vector<RigidModel*>& rigid_models, const vector<SkeletalModel*>& skeletal_models, const std::vector<Light>& lights, const Environment& env)
{
    auto& context = D3D::deviceContext;

    // RTV, DSV
    context->RSSetViewports(1, &D3D::viewport_screen);
    context->OMSetRenderTargets(1, D3D::sceneHDRRTV.GetAddressOf(), D3D::depthStencilView.Get());

    // TODO :: depth test only, transparent queue Á¤·Ä drawcall
    context->OMSetDepthStencilState(D3D::defualtDSS.Get(), 0);

    // IA
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // PS
    context->PSSetShader(D3D::PS_ForwardLighting.Get(), NULL, 0);

    // Sampler
    context->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());
    context->PSSetSamplers(1, 1, D3D::shadowSamplerState.GetAddressOf());
    context->PSSetSamplers(2, 1, D3D::linearClamSamplerState.GetAddressOf());

    // Blend State
    float blendFactor[4] = { 0,0,0,0 }; UINT sampleMask = 0xffffffff;
    context->OMSetBlendState(D3D::alphaBlendState.Get(), blendFactor, sampleMask);

    // SRV
    context->PSSetShaderResources(6, 1, D3D::shadowSRV.GetAddressOf());
    context->PSSetShaderResources(9, 1, env.ibl.irradiance.GetAddressOf());
    context->PSSetShaderResources(10, 1, env.ibl.specularEnv.GetAddressOf());
    context->PSSetShaderResources(11, 1, env.ibl.brdfLut.GetAddressOf());

    // CB
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    context->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);

    // Render
    // Static, Rigid Model
    context->IASetInputLayout(D3D::inputLayout_RigidVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_BaseLit_Rigid.Get(), NULL, 0);
    for (auto& m : static_models)
    {
        if (m->blendType == RenderBlendType::Transparent)
        {
            // Light
            for (const Light& light : lights)
            {
                // CB - Light
                D3D::lightingCBData.lightType = static_cast<int>(light.type);
                D3D::lightingCBData.isSunLight = light.isSunLight;
                D3D::lightingCBData.lightColor = light.color;
                D3D::lightingCBData.directIntensity = light.intensity;
                D3D::lightingCBData.lightDirection = light.direction;
                D3D::lightingCBData.lightPos = light.position;
                D3D::lightingCBData.lightRange = light.range;
                D3D::lightingCBData.innerAngle = light.innerAngle;
                D3D::lightingCBData.outerAngle = light.outerAngle;
                context->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);

                // IB, VB, SRV, CB -> DrawCall
                m->Draw();
            }
        }     
    }

    for (auto& m : rigid_models)
    {
        if (m->blendType == RenderBlendType::Transparent)
        {
            // Light
            for (const Light& light : lights)
            {
                // CB - Light
                D3D::lightingCBData.lightType = static_cast<int>(light.type);
                D3D::lightingCBData.isSunLight = light.isSunLight;
                D3D::lightingCBData.lightColor = light.color;
                D3D::lightingCBData.directIntensity = light.intensity;
                D3D::lightingCBData.lightDirection = light.direction;
                D3D::lightingCBData.lightPos = light.position;
                D3D::lightingCBData.lightRange = light.range;
                D3D::lightingCBData.innerAngle = light.innerAngle;
                D3D::lightingCBData.outerAngle = light.outerAngle;
                context->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);

                // IB, VB, SRV, CB -> DrawCall
                m->Draw();
            }
        }
    }

    // Skeletal Model
    context->IASetInputLayout(D3D::inputLayout_BoneWeightVertex.Get());
    D3D::deviceContext->VSSetShader(D3D::VS_BaseLit_Skeletal.Get(), NULL, 0);
    for (auto& m : skeletal_models)
    {
        if (m->blendType == RenderBlendType::Transparent)
        {
            // Light
            for (const Light& light : lights)
            {
                // CB - Light
                D3D::lightingCBData.lightType = static_cast<int>(light.type);
                D3D::lightingCBData.isSunLight = light.isSunLight;
                D3D::lightingCBData.lightColor = light.color;
                D3D::lightingCBData.directIntensity = light.intensity;
                D3D::lightingCBData.lightDirection = light.direction;
                D3D::lightingCBData.lightPos = light.position;
                D3D::lightingCBData.lightRange = light.range;
                D3D::lightingCBData.innerAngle = light.innerAngle;
                D3D::lightingCBData.outerAngle = light.outerAngle;
                context->UpdateSubresource(D3D::lightingBuffer.Get(), 0, nullptr, &D3D::lightingCBData, 0, 0);

                // IB, VB, SRV, CB -> DrawCall
                m->Draw();
            }
        }
    }

    // clean up
    context->OMSetRenderTargets(0, nullptr, nullptr);
    context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->PSSetShaderResources(6, 1, nullSRV);  // shadowMap
}
