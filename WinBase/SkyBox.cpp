#include "SkyBox.h"
#include "D3D.h"
#include "Helper.h"
#include "Structures.hpp"
#include <Directxtk/DDSTextureLoader.h>

void SkyBox::InitRenderPipeLine()
{
    // Vertex Buffer, Index Buffer
    Skybox_Vertex vertices[] =
    {
        Vector3(-1.0f,  1.0f, -1.0f),
        Vector3(1.0f,  1.0f, -1.0f),
        Vector3(1.0f, -1.0f, -1.0f),
        Vector3(-1.0f, -1.0f, -1.0f),
        Vector3(-1.0f,  1.0f,  1.0f),
        Vector3(1.0f,  1.0f,  1.0f),
        Vector3(1.0f, -1.0f,  1.0f),
        Vector3(-1.0f, -1.0f,  1.0f),
    };

    UINT indices[] =
    {
        0,1,2, 0,2,3, // back
        4,6,5, 4,7,6, // front
        4,5,1, 4,1,0, // top
        3,2,6, 3,6,7, // bottom
        1,5,6, 1,6,2, // right
        4,0,3, 4,3,7  // left
    };
    indexCount = ARRAYSIZE(indices);

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = vertices;
    D3D::device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
    vertexBufferStride = sizeof(Skybox_Vertex);
    vertexBufferOffset = 0;

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    D3D::device->CreateBuffer(&ibDesc, &ibData, &indexBuffer);

    // CubeMap Texture Load
    CreateDDSTextureFromFile(D3D::device.Get(), L"../Resource/skybox_cubmap.dds", nullptr, &skyboxTRV);
}

void SkyBox::Render(Matrix& view, Matrix& projection)
{
    // 카메라 이동행렬 제거 -> 카메라가 이동해도 큐브는 항상 카메라 원점에 고정
    // 스카이 박스 정점은 카메라 좌표계에서 항상 +- 1 정도 거리의 정점으로 유지
    // 큐브는 투영후 카메라의 Far Plane에 수렴하는 값으로 나오고 z(깊이)는 1근처가 됨
    Matrix viewNoTranslation = view;
    viewNoTranslation._41 = 0.0f;
    viewNoTranslation._42 = 0.0f;
    viewNoTranslation._43 = 0.0f;

    // Constant buffer Update
    D3D::transformCBData.view = XMMatrixTranspose(viewNoTranslation);
    D3D::transformCBData.projection = XMMatrixTranspose(projection);
    

    // 렌더 파이프라인 바인딩
    D3D::deviceContext->IASetVertexBuffers(1, 1, &vertexBuffer, &vertexBufferStride, &vertexBufferOffset);
    D3D::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    D3D::deviceContext->IASetInputLayout(D3D::inputLayout_Skybox.Get());
    D3D::deviceContext->VSSetShader(D3D::Skybox_VS.Get(), nullptr, 0);
    D3D::deviceContext->PSSetShader(D3D::Skybox_PS.Get(), nullptr, 0);
    D3D::deviceContext->PSSetShaderResources(4, 1, &skyboxTRV);
    D3D::deviceContext->VSSetConstantBuffers(0, 1, &D3D::transformBuffer);
    D3D::deviceContext->PSSetConstantBuffers(0, 1, &D3D::transformBuffer);
    D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);

    // Rasterizer, DepthStencilState 설정
    //D3DBase::deviceContext->RSSetState(skyboxRS);     // 메쉬 자체가 뒷면 기준이기때문에 생략 가능
    D3D::deviceContext->OMSetDepthStencilState(D3D::depthStencilState.Get(), 1);

    // Draw
    D3D::deviceContext->DrawIndexed(indexCount, 0, 0);

    // Rasterizer, DepthStencilState 원상복귀
    //D3DBase::deviceContext->RSSetState(nullptr);
    D3D::deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void SkyBox::UninitRenderPipeLine()
{
    SAFE_RELEASE(vertexBuffer);
    SAFE_RELEASE(indexBuffer);
    SAFE_RELEASE(skyboxTRV);
}