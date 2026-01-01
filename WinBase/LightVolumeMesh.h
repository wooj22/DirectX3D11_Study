#pragma once
#define NOMINMAX
#include "Structures.hpp"
#include "D3D.h"
#include "Light.hpp"
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
    int type;   // 0 : sphere, 1 : cone (юс╫ц)
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
    UINT indexCount = 0;
    UINT stride = sizeof(Position_Vertex);
    DXGI_FORMAT indexFormat = DXGI_FORMAT_R32_UINT;

    Matrix world = Matrix::Identity;

public:
    void UpdateWolrd(Light& light)
    {
        if (type == 0)
        {
            Matrix S = Matrix::CreateScale(light.range);
            Matrix T = Matrix::CreateTranslation(light.position);
            world = S * T;
        }
        else if (type == 1)
        {
            float height = light.range;
            float outerRad = DirectX::XMConvertToRadians(light.outerAngle);
            float radius = height * tanf(outerRad);
            Matrix S = Matrix::CreateScale(radius, radius, height);

            Vector3 coneForward(0, 0, 1);

            Vector3 dir = light.direction;
            dir.Normalize();

            Vector3 axis = coneForward.Cross(dir);
            float angle = acosf(coneForward.Dot(dir));

            Matrix R = axis.LengthSquared() < 0.0001f
                ? Matrix::Identity
                : Matrix::CreateFromAxisAngle(axis, angle);

            Matrix T = Matrix::CreateTranslation(light.position);

            world = S * R * T;
        }
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