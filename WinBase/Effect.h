#pragma once
#include "SpriteSheet.hpp"
#include "Particle.hpp"
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;


// Billboard Type
enum class BillboardType
{
    ScreenFacing,       // Quad가 항상 정면
    YAxis,              // Y축 고정
};

/*
    [ Effect ]

*/

class Effect  // 현재는 Filpbook 기준
{
public:
    BillboardType billboard = BillboardType::ScreenFacing;

    SpriteSheet sheet;
    Particle    particle;

    bool  loop = true;
    bool  alive = true;
    int   frame = 0;

    void Play(Vector3 pos);
    void Update();
};