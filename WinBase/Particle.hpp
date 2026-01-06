#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

struct Particle
{
    Vector3 pos;
    Vector2 size;
    float  rotation;
    Vector4 color;

    float  age;
    float  life;
};