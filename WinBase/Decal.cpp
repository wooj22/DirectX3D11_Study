#include "Decal.h"

void Decal::StartRingEffect(float startTime, float duration, float speed)
{
    if (type == DecalType::TextureMap) return;

    ringStartTime = startTime;
    ringDuration = duration;
    ringSpeed = speed;
}