#pragma once
#include <DirectXMath.h>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Vertex
struct Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texcoord;

	Vertex() = default;

	Vertex(const Vector3& pos, const Vector3& n, const Vector3& t,
		const Vector3& b, const Vector2& uv)
		: position(pos), normal(n), tangent(t), bitangent(b), texcoord(uv)
	{
	}
};

// ConstantBuffer
struct alignas(16) ConstantBuffer
{
	Matrix world;
	Matrix view;
	Matrix projection;

	Vector4 lightDirection;
	Vector4 lightColor;

	float indirectLight;
	float directLight;

	float ambientHighlight;
	float diffuseHighlight;
	float specularHighlight;
	float shininess;
	Vector2 padding1;

	Vector3 cameraPos;

	UINT useDiffuse;
	UINT useNormal;
	UINT useSpecular;
	UINT useEmissive;
};