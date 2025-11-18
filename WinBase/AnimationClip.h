#pragma once
#include <iostream>
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// vector3 keyframe
struct VectorKey
{
    float time;
    Vector3 value;
};

// quaternion keyframe
struct QuatKey
{
    float time;
    Quaternion value;
};

// node animation
struct NodeAnimation
{
    std::string nodeName;
    std::vector<VectorKey> positionKeys;
    std::vector<QuatKey> rotationKeys;
    std::vector<VectorKey> scaleKeys;

    // 키 프레임 보간
    void Interpolate(float time, Vector3& outPosition, Quaternion& outRotation, Vector3& outScale)
    {
        // position
        if (!positionKeys.empty())
        {
            if (positionKeys.size() == 1) outPosition = positionKeys[0].value;
            else
            {
                const VectorKey* prev = &positionKeys[0];
                const VectorKey* next = nullptr;

                for (size_t i = 1; i < positionKeys.size(); ++i)
                {
                    if (time < positionKeys[i].time)
                    {
                        next = &positionKeys[i];
                        break;
                    }

                    prev = &positionKeys[i];
                }

                if (!next) next = &positionKeys.back();

                float t = (next->time - prev->time) > 0 ? (time - prev->time) / (next->time - prev->time) : 0.f;
                outPosition = Vector3::Lerp(prev->value, next->value, t);
            }
        }
        else
            outPosition = Vector3::Zero;

        // rotation
        if (!rotationKeys.empty())
        {
            if (rotationKeys.size() == 1) outRotation = rotationKeys[0].value;
            else
            {
                const QuatKey* prev = &rotationKeys[0];
                const QuatKey* next = nullptr;

                for (size_t i = 1; i < rotationKeys.size(); ++i)
                {
                    if (time < rotationKeys[i].time)
                    {
                        next = &rotationKeys[i];
                        break;
                    }
                    prev = &rotationKeys[i];
                }

                if (!next) next = &rotationKeys.back();

                float t = (next->time - prev->time) > 0 ? (time - prev->time) / (next->time - prev->time) : 0.f;
                outRotation = Quaternion::Slerp(prev->value, next->value, t);
            }
        }
        else
            outRotation = Quaternion::Identity;

        // scale
        if (!scaleKeys.empty())
        {
            if (scaleKeys.size() == 1)
            {
                outScale = scaleKeys[0].value;
            }
            else
            {
                const VectorKey* prev = &scaleKeys[0];
                const VectorKey* next = nullptr;

                for (size_t i = 1; i < scaleKeys.size(); ++i)
                {
                    if (time < scaleKeys[i].time)
                    {
                        next = &scaleKeys[i];
                        break;
                    }
                    prev = &scaleKeys[i];
                }

                if (!next) next = &scaleKeys.back();

                float t = (next->time - prev->time) > 0 ? (time - prev->time) / (next->time - prev->time) : 0.f;
                outScale = Vector3::Lerp(prev->value, next->value, t);
            }
        }
        else
            outScale = Vector3::One;

    }
};

// animation clip
struct AnimationClip
{
    std::string  name;					        // 애니메이션 이름
    float        duration;                      // 애니메이션 길이 (초)
    float        ticksPerSecond;                // 초당 프레임 수
    std::vector<NodeAnimation> nodeAnimations;  // 노드 애니메이션들
};

