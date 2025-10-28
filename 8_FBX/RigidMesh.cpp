#include "RigidMesh.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Camera.h"
#include "../WinBase/Time.h"
#include "DirectionalLight.hpp"
using namespace DirectX;

RigidMesh::RigidMesh()
{
	SetTransform(Vector3::Zero, Vector3::Zero, Vector3::One);
}

RigidMesh::RigidMesh(Vector3 p, Vector3 r, Vector3 s)
{
	SetTransform(p, r, s);
}

void RigidMesh::InitTransform()
{
	position = Vector3::Zero;
	rotation = Vector3::Zero;
	scale = Vector3::One;
	world = XMMatrixIdentity();
}

void RigidMesh::SetTransform(Vector3 p, Vector3 r, Vector3 s)
{
	position = p;
	rotation = r;
	scale = s;
	MakeWorld();
}

void RigidMesh::SetPosition(Vector3 p)
{
	position = p;
	MakeWorld();
}

void RigidMesh::SetRotation(Vector3 r)
{
	rotation = r;
	MakeWorld();
}

void RigidMesh::SetScale(Vector3 s)
{
	scale = s;
	MakeWorld();
}

void RigidMesh::MakeWorld()
{
	Matrix tm = XMMatrixTranslationFromVector(position);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	Matrix rm = XMMatrixRotationQuaternion(q);
	Matrix sm = XMMatrixScalingFromVector(scale);
	world = sm * rm * tm;
}

void RigidMesh::Update()
{
	MakeWorld();

	// animation time update
	currentAnimTime += Time::GetDeltaTime();
	if (currentAnimTime > animationClips[0].duration)
		currentAnimTime = fmod(currentAnimTime, animationClips[0].duration);

	// local matrix update
	// animaton key frame값을 보간해서 local matrix 업데이트
	for (auto& sub : subMeshes)
	{
		AnimationClip& clip = animationClips[0];		// 일단 고정
		for (auto& nodeAnim : clip.nodeAnimations)
		{
			if (nodeAnim.nodeName == sub.nodeName)
			{
				Vector3 pos;  Quaternion rot;	Vector3 scl;
				nodeAnim.Interpolate(currentAnimTime, pos, rot, scl);

				/*sub.localMatrix = Matrix::CreateScale(scl) *
									Matrix::CreateFromQuaternion(rot) *
									Matrix::CreateTranslation(pos);*/

				sub.localMatrix = sub.bindMatrix;
				break;
			}
		}
	}

	// model matrix update
	for (auto& sub : subMeshes)
	{
		OutputDebugStringA((sub.nodeName + "\n").c_str());
		OutputDebugStringA(("Parent Index: " + std::to_string(sub.parentIndex) + "\n").c_str());
		if (sub.parentIndex != -1)
			OutputDebugStringA(("Parent Name: " + subMeshes[sub.parentIndex].nodeName + "\n").c_str());
		else
			OutputDebugStringA("No Parent\n");

		if (sub.parentIndex != -1)
			sub.modelMatrix = subMeshes[sub.parentIndex].modelMatrix * sub.localMatrix;
		else
			sub.modelMatrix = sub.localMatrix;
	}
}

void RigidMesh::Render(ID3D11Buffer* constantBuffer, ConstantBuffer& cb)
{
	for (int i = 0; i < subMeshes.size(); ++i)
	{
		RigidSubMesh& sub = subMeshes[i];
		Material& mat = materials[i];

		// world
		cb.world = (sub.modelMatrix * world).Transpose();

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