#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// ºí¸°Æþ ½¦ÀÌµù µÉ Directional Light
struct DirectionalLight
{
	Vector4 direction = { 0, 0, 1, 0 };
	Vector4 color{ 1.0, 1.0f, 1.0, 1.0 };

	// ±¤¿ø ¼¼±â
	float indirectLight = 0.3f;		// °£Á¢±¤ ¼¼±â -> È¯°æ±¤
	float directLight = 1.0f;		// Á÷Á¢±¤ ¼¼±â -> ³­¹Ý»ç±¤, Á¤¹Ý»ç±¤
};