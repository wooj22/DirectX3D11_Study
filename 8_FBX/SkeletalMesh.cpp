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
	currentAnimTime += Time::GetDeltaTime() * 0.5;
	if (currentAnimTime > animationClips[0].duration)
		currentAnimTime = fmod(currentAnimTime, animationClips[0].duration);

	// local matrix update
	// animaton key frame값을 보간해서 local matrix 업데이트
	// TODO :: 해시테이블로 바꾸기
	for (auto& sub : subMeshes)
	{
		AnimationClip& clip = animationClips[0];
		for (auto& nodeAnim : clip.nodeAnimations)
		{
			if (nodeAnim.nodeName == sub.nodeName)
			{
				Vector3 pos;  Quaternion rot;	Vector3 scl;
				nodeAnim.Interpolate(currentAnimTime, pos, rot, scl);

				sub.localMatrix = Matrix::CreateScale(scl) *
					Matrix::CreateFromQuaternion(rot) *
					Matrix::CreateTranslation(pos);
				break;
			}
			else
			{
				sub.localMatrix = sub.bindMatrix;
			}
		}
	}

	// model matrix update
	for (auto& sub : subMeshes)
	{
		if (sub.parentIndex != -1)
			sub.modelMatrix = sub.localMatrix * subMeshes[sub.parentIndex].modelMatrix;
		else
			sub.modelMatrix = sub.localMatrix;
	}
}

void SkeletalMesh::Render(ID3D11Buffer* constantBuffer, ConstantBuffer& cb)
{
    // world matrix
    cb.skeletal_world = world.Transpose();

	for (int i = 0; i < subMeshes.size(); ++i)
	{
		SkeletalSubMesh& sub = subMeshes[i];
		Material& mat = materials[i];

        cb.world = (sub.modelMatrix * world).Transpose();

		// model matrix
		cb.skeletal_model = sub.modelMatrix.Transpose();

        // offset matrix
        // TODO :: 오류. 이걸 하면 이상하게 input layout 교체가 안됨
        /*cb.boneCount = sub.boneCount;
        for (int j = 0; j < sub.boneCount; j++)
        {
            cb.boneOffset[j] = sub.bones[i].offsetMatrix.Transpose();
        }*/

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

		// draw call
		D3D::deviceContext->DrawIndexed(sub.indexCount, 0, 0);
	}
}