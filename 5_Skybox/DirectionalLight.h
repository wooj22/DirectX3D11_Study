#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

struct DirectionalLight
{
	Vector4 direction = { -1.0f, -1.0f, 1.0f, 0 };
	Vector4 color{ 1.0, 1.0f, 1.0, 1.0 };
};