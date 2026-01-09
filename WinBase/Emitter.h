#include "SpriteSheet.hpp"
#include "Particle.hpp"
#include <vector>
using std::vector;

// 하드한 부분이 많지만...

/*
    [ Emitter ]

     파티클은 언제, 어디서, 몇개 만들어낼지 정하는 규칙으로, 
     파티클을 실제로 Spawn/Update/Render하는 단위이다.
     Effect를 만들고싶다면 이 Emitter를 정의하여 Effect에 추가해주면 된다.

     - Emitter가 소속된 Effect에서 Update를 호출해주며, Emitter는 본인이 소유한 particle을 관리한다.
     - 하나의 Emitter는 같은 RenderPipeLine Stage 상태를 공유한다. (CB, Shader, SRV, Blenader State 등..)
     - Particle Renderer에서는 Emitter 단위로 DrawCall을 수행하여 (GPU 상태가 같으니까)
       Particle Pass에서의 Draw Call 수는 scene에 있는 Emitter의 개수와 같다.



    ** ParticleMode **
     1. Fixed   : Filpbook 전용 Mode      -> SpawnFixed data 지정하기
     2. Dynamic : 일반 파티클 시스템 Mode  -> SpawnDynamic data 지정하기



    ** Flipbook 구분 **
     
     SpriteSheet에는 실제 sprite sheet가 있을수도 있고, 하나의 sprite가 있을 수도 있다.
     또는 Sprite Sheet에서 하나의 Sprite만 Particle Resource로 사용할 수 도 있다.
     TODO :: 추후에는 Sprite를 중간에 추가하는게 나을듯

     < Particle Filpbook 설정 >
      1) frameCount      : 1개 이상이면 filpbook animation
      2) fps             : 0 이상이면 1초에 재생할 frame이 많다는 의미이므로 filpbook animation
      선택) loop          : ture라면 frame이 반복재생, false라면 한번만 재생

     이 두개가 설정되어있으면 Emitter가 particle을 udpate할때 알맞은 Frame을 계산해준다.
     만약 단일 Sprite라면 Frame은 0으로 고정된다.



    ** Single Particle Effect (Filpbook Effect) **
     
     Filpbook Effect라하면 단일 Quad를 띄워 frame animation으로 Effect를 연출하는 것을 말한다.
     아래 Emission에 따라 Filpbook Effect를 연출할수도, Particle System을 연출할수도 있다.

     < Filpbook Effect 설정 >
      1) maxParticles = 1
      2) burstCount   = 1

      Effect를 Play할때 burst로 하나를 생성하고, 추가 생성하지 않는다.

      3) ParticleMode = ParticleMode::Fixed
      4) FlipbookPlayMode 필수 선택 지정

      지정된 위치에서 burst Particle이 생성되도록 하며, 이후 Motion udpate를 하지 않는다.
      particle이 하나이기 때문에 emitter의 duration이 작동하지 않으며
      particle이 죽은 시점(!alive)에 emitter의 playing이 끝납니다.
*/

enum class BillboardType
{
    ScreenFacing,       // Quad가 항상 정면
    YAxis,              // Y축 고정
};

enum class ParticleMode
{
    Dynamic,            // Random 생성, 모션 o   -> particle system
    Fixed               // 고정 생성, 모션 x     -> filpbook Effect
};

enum class FlipbookPlayMode
{
    Once_Then_Die,       // 한 번 재생하고 종료
    Once_Then_Hold,      // 한 번 재생하고 마지막 프레임 고정
    Loop                 // 계속 반복
};

struct SpawnDynamic
{
    float   lifeMin = 0.5f;
    float   lifeMax = 1.0f;

    float   rotationMin = 0.0f;
    float   rotationMax = 0.0f;
    Vector2 sizeMin = Vector2::One;
    Vector2 sizeMax = Vector2::One;

    float   speedMin = 0.0f;
    float   speedMax = 10.0f;
    float   angularMin = 0.0f;
    float   angularMax = 0.0f;

    Vector4 startColor = { 1,1,1,1 };       // TODO :: range
};

struct SpawnFixed
{
    float     rotation   = 0.0f;
    Vector2   size       = Vector2::One;
    Vector4   startColor = { 1,1,1,1 };
};


class Emitter
{
public:
    SpriteSheet      sheet;
    vector<Particle> particles;
    BillboardType    billboard = BillboardType::ScreenFacing;

    // position
    Vector3 position    = Vector3::Zero;       // effect pos
    Vector3 localOffset = Vector3::Zero;       // effect pos 기준 offset

    // control
    bool enabled = true;
    bool playing = true;

    // timer
    float elapsed = 0.0f;       // Duration timer
    float emitAcc = 0.0f;       // Spawn timer

    // emission
    float duration = 0.0f;
    bool  looping = true;
    float emitRate = 0.0f;
    int   burstCount = 0;
    int   maxParticles = 256;

    // mode, data
    ParticleMode particleMode = ParticleMode::Dynamic;
    SpawnDynamic dynamicData;
    SpawnFixed   fixedData;

    // Filpbook particle life 특수 처리 ⭐
    FlipbookPlayMode filpbookPlayMode = FlipbookPlayMode::Loop;
    float holdTime = 5.0f;
    float infinite = 999999.0f;


public:
    void Update();
    void Spawn(int count);
};

float Rand01();
float RandRange(float a, float b);
Vector3 RandomUnitVector3();
