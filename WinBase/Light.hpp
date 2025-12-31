#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

enum class LightType
{
    Directional = 0,
    Point = 1,
    Spot = 2
};

struct Light
{
    LightType type;

    Vector3   color;
    float     intensity;

    Vector3   direction;        // directional, spot
    Vector3   position;         // point, spot
    
    float     range;            // point, spot
    float     innerAngle;       // spot
    float     outerAngle;       // spot

    Light(LightType type) : type(type),
        color(1.0f, 1.0f, 1.0f),
        intensity(1.0f),
        direction(0.0f, -1.0f, 0.0f),
        position(0.0f, 0.0f, 0.0f),
        range(10.0f),
        innerAngle(0.0f),
        outerAngle(0.0f)
    {
        switch (type)
        {
        case LightType::Directional:
            intensity = 1.0f;
            direction = Vector3(0.0f, -1.0f, 0.0f);
            range = 0.0f;
            break;

        case LightType::Point:
            intensity = 1.0f;
            position = Vector3(0.0f, 0.0f, 0.0f);
            range = 10.0f;
            break;

        case LightType::Spot:
            intensity = 1.0f;
            position = Vector3(0.0f, 0.0f, 0.0f);
            direction = Vector3(0.0f, -1.0f, 0.0f);
            range = 15.0f;
            innerAngle = 15.0f;
            outerAngle = 30.0f;
            break;
        }
    }
};