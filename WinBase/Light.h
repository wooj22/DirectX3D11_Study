#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

/*
    [ Light ]

    Directional, Point, Spot 라이트를 
    
*/

enum class LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2
};

class Light
{
public:
    LightType type;
    
    inline static bool hasSunLight;
    UINT isSunLight = 0;

    Vector3   color;
    float     intensity;

    Vector3   direction;        // directional, spot
    Vector3   position;         // point, spot
    
    float     range;            // point, spot
    float     innerAngle;       // spot
    float     outerAngle;       // spot

    Light(LightType type, bool isSun = false);
    void SetSunLight(bool isSun);
};