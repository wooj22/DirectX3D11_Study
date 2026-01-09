#include "ParticleRenderer.h"
#include "D3D.h"
#include "Camera.h"
#include <algorithm>

UINT maxParticleInstances = 1000;

void ParticleRenderer::Init()
{
    // Particle Quad Mesh Create
    quad.Init();

    // Instance Buffer Create
    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(ParticleInstance) * maxParticleInstances;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;      // Map !
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // Map !
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D::device.Get()->CreateBuffer(&desc, nullptr, instanceBuffer.ReleaseAndGetAddressOf());
    assert(instanceBuffer != nullptr);
}

void ParticleRenderer::ParticlePass(Camera& camera, const vector<Effect>& effects)
{
    auto* ctx = D3D::deviceContext.Get();
    Matrix view = camera.GetView();
    Matrix projection = camera.GetProjection();

    // RTV, DSV
    ctx->RSSetViewports(1, &D3D::viewport_screen);
    ctx->OMSetRenderTargets(1, D3D::sceneHDRRTV.GetAddressOf(), D3D::depthStencilView.Get());

    // IA
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(D3D::inputLayout_Particle.Get());

    // DSS
    ctx->OMSetDepthStencilState(D3D::depthTestOnlyDSS.Get(), 0);

    // RS
    ctx->RSSetState(D3D::cullNoneRS.Get());

    // Shader
    ctx->VSSetShader(D3D::VS_Effect.Get(), nullptr, 0);
    ctx->PSSetShader(D3D::PS_Effect.Get(), nullptr, 0);

    // Sampler
    ctx->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());

    // Blend State (alpha)
    float blendFactor[4] = { 0,0,0,0 };
    ctx->OMSetBlendState(D3D::alphaBlendState.Get(), blendFactor, 0xffffffff);
    
    // CB
    D3D::transformCBData.view = XMMatrixTranspose(view);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);


    // Effect Sort (Back-to-Front)
    vector<FxSortItem> sortedFx;
    sortedFx.reserve(effects.size());

    const Vector3 camPos = camera.position; 
    const Vector3 camForward = camera.GetForward();

    for (const auto& fx : effects)
    {
        if (!fx.enabled || !fx.playing) continue;

        Vector3 dist = fx.position - camPos;
        float key = dist.Dot(camForward);
        sortedFx.push_back({ &fx, key });
    }

    std::sort(sortedFx.begin(), sortedFx.end(),
        [](const FxSortItem& a, const FxSortItem& b) { return a.key > b.key; });

    
    // Effect Render
    vector<ParticleInstance> instances;
    instances.reserve(256);

    for (const auto& item : sortedFx)
    {
        const auto& fx = *item.fx;

        // Emitter ´ÜÀ§ Local Batching
        for (const auto& em : fx.emitters)
        {
            if (!em.enabled) continue;
            if (em.particles.empty()) continue;

            // CB
            D3D::effectCBData.atlasGrid = { (float)em.sheet.cols , (float)em.sheet.rows };
            D3D::effectCBData.invAtlasGrid = { 1.0f / (float)em.sheet.cols, 1.0f / (float)em.sheet.rows };
            D3D::effectCBData.baseSizeScale = em.sheet.baseSizeScale;
            D3D::effectCBData.billboardType = (int)em.billboard;
            D3D::deviceContext->UpdateSubresource(D3D::effectBuffer.Get(), 0, nullptr, &D3D::effectCBData, 0, 0);

            // SRV
            D3D::deviceContext->PSSetShaderResources(20, 1, em.sheet.srv.GetAddressOf());

            // Instance Buffer (particle)
            instances.clear();
            instances.reserve(em.particles.size());

            for (const auto& p : em.particles)
            {
                ParticleInstance i{};
                i.pos = p.pos;
                i.rotation = p.rotation;
                i.size = p.size;
                i.color = p.color;
                i.frame = p.frame;
                instances.push_back(i);
            }

            if (instances.empty()) continue;

            // Map
            D3D11_MAPPED_SUBRESOURCE mapped{};
            ctx->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
            memcpy(mapped.pData, instances.data(), instances.size() * sizeof(ParticleInstance));
            ctx->Unmap(instanceBuffer.Get(), 0);

            // Render
            quad.DrawIndexedInstanced((UINT)instances.size(), instanceBuffer.Get(), sizeof(ParticleInstance));
        }
    }

    // clear
    ctx->RSSetState(nullptr);
    ctx->OMSetDepthStencilState(nullptr, 0);
    ctx->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}