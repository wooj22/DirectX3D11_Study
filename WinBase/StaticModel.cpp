#include "StaticModel.h"
#include "D3D.h"
#include "Camera.h"
#include "DirectionalLight.hpp"
#include "Structures.hpp"

StaticModel::StaticModel()
{
    SetTransform(Vector3::Zero, Vector3::Zero, Vector3::One);
}

StaticModel::StaticModel(Vector3 p, Vector3 r, Vector3 s)
{
    SetTransform(p, r, s);
}

void StaticModel::InitTransform()
{
    position = Vector3::Zero;
    rotation = Vector3::Zero;
    scale = Vector3::One;
    world = XMMatrixIdentity();
}

void StaticModel::SetTransform(Vector3 p, Vector3 r, Vector3 s)
{
    position = p;
    rotation = r;
    scale = s;
    MakeWorld();
}

void StaticModel::SetPosition(Vector3 p)
{
    position = p;
    MakeWorld();
}

void StaticModel::SetRotation(Vector3 r)
{
    rotation = r;
    MakeWorld();
}

void StaticModel::SetScale(Vector3 s)
{
    scale = s;
    MakeWorld();
}

void StaticModel::MakeWorld()
{
    Matrix tm = XMMatrixTranslationFromVector(position);
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    Matrix rm = XMMatrixRotationQuaternion(q);
    Matrix sm = XMMatrixScalingFromVector(scale);
    world = sm * rm * tm;
}

void StaticModel::Update()
{
    //MakeWorld();
}

void StaticModel::Render()
{
    // world matrix
    D3D::transformCBData.model = Matrix::Identity.Transpose();
    D3D::transformCBData.world = XMMatrixTranspose(world);

    // sub mesh render
    for (int i = 0; i < subMeshes.size(); ++i)
    {
        StaticSubMesh& sub = subMeshes[i];
        Material& mat = materials[i];

        // vertex buffer, indexbuffer
        D3D::deviceContext->IASetVertexBuffers(0, 1, &sub.vertexBuffer, &sub.vertexBufferStride, &sub.vertexBufferOffset);
        D3D::deviceContext->IASetIndexBuffer(sub.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // texture
        D3D::deviceContext->PSSetShaderResources(0, 1, &mat.diffuseSRV);
        D3D::deviceContext->PSSetShaderResources(1, 1, &mat.normalSRV);
        D3D::deviceContext->PSSetShaderResources(2, 1, &mat.specualrSRV);
        D3D::deviceContext->PSSetShaderResources(3, 1, &mat.emissiveSRV);
        D3D::materialCBData.useDiffuse = (materials[i].textureFlags & TEX_DIFFUSE) != 0;
        D3D::materialCBData.useNormal = (materials[i].textureFlags & TEX_NORMAL) != 0;
        D3D::materialCBData.useSpecular = (materials[i].textureFlags & TEX_SPECULAR) != 0;
        D3D::materialCBData.useEmissive = (materials[i].textureFlags & TEX_EMISSIVE) != 0;

        // constant buffer
        D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0); // constant buffe°¡ ¿¹¿Ü
        D3D::deviceContext->UpdateSubresource(D3D::materialBuffer.Get(), 0, nullptr, &D3D::materialCBData, 0, 0);

        // draw call
        D3D::deviceContext->DrawIndexed(sub.indexCount, 0, 0);
    }
}