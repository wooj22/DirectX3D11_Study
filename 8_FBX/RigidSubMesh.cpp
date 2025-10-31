#include "RigidSubMesh.h"

void RigidSubMesh::CreateBuffer()
{
    // vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices.data();
    vertexBufferStride = sizeof(Vertex);
    vertexBufferOffset = 0;

    HRESULT hr = D3D::device.Get()->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
    if (FAILED(hr)) OutputDebugStringA("Failed to create vertex buffer\n");

    // index buffer
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(WORD) * indices.size();
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();
    indexCount = indices.size();

    hr = D3D::device.Get()->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
    if (FAILED(hr)) OutputDebugStringA("Failed to create index buffer\n");
}