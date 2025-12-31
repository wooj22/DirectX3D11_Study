#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// TODO :: Delete
// 지금은 Light 구조체로 Direction/ Point/ Spot 라이트를 모두 처리하고 있음

// Directional Light
struct DirectionalLight
{
    Vector3 direction = { 0, -1, 0};
    Vector3 color{ 1.0, 1.0f, 1.0};

    // Intensity
    float directIntensity = 1.0f;
    float indirectIntensity = 0.3f;		 		
};