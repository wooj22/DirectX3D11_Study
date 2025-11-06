#include "SkeletalMesh.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Camera.h"
#include "../WinBase/Time.h"
#include "DirectionalLight.hpp"
using namespace DirectX;


SkeletalMesh::SkeletalMesh()
{
	SetTransform(Vector3::Zero, Vector3::Zero, Vector3::One);
}

SkeletalMesh::SkeletalMesh(Vector3 p, Vector3 r, Vector3 s)
{
	SetTransform(p, r, s);
}

void SkeletalMesh::InitTransform()
{
	position = Vector3::Zero;
	rotation = Vector3::Zero;
	scale = Vector3::One;
	world = XMMatrixIdentity();
}

void SkeletalMesh::SetTransform(Vector3 p, Vector3 r, Vector3 s)
{
	position = p;
	rotation = r;
	scale = s;
	MakeWorld();
}

void SkeletalMesh::SetPosition(Vector3 p)
{
	position = p;
	MakeWorld();
}

void SkeletalMesh::SetRotation(Vector3 r)
{
	rotation = r;
	MakeWorld();
}

void SkeletalMesh::SetScale(Vector3 s)
{
	scale = s;
	MakeWorld();
}

void SkeletalMesh::MakeWorld()
{
	Matrix tm = XMMatrixTranslationFromVector(position);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	Matrix rm = XMMatrixRotationQuaternion(q);
	Matrix sm = XMMatrixScalingFromVector(scale);
	world = sm * rm * tm;
}

void SkeletalMesh::Update()
{
	//MakeWorld();

	// animation time update
	currentAnimTime += Time::GetDeltaTime();
	if (currentAnimTime > animationClips[0].duration)
		currentAnimTime = fmod(currentAnimTime, animationClips[0].duration);

	// bone local update
	for (auto& bone : skeleton.bones)
	{
		AnimationClip& clip = animationClips[0];
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
    skeleton.UpdateBoneWorld();
}

void SkeletalMesh::Render(ID3D11Buffer* constantBuffer, ID3D11Buffer* offsetMatrixCB, ID3D11Buffer* poseMatrixCB,
    ConstantBuffer& cb, OffsetMatrixCB& offsetCB, PoseMatrixCB& poseCB)
{
    // model world
    cb.world = world.Transpose();

    // bone world (animation)
    for (int j = 0; j < skeleton.boneCount; j++)
    {
        poseCB.bonePose[j] = skeleton.bones[j].worldMatrix.Transpose();
    }

    // bone offset
    for (int j = 0; j < skeleton.boneCount; j++)
    {
        offsetCB.boneOffset[j] = skeleton.bones[j].offsetMatrix.Transpose();
    }

    // mesg rebder
	for (int i = 0; i < subMeshes.size(); ++i)
	{
		SkeletalSubMesh& sub = subMeshes[i];
		Material& mat = materials[i];

		// vertex buffer, indexbuffer
		D3D::deviceContext->IASetVertexBuffers(0, 1, &sub.vertexBuffer, &sub.vertexBufferStride, &sub.vertexBufferOffset);
		D3D::deviceContext->IASetIndexBuffer(sub.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

		// texture
		D3D::deviceContext->PSSetShaderResources(0, 1, &mat.diffuseSRV);
		D3D::deviceContext->PSSetShaderResources(1, 1, &mat.normalSRV);
		D3D::deviceContext->PSSetShaderResources(2, 1, &mat.specualrSRV);
		D3D::deviceContext->PSSetShaderResources(3, 1, &mat.emissiveSRV);
		cb.useDiffuse = (materials[i].textureFlags & TEX_DIFFUSE) != 0;
		cb.useNormal = (materials[i].textureFlags & TEX_NORMAL) != 0;
		cb.useSpecular = (materials[i].textureFlags & TEX_SPECULAR) != 0;
		cb.useEmissive = (materials[i].textureFlags & TEX_EMISSIVE) != 0;

		// constant buffer
		D3D::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
        D3D::deviceContext->UpdateSubresource(offsetMatrixCB, 0, nullptr, &offsetCB, 0, 0);
        D3D::deviceContext->UpdateSubresource(poseMatrixCB, 0, nullptr, &poseCB, 0, 0);

		// draw call
		D3D::deviceContext->DrawIndexed(sub.indexCount, 0, 0);
	}
}