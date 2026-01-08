#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

// Filpbook Àü¿ë particle
struct Particle
{
    Vector3 pos = Vector3::Zero;
    float   rotation = 0;
    Vector2 size = Vector2::One;
    Vector4 color = { 1,1,1,1 };

    float  age;
    float  life;
};