#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Particle(Quad) 하나의 상태 Instance
// Effect의 Resource로 사용됩니다. -> CB로 넘길 Data
struct Particle
{
    Vector3 pos;
    float   rotation;
    Vector2 size;
    Vector4 color;

    float  age;
    float  life;
};