#include "Light.h"

Light::Light(LightType type, bool isSun)
    : type(type),
    color(1.0f, 1.0f, 1.0f),
    intensity(1.0f),
    direction(0.0f, -1.0f, 0.0f),
    position(0.0f, 0.0f, 0.0f),
    range(10.0f),
    innerAngle(0.0f),
    outerAngle(0.0f),
    isSunLight(0)
{
    switch (type)
    {
    case LightType::Directional:
        if (isSun && !hasSunLight)   // sunlight는 하나로 제한
        {
            hasSunLight = true;
            isSunLight = true;
        }
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

void Light::SetSunLight(bool isSun)
{
    if (type == LightType::Directional)
    {
        if (isSun && !hasSunLight)
        {
            hasSunLight = true;
            isSunLight = 1;
        }
        else
        {
            isSunLight = 0;
        }
    }
}