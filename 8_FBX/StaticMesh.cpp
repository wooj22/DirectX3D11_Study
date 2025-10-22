#include "StaticMesh.h"
#include "../WinBase/D3D.h"
#include "../WinBase/Camera.h"
#include "DirectionalLight.hpp"

StaticMesh::StaticMesh()
{
	position = Vector3::Zero;
	rotation = Vector3::Zero;
	scale = Vector3::One;
	world = XMMatrixIdentity();
}

StaticMesh::StaticMesh(Vector3 p, Vector3 r, Vector3 s)
{
	position = p;
	rotation = r;
	scale = s;

	Matrix tm = XMMatrixTranslationFromVector(p);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(r.x, r.y, r.z);
	Matrix rm = XMMatrixRotationQuaternion(q);
	Matrix sm = XMMatrixScalingFromVector(s);
	world = sm * rm * tm;
}

StaticMesh::~StaticMesh()
{
	
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

	Matrix tm = XMMatrixTranslationFromVector(p);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(r.x, r.y, r.z);
	Matrix rm = XMMatrixRotationQuaternion(q);
	Matrix sm = XMMatrixScalingFromVector(s);
	world = sm * rm * tm;
}

void StaticMesh::Update()
{
	// world update
	Matrix t1 = XMMatrixTranslationFromVector(position);
	XMVECTOR q = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	Matrix r1 = XMMatrixRotationQuaternion(q);
	Matrix s1 = XMMatrixScalingFromVector(scale);
	world = s1 * r1 * t1;
}

void StaticMesh::Render(ID3D11Buffer* constantBuffer, ConstantBuffer& cb)
{
	// vertex, index
	D3D::deviceContext->IASetVertexBuffers(0, 1, &subMeshes[0].vertexBuffer, 
		&subMeshes[0].vertexBufferStride, &subMeshes[0].vertexBufferOffset);
	D3D::deviceContext->IASetIndexBuffer(subMeshes[0].indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// constant buffer
	cb.world = world;
	D3D::deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

	// srv
	D3D::deviceContext->PSSetShaderResources(0, 1, &materials[0].diffuseSRV);
	D3D::deviceContext->PSSetShaderResources(1, 1, &materials[0].normalSRV);
	D3D::deviceContext->PSSetShaderResources(2, 1, &materials[0].specualrSRV);

	// draw
	D3D::deviceContext->DrawIndexed(subMeshes[0].indexCount, 0, 0);
}