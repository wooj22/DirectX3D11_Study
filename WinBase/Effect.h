#pragma once
#include "SpriteSheet.hpp"
#include "Particle.hpp"
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;


class Effect  // 현재는 Filpbook 기준
{
public:
    SpriteSheet sheet;
    Particle particle;
    bool loop;
    bool alive;

    void Play(Vector3 pos);
    void Update(float dt);
};