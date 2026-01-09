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


/*
    [ Emitter ]


*/

class Emitter
{
public:
    SpriteSheet      sheet;
    vector<Particle> particles;
    BillboardType    billboard = BillboardType::ScreenFacing;

    // position
    Vector3 position = Vector3::Zero;       // effect pos
    Vector3 localOffset = Vector3::Zero;

    // control
    bool enabled = true;
    bool playing = true;

    // timer
    float elapsed = 0.0f;       // Duration timer
    float emitAcc = 0.0f;       // Spawn timer

    // emission
    int   maxParticles = 256;
    float emitRate = 0.0f;
    int   burstCount = 0;
    float duration = 0.0f;
    bool  looping = true;

    // spawn range
    float   lifeMin = 0.5f;
    float   lifeMax = 1.0f;
    float   speedMin = 0.0f;
    float   speedMax = 10.0f;
    Vector2 sizeMin = Vector2::One;
    Vector2 sizeMax = Vector2::One;
    float   rotationMin = 0.0f;
    float   rotationMax = 0.0f;
    float   angularMin = 0.0f;
    float   angularMax = 0.0f;

    // color
    Vector4 startColor = { 1,1,1,1 };

public:
    void Update();
    void Spawn(int count);
};

float Rand01();
float RandRange(float a, float b);
Vector3 RandomUnitVector3();
