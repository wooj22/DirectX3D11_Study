#include "Skybox.h"
#include "../WinBase/Helper.h"
#include <Directxtk/DDSTextureLoader.h>

void Skybox::InitRenderPipeLine()
{
    // Vertex Buffer, Index Buffer
    Skybox_Vertex vertices[] =
    {
        Vector3(- 1.0f,  1.0f, -1.0f), 
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
    D3DBase::device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);
    vertexBufferStride = sizeof(Skybox_Vertex);
    vertexBufferOffset = 0;

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = {};
    ibData.pSysMem = indices;
    D3DBase::device->CreateBuffer(&ibDesc, &ibData, &indexBuffer);

    // Input Layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    ID3D10Blob* vsBlob = nullptr;
    D3DCompileFromFile(L"Skybox_VS.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
    D3DBase::device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);
    
    // Vertex Shader
    D3DBase::device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &skyboxVS);
    vsBlob->Release();

    // Pixel Shader
    ID3D10Blob* psBlob = nullptr;
    D3DCompileFromFile(L"Skybox_PS.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlob, nullptr);
    D3DBase::device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &skyboxPS);
    psBlob->Release();

    // CubeMap Texture Load
    CreateDDSTextureFromFile(D3DBase::device.Get(), L"../Resource/Skybox.dds", nullptr, &skyboxTRV);

    // Constant Buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(Skybox_ConstantBuffer);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    D3DBase::device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);

    // DepthStencilState (LESS_EQUAL)
    D3D11_DEPTH_STENCIL_DESC dssDesc = {};
    dssDesc.DepthEnable = TRUE;
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    D3DBase::device->CreateDepthStencilState(&dssDesc, &skyboxDSS);
}

void Skybox::Render(Matrix& view, Matrix& projection)
{
    // 카메라 위치 제거
    Matrix viewNoTranslation = view;
    viewNoTranslation._41 = 0.0f;
    viewNoTranslation._42 = 0.0f;
    viewNoTranslation._43 = 0.0f;

    // Constant buffer Update
    Skybox_ConstantBuffer cb;
    cb.world = XMMatrixIdentity();
    cb.view = XMMatrixTranspose(viewNoTranslation);
    cb.projection = XMMatrixTranspose(projection);
    D3DBase::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

    // 렌더 파이프라인 바인딩
    D3DBase::deviceContext->IASetVertexBuffers(1, 1, &vertexBuffer, &vertexBufferStride, &vertexBufferOffset);
    D3DBase::deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    D3DBase::deviceContext->IASetInputLayout(inputLayout);
    D3DBase::deviceContext->VSSetShader(skyboxVS, nullptr, 0);
    D3DBase::deviceContext->PSSetShader(skyboxPS, nullptr, 0);
    D3DBase::deviceContext->PSSetShaderResources(1, 1, &skyboxTRV);
    D3DBase::deviceContext->VSSetConstantBuffers(1, 1, &constantBuffer);
    D3DBase::deviceContext->PSSetConstantBuffers(1, 1, &constantBuffer);

    // DepthStencilState 적용
    D3DBase::deviceContext->OMSetDepthStencilState(skyboxDSS, 0);

    // Draw
    D3DBase::deviceContext->DrawIndexed(indexCount, 0, 0);

    // DepthFunc 원래대로 돌려주기 (일반 오브젝트를 위해)
    D3DBase::deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void Skybox::UninitRenderPipeLine()
{
    SAFE_RELEASE(vertexBuffer);
    SAFE_RELEASE(indexBuffer);
    SAFE_RELEASE(constantBuffer);
    SAFE_RELEASE(inputLayout);
    SAFE_RELEASE(skyboxVS);
    SAFE_RELEASE(skyboxPS);
    SAFE_RELEASE(skyboxTRV);
    SAFE_RELEASE(skyboxDSS);
}