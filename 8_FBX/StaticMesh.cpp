#include "StaticMesh.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Camera.h"
#include "DirectionalLight.hpp"

StaticMesh::StaticMesh()
{
	SetTransform(Vector3::Zero, Vector3::Zero, Vector3::One);
}

StaticMesh::StaticMesh(Vector3 p, Vector3 r, Vector3 s)
{
	SetTransform(p, r, s);
}

void StaticMesh::InitTransform()
{
	position = Vector3::Zero;
	rotation = Vector3::Zero;
	scale = Vector3::One;
	world = XMMatrixIdentity();
}

void StaticMesh::SetTransform(Vector3 p, Vector3 r, Vector3 s)
{
	position = p;
	rotation = r;
	scale = s;
	MakeWorld();
}

void StaticMesh::SetPosition(Vector3 p)
{
	position = p;
	MakeWorld();
}

void StaticMesh::SetRotation(Vector3 r) 
{
	rotation = r;
	MakeWorld();
}

void StaticMesh::SetScale(Vector3 s)
{
	scale = s;
	MakeWorld();
}

void StaticMesh::MakeWorld() 
{
	Matrix tm = XMMatrixTranslationFromVector(position);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	Matrix rm = XMMatrixRotationQuaternion(q);
	Matrix sm = XMMatrixScalingFromVector(scale);
	world = sm * rm * tm;
}

void StaticMesh::Update()
{
	MakeWorld();
}

void StaticMesh::Render(ID3D11Buffer* constantBuffer, ConstantBuffer& cb)
{
	// world matrix
	cb.world = XMMatrixTranspose(world);

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