#include "DecalRenderer.h"
#include "DecalVolumeMesh.h"
#include "Decal.h"
#include "Camera.h"
#include "D3D.h"
#include "Structures.hpp"

DecalRenderer::~DecalRenderer()
{
    if (decalVolume) delete decalVolume;
}

void DecalRenderer::Init()
{
    decalVolume = CreateDecalVolume(D3D::device.Get());
}

void DecalRenderer::DecalPass(const Camera& camera, const std::vector<Decal>& decals)
{
    auto& context = D3D::deviceContext;

    context->RSSetViewports(1, &D3D::viewport_screen);
    context->OMSetRenderTargets(1, D3D::albedoRTV.GetAddressOf(), D3D::depthStencilReadOnlyView.Get());

    // IA
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(D3D::inputLayout_Position.Get());

    // Shader
    context->VSSetShader(D3D::VS_Decal.Get(), nullptr, 0);
    context->PSSetShader(D3D::PS_Decal.Get(), nullptr, 0);

    // Sampler
    context->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());
    context->PSSetSamplers(2, 1, D3D::linearClamSamplerState.GetAddressOf());

    // SRV
    context->PSSetShaderResources(16, 1, D3D::normalSRV.GetAddressOf());
    context->PSSetShaderResources(19, 1,D3D::depthSRV.GetAddressOf());

    // Blend State
    float blendFactor[4] = { 0,0,0,0 }; UINT sampleMask = 0xffffffff;
    context->OMSetBlendState(D3D::alphaBlendState.Get(), blendFactor, sampleMask);

    // CB - Transform
    auto view = camera.GetView(); auto projection = camera.GetProjection();
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::transformCBData.invViewProjection = XMMatrixTranspose(XMMatrixInverse(nullptr, view * projection));
    context->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);

    const UINT stencilRef = 0x01;   // Stencil Reference Value

    // Render
    for (const Decal& decal : decals)
    {
        // DSS
        context->OMSetDepthStencilState(D3D::depthTestOnlyDSS.Get(), 0);

        // DSS - Stencil Test(ground or all)
        if (decal.isGroundDecal)
            context->OMSetDepthStencilState(D3D::groundTestDSS.Get(), stencilRef);
        else                               
            context->OMSetDepthStencilState(D3D::depthTestOnlyDSS.Get(), 0);

        // CB - Decal
        D3D::decalCBData.opacity = decal.opacity;
        D3D::decalCBData.upThreshold = decal.upThreshold;
        D3D::decalCBData.tiling = decal.tiling;
        D3D::decalCBData.offset = decal.offset;
        D3D::decalCBData.decalType = (int)decal.type;
        D3D::decalCBData.ringStartTime = decal.ringStartTime;
        D3D::decalCBData.ringDuration = decal.ringDuration;
        D3D::decalCBData.ringMaxRadius = decal.ringMaxRadius;
        D3D::decalCBData.ringSpeed = decal.ringSpeed;
        D3D::decalCBData.ringThickness = decal.ringThickness;
        D3D::decalCBData.ringFeather = decal.ringFeather;
        D3D::decalCBData.ringColor = decal.ringColor;
        context->UpdateSubresource(D3D::decalBuffer.Get(), 0, nullptr, &D3D::decalCBData, 0, 0);

        // SRV
        if (decal.decalSRV)
        {
            context.Get()->PSSetShaderResources(21, 1, decal.decalSRV.GetAddressOf());
        }

        // Render
        decalVolume->UpdateWolrd(&decal);
        decalVolume->Draw();
    }

    // clean up
    context->OMSetDepthStencilState(nullptr, 0);
    context->RSSetState(nullptr);

    ID3D11RenderTargetView* nullRTV[1] = { nullptr };
    context->OMSetRenderTargets(1, nullRTV, nullptr);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->PSSetShaderResources(16, 1, nullSRV);  // normal
    context->PSSetShaderResources(19, 1, nullSRV);  // depth
}

