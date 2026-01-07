#include "ParticleQuadMesh.h"
#include "D3D.h"

void ParticleQuadMesh::Init()
{
    // Quad: corner(-0.5~0.5), UV(0~1)
    ParticleQuadVertex vertices[4] =
    {
        // left  top
        { Vector2(-0.5f,  0.5f), Vector2(0.0f, 0.0f) },
        // right top
        { Vector2(0.5f,  0.5f), Vector2(1.0f, 0.0f) },
        // right bottom
        { Vector2(0.5f, -0.5f), Vector2(1.0f, 1.0f) },
        // left  bottom
        { Vector2(-0.5f, -0.5f), Vector2(0.0f, 1.0f) },
    };

    // Index
    // 0-1-2, 0-2-3
    uint16_t indices[6] = { 0, 1, 2, 0, 2, 3 };

    // --- Vertex Buffer ---
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = UINT(sizeof(vertices));
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;

    D3D::device.Get()->CreateBuffer(&vbDesc, &vbData, vertexBuffer.GetAddressOf());

    // --- Index Buffer ---
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = UINT(sizeof(indices));
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;

    D3D::device.Get()->CreateBuffer(&ibDesc, &ibData, indexBuffer.GetAddressOf());
}

void ParticleQuadMesh::DrawIndexedInstanced(UINT instanceCount, ID3D11Buffer* instanceVB, UINT instanceStride)
{
    auto* ctx = D3D::deviceContext.Get();

    // VB, IB
    // Vertex Data + Instance Data
    ID3D11Buffer* vbs[] = { vertexBuffer.Get(), instanceVB };
    UINT strides[] = { stride, instanceStride };
    UINT offsets[] = { 0, 0 };

    ctx->IASetVertexBuffers(0, 2, vbs, strides, offsets);
    ctx->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

    // Draw Call
    // Quad 1개 + Instance N개 -> N번 반복해서 Draw
    ctx->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}