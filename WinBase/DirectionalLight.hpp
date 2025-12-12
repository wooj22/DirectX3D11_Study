#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Directional Light
struct DirectionalLight
{
    Vector4 direction = { 0, -1, 0, 0 };
    Vector4 color{ 1.0, 1.0f, 1.0, 1.0 };

    // Intensity
    float directIntensity = 1.0f;
    float indirectIntensity = 0.3f;		 		
};