#include "ParticleRenderer.h"
#include "D3D.h"

void ParticleRenderer::Init()
{
    // Particle Quad Mesh Create
    quad.Init();

    // Instance Buffer Create
    UINT maxInstances = 100;

    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(ParticleInstance) * maxInstances;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;      // Map !
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // Map !
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D::device.Get()->CreateBuffer(&desc, nullptr, instanceBuffer.ReleaseAndGetAddressOf());
    assert(instanceBuffer != nullptr);
}

void ParticleRenderer::ParticlePass(const Matrix& view, const Matrix& projection, const vector<Effect>& effects)
{
    auto* ctx = D3D::deviceContext.Get();

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

    // TODO :: Batching
    {
        for (const auto& e : effects)
        {
            if (!e.alive) continue;

            // CB
            D3D::effectCBData.atlasGrid = { (float)e.sheet.cols , (float)e.sheet.rows };
            D3D::effectCBData.invAtlasGrid = { 1.0f / (float)e.sheet.cols, 1.0f / (float)e.sheet.rows };
            D3D::effectCBData.baseSizeScale = e.sheet.baseSizeScale;
            D3D::effectCBData.billboardType = (int)e.billboard;
            D3D::deviceContext->UpdateSubresource(D3D::effectBuffer.Get(), 0, nullptr, &D3D::effectCBData, 0, 0);

            // SRV
            D3D::deviceContext->PSSetShaderResources(20, 1, e.sheet.srv.GetAddressOf());

            // Particle Instance
            // 지금은 Effect에 Particle이 하나지만, 이제 파티클 시스템으로 확장할 예정
            // 꼭 전체 배칭 하지 않고, 일단 Effect별로 DrawCall하는건 어떨까..?
            vector<ParticleInstance> instances;
            instances.reserve(e.particles.size());
            {
                ParticleInstance i{};
                i.pos = e.particle.pos;
                i.rotation = e.particle.rotation;
                i.size = e.particle.size;
                i.color = e.particle.color;
                i.frame = e.frame;

                instances.push_back(i);

                if (instances.empty()) return;
            }

            // Instance Buffer
            D3D11_MAPPED_SUBRESOURCE mapped{};

            ctx->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);               // gpu buffer의 주소를 mapped에 빌려담음
            memcpy(mapped.pData, instances.data(), instances.size() * sizeof(ParticleInstance));  // gpu buffer 내용 변경
            ctx->Unmap(instanceBuffer.Get(), 0);    // gpu야 잘썼엉

            // Render
            // Quad 1개 + Instance N개 -> N번 반복해서 Draw
            quad.DrawIndexedInstanced((UINT)instances.size(), instanceBuffer.Get(), sizeof(ParticleInstance));
        }
    }

    // clear
    ctx->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}