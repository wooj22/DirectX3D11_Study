#include "SkeletalModel.h"
#include "D3D.h"
#include "Camera.h"
#include "Time.h"
#include "DirectionalLight.hpp"
#include "Structures.hpp"
using namespace DirectX;


SkeletalModel::SkeletalModel()
{
    SetTransform(Vector3::Zero, Vector3::Zero, Vector3::One);
}

SkeletalModel::SkeletalModel(Vector3 p, Vector3 r, Vector3 s)
{
    SetTransform(p, r, s);
}

void SkeletalModel::InitTransform()
{
    position = Vector3::Zero;
    rotation = Vector3::Zero;
    scale = Vector3::One;
    world = XMMatrixIdentity();
}

void SkeletalModel::SetTransform(Vector3 p, Vector3 r, Vector3 s)
{
    position = p;
    rotation = r;
    scale = s;
    MakeWorld();
}

void SkeletalModel::SetPosition(Vector3 p)
{
    position = p;
    MakeWorld();
}

void SkeletalModel::SetRotation(Vector3 r)
{
    rotation = r;
    MakeWorld();
}

void SkeletalModel::SetScale(Vector3 s)
{
    scale = s;
    MakeWorld();
}

void SkeletalModel::MakeWorld()
{
    Matrix tm = XMMatrixTranslationFromVector(position);
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    Matrix rm = XMMatrixRotationQuaternion(q);
    Matrix sm = XMMatrixScalingFromVector(scale);
    world = sm * rm * tm;
}

void SkeletalModel::Update()
{
    MakeWorld();

    // animation time update
    currentAnimTime += Time::GetDeltaTime();
    if (currentAnimTime > model->animationClips[0].duration)
        currentAnimTime = fmod(currentAnimTime, model->animationClips[0].duration);

    // bone local update
    for (auto& bone : model->skeleton.bones)
    {
        AnimationClip& clip = model->animationClips[0];
        for (auto& nodeAnim : clip.nodeAnimations)
        {
            if (nodeAnim.nodeName == bone.name)
            {
                Vector3 pos;  Quaternion rot;	Vector3 scl;
                nodeAnim.Interpolate(currentAnimTime, pos, rot, scl);

                bone.localMatrix = Matrix::CreateScale(scl) *
                    Matrix::CreateFromQuaternion(rot) *
                    Matrix::CreateTranslation(pos);
                break;
            }
            else
            {
                bone.localMatrix = bone.bindMatrix;
            }
        }
    }

    // bone world update
    model->skeleton.UpdateBoneWorld();
}

void SkeletalModel::Render()
{
    // model world
    D3D::transformCBData.world = world.Transpose();

    // bone world (animation)
    for (int j = 0; j < model->skeleton.bones.size(); j++)
    {
        D3D::poseCBData.bonePose[j] = model->skeleton.bones[j].worldMatrix.Transpose();
    }

    // bone offset
    for (int j = 0; j < model->skeleton.bones.size(); j++)
    {
        D3D::offsetCBData.boneOffset[j] = model->skeleton.bones[j].offsetMatrix.Transpose();
    }

    // mesh render
    for (int i = 0; i < model->subMeshes.size(); ++i)
    {
        SkeletalSubMesh& sub = model->subMeshes[i];
        Material& mat = model->materials[i];

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
        D3D::deviceContext->UpdateSubresource(D3D::offsetMatrixBuffer.Get(), 0, nullptr, &D3D::offsetCBData, 0, 0);
        D3D::deviceContext->UpdateSubresource(D3D::poseMatrixBuffer.Get(), 0, nullptr, &D3D::poseCBData, 0, 0);

        // draw call
        D3D::deviceContext->DrawIndexed(sub.indexCount, 0, 0);
    }
}