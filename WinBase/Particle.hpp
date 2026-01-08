#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Particle(Quad) 하나의 상태 Instance
// Effect의 Resource로 사용됩니다. -> CB로 넘길 Data
struct Particle
{
    Vector3 pos      = Vector3::Zero;
    float   rotation = 0;
    Vector2 size     = Vector2::One;
    Vector4 color    = { 1,1,1,1 };

    float  age;
    float  life;
    bool  alive = true;
    int   frame = 0;
};