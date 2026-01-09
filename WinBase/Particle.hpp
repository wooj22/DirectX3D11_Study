#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Particle(Quad) 하나의 상태 Instance
// Effect의 Resource로 사용됩니다. -> CB로 넘길 Data
struct Particle
{
    Vector3 pos = Vector3::Zero;
    Vector3 vel = Vector3::Zero;

    float rotation   = 0.0f;
    float angularVel = 0.0f;

    Vector2 size  = Vector2::One;
    Vector4 color = { 1,1,1,1 };

    float age  = 0.0f;
    float life = 1.0f;

    int  frame = 0;
    bool alive = true;
};