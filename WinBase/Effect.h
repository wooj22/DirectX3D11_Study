#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;


struct SpriteSheet;
struct Particle;
class ParticleRenderer;

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