#pragma once

struct Material
{
	// 반사 계수
	float ambientReflection = 0.1f;		// 환경광 반사 계수
	float diffuseReflection = 0.8f;		// 난반사 계수(텍스처에서 사용)
	float specularReflection = 0.3f;	// 정반사 계수

	// 광택 계수
	float shininess = 32.0f;
};