#pragma once

struct Material
{
	// 반사 계수
	float ambientHighlight = 0.1f;		// 환경광 반사 계수
	float diffuseHighlight = 0.8f;		// 난반사 반사 계수
	float specularHighlight = 1.0f;		// 정반사 반사 계수

	// 광택 계수
	float shininess = 300.0f;
};