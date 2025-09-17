#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

struct DirectionalLight
{
	// ���� direction�� light�� ����������, ���� ������ ���� ǥ��->light�� ������ ���
	Vector4 direction = { 0.5f, 0.8f, -0.8, 1.0f };
	Vector4 color{ 1.0, 1.0f, 1.0, 1.0 };
};