#pragma once
#define NOMINMAX
#include "Structures.hpp"
#include "D3D.h"
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>


using namespace DirectX;
using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;


class LightVolumeMesh
{
public:
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    UINT indexCount = 0;
    UINT stride = sizeof(Position_Vertex);
    DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT;

    Matrix world = Matrix::Identity;

public:
    void UpdateWolrd(const Vector3& lightPos, float lightRange)
    {
        // world update
        Matrix S = Matrix::CreateScale(lightRange);
        Matrix T = Matrix::CreateTranslation(lightPos);
        world = S * T;
    }

    void Draw() const
    {
        // cb upate
        D3D::transformCBData.world = XMMatrixTranspose(world);
        D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);

        // pipeline set
        UINT offset = 0;
        ID3D11Buffer* vbRaw = vertexBuffer.Get();
        D3D::deviceContext.Get()->IASetVertexBuffers(0, 1, &vbRaw, &stride, &offset);
        D3D::deviceContext.Get()->IASetIndexBuffer(indexBuffer.Get(), indexFormat, 0);
        D3D::deviceContext.Get()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D::deviceContext.Get()->IASetInputLayout(D3D::inputLayout_Position.Get());
        D3D::deviceContext.Get()->VSSetShader(D3D::VS_LightVolume.Get(), nullptr, 0);

        // draw call
        D3D::deviceContext.Get()->DrawIndexed(indexCount, 0, 0);
    }
};


LightVolumeMesh CreateLightVolumeSphere(ID3D11Device* device, int slices = 24, int stacks = 16);
LightVolumeMesh CreateLightVolumeCone(ID3D11Device* device, int slices = 24, bool capBase = false);