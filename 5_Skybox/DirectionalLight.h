#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

struct DirectionalLight
{
	// 원래 direction은 light의 방향이지만, 연산 생략을 위해 표면->light의 방향을 써둠
	Vector4 direction = { 0.5f, 0.8f, -0.8, 1.0f };
	Vector4 color{ 1.0, 1.0f, 1.0, 1.0 };
};