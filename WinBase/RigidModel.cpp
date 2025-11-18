#include "RigidModel.h"
#include "D3D.h"
#include "Camera.h"
#include "Time.h"
#include "DirectionalLight.hpp"
#include "Structures.hpp"
using namespace DirectX;

RigidModel::RigidModel()
{
    SetTransform(Vector3::Zero, Vector3::Zero, Vector3::One);
}

RigidModel::RigidModel(Vector3 p, Vector3 r, Vector3 s)
{
    SetTransform(p, r, s);
}

void RigidModel::InitTransform()
{
    position = Vector3::Zero;
    rotation = Vector3::Zero;
    scale = Vector3::One;
    world = XMMatrixIdentity();
}

void RigidModel::SetTransform(Vector3 p, Vector3 r, Vector3 s)
{
    position = p;
    rotation = r;
    scale = s;
    MakeWorld();
}

void RigidModel::SetPosition(Vector3 p)
{
    position = p;
    MakeWorld();
}

void RigidModel::SetRotation(Vector3 r)
{
    rotation = r;
    MakeWorld();
}

void RigidModel::SetScale(Vector3 s)
{
    scale = s;
    MakeWorld();
}

void RigidModel::MakeWorld()
{
    Matrix tm = XMMatrixTranslationFromVector(position);
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    Matrix rm = XMMatrixRotationQuaternion(q);
    Matrix sm = XMMatrixScalingFromVector(scale);
    world = sm * rm * tm;
}

void RigidModel::Update()
{
    //MakeWorld();

    // animation time update
    currentAnimTime += Time::GetDeltaTime();
    if (currentAnimTime > model->animationClips[0].duration)
        currentAnimTime = fmod(currentAnimTime, model->animationClips[0].duration);

    // local matrix update
    // animaton key frame값을 보간해서 local matrix 업데이트
    // TODO :: 해시테이블로 바꾸기
    for(int i=0; i<model->subMeshes.size(); i++)
    {
        auto& sub = model->subMeshes[i];

        AnimationClip& clip = model->animationClips[0];
        for (auto& nodeAnim : clip.nodeAnimations)
        {
            if (nodeAnim.nodeName == sub.nodeName)
            {
                Vector3 pos;  Quaternion rot;	Vector3 scl;
                nodeAnim.Interpolate(currentAnimTime, pos, rot, scl);

                localMatrix[i] = Matrix::CreateScale(scl) *
                    Matrix::CreateFromQuaternion(rot) *
                    Matrix::CreateTranslation(pos);
                break;
            }
            else
            {
                localMatrix[i] = sub.bindMatrix;
            }
        }
    }

    // model matrix update
    for (int i = 0; i < model->subMeshes.size(); i++)
    {
        auto& sub = model->subMeshes[i];

        if (sub.parentIndex != -1)
            modelMatrix[i] = localMatrix[i] * modelMatrix[sub.parentIndex];
        else
            modelMatrix[i] = localMatrix[i];
    }
}

void RigidModel::Render()
{
    // world
    D3D::transformCBData.world = world.Transpose();

    for (int i = 0; i < model->subMeshes.size(); ++i)
    {
        RigidSubMesh& sub = model->subMeshes[i];
        Material& mat = model->materials[i];

        // model
        D3D::transformCBData.model = modelMatrix[i].Transpose();

        // vertex buffer, indexbuffer
        D3D::deviceContext->IASetVertexBuffers(0, 1, &sub.vertexBuffer, &sub.vertexBufferStride, &sub.vertexBufferOffset);
        D3D::deviceContext->IASetIndexBuffer(sub.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

        // texture
        D3D::deviceContext->PSSetShaderResources(0, 1, &mat.diffuseSRV);
        D3D::deviceContext->PSSetShaderResources(1, 1, &mat.normalSRV);
        D3D::deviceContext->PSSetShaderResources(2, 1, &mat.specualrSRV);
        D3D::deviceContext->PSSetShaderResources(3, 1, &mat.emissiveSRV);
        D3D::materialCBData.useDiffuse = (model->materials[i].textureFlags & TEX_DIFFUSE) != 0;
        D3D::materialCBData.useNormal = (model->materials[i].textureFlags & TEX_NORMAL) != 0;
        D3D::materialCBData.useSpecular = (model->materials[i].textureFlags & TEX_SPECULAR) != 0;
        D3D::materialCBData.useEmissive = (model->materials[i].textureFlags & TEX_EMISSIVE) != 0;

        // constant buffer
        D3D::deviceContext->UpdateSubresource(D3D::transformBuffer.Get(), 0, nullptr, &D3D::transformCBData, 0, 0);
        D3D::deviceContext->UpdateSubresource(D3D::materialBuffer.Get(), 0, nullptr, &D3D::materialCBData, 0, 0);

        // draw call
        D3D::deviceContext->DrawIndexed(sub.indexCount, 0, 0);
    }
}