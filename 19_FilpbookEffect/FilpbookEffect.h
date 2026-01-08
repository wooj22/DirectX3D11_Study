#pragma once
#include "SpriteSheet.hpp"
#include "Particle.hpp"
#include <vector>
using std::vector;
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;


// Billboard Type
enum class BillboardType
{
    ScreenFacing,       // Quad가 항상 정면
    YAxis,              // Y축 고정
};

// FilpbookEffect
class FilpbookEffect
{
public:
    BillboardType billboard = BillboardType::ScreenFacing;

    SpriteSheet sheet;
    Particle    particle;           // Filpbook

    bool  loop = true;
    bool  alive = true;
    int   frame = 0;

    void Play(Vector3 pos);
    void Update();
};