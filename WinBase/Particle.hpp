#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

/*
    [ Particle ]

    GPU에 Instance Buffer로 전달될 파티클의 상태 데이터
*/


struct Particle
{
    // instance data
    Vector3   pos        = Vector3::Zero;
    float     rotation   = 0.0f;
    Vector2   size       = Vector2::One;
    Vector4   color      = { 1,1,1,1 };
    int       frame      = 0;

    // particle system (spawn - moving)
    Vector3  vel        = Vector3::Zero;   // position
    float    angularVel = 0.0f;            // rotation

    // life
    bool  alive = true;     // 활성 여부
    float age  = 0.0f;      // 현재 나이
    float life = 1.0f;      // 수명
};