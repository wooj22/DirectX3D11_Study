#include "ParticleRenderer.h"
#include "D3D.h"

ParticleRenderer::ParticleRenderer()
{
    UINT maxInstances = 100;

    D3D11_BUFFER_DESC desc{};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(ParticleInstance) * maxInstances;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;      // Map !
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;   // Map !
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    D3D::device->CreateBuffer(&desc, nullptr, instanceBuffer.ReleaseAndGetAddressOf());
}

void ParticleRenderer::ParticlePass(const vector<Effect*>& effects)
{
    auto* ctx = D3D::deviceContext.Get();

    // IA
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->IASetInputLayout(D3D::inputLayout_Particle.Get());

    // Shader
    ctx->VSSetShader(D3D::VS_Effect.Get(), nullptr, 0);
    ctx->PSSetShader(nullptr, nullptr, 0);

    // SRV


    // Sampler
    ctx->PSSetSamplers(0, 1, D3D::linearSamplerState.GetAddressOf());

    // State
    float blendFactor[4] = { 0,0,0,0 };
    ctx->OMSetBlendState(D3D::alphaBlendState.Get(), blendFactor, 0xffffffff);
    ctx->OMSetDepthStencilState(D3D::depthTestOnlyDSS.Get(), 0);
    ctx->RSSetState(D3D::cullNoneRS.Get());

    // CB


    // alive == true인 파티클 배열
    vector<ParticleInstance> instances;
    instances.reserve(effects.size());

    for (const auto& e : effects)
    {
        if (!e || !e->alive) continue;

        ParticleInstance i{};
        i.pos = e->particle.pos;
        i.rotation = e->particle.rotation;
        i.size = e->particle.size;
        i.color = e->particle.color;
        i.frame = e->frame;

        instances.push_back(i);
    }

    if (instances.empty()) return;

    // Instance Buffer
    D3D11_MAPPED_SUBRESOURCE mapped{};

    ctx->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);               // gpu buffer의 주소를 mapped에 빌려담음
    memcpy(mapped.pData, instances.data(), instances.size() * sizeof(ParticleInstance));  // gpu buffer 내용 변경
    ctx->Unmap(instanceBuffer.Get(), 0);    // gpu야 잘썼엉

    // Render
    // Quad 1개 + Instance N개 -> N번 반복해서 Draw
    quad.DrawIndexedInstanced((UINT)instances.size(), instanceBuffer.Get(), sizeof(ParticleInstance));
}