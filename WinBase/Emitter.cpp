#define NOMINMAX        // min, max
#include "Emitter.h"
#include "Time.h"
#include <algorithm>

#include <DirectXMath.h>
using namespace DirectX;

#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;


void Emitter::Update()
{
    float dt = Time::GetDeltaTime();
    elapsed += dt;

    // Dynamic Emitter Life Cheak
    if (particleMode == ParticleMode::Dynamic)
    {
        // duration
        if (duration > 0.0f && elapsed >= duration)
        {
            if (looping)
            {
                elapsed = 0.0f;
                emitAcc = 0.0f;
            }
            else
            {
                playing = false;    // spawn stop
            }
        }

        // emission
        bool canEmit = (duration <= 0.0f) || (elapsed < duration) || looping;
        if (canEmit && emitRate > 0.0f)
        {
            emitAcc += emitRate * dt;
            int toSpawn = (int)emitAcc;
            if (toSpawn > 0)
            {
                emitAcc -= (float)toSpawn;
                Spawn(toSpawn);
            }
        }
    }


    // Particle Update
    for (auto& p : particles)
    {
        if (!p.alive) continue;

        // life
        p.age += dt;
        if (p.age >= p.life)
        {
            p.alive = false;
            continue;
        }

        // motion
        if(particleMode == ParticleMode::Dynamic)
        {
            // motion
            p.pos += p.vel * dt;
            p.rotation += p.angularVel * dt;

            // alpha fade
            float t = p.age / p.life;
            t = std::clamp(t, 0.0f, 1.0f);
            float a = 1.0f - t;
            a = a * a;
            p.color.w = dynamicData.colorMax.w * a;
        }
        else if (particleMode == ParticleMode::Fixed)
        {
            p.pos = position;
        }

        // frame (filpbook)
        if (sheet.fps > 0.0f && sheet.frameCount > 1)
        {
            int f = (int)(p.age * sheet.fps);

            bool loopFrames =
                (filpbookPlayMode == FlipbookPlayMode::Loop)
                ? true : sheet.loop;

            if (loopFrames)
            {
                f %= sheet.frameCount;
                if (f < 0) f += sheet.frameCount;
            }
            else
            {
                f = std::clamp(f, 0, sheet.frameCount - 1);
            }

            p.frame = f;
        }
        else
        {
            p.frame = 0;    // single sprite particle
        }
    }

    // particle dead (No erase)
    for (size_t i = 0; i < particles.size(); )
    {
        if (!particles[i].alive)
        {
            // 죽은 파티클 자리에 마지막 파티클 복사 -> pop_back()
            particles[i] = particles.back();
            particles.pop_back();
            continue;
        }
        ++i;
    }

    // Fixed Emitter Life Cheak
    if (particleMode == ParticleMode::Fixed)
    {
        if (particles.empty())
            playing = false;
    }
}

void Emitter::Spawn(int count)
{
    if (count <= 0) return;
    if (!enabled || !playing) return;

    // spawn count limit
    int available = maxParticles - (int)particles.size();
    if (available <= 0) return;
    count = std::min(count, available);

    // reserve
    particles.reserve((size_t)maxParticles);

    // spawn
    for (int i = 0; i < count; ++i)
    {
        Particle p{};
        p.alive = true;
        p.age = 0.0f;
        p.frame = 0;
        p.pos = position;

        if (particleMode == ParticleMode::Dynamic)
        {
            // life
            p.life = RandRange(dynamicData.lifeMin, dynamicData.lifeMax);
            if (p.life <= 0.0f) p.life = 0.01f;

            // velocity (for shape)
            float speed = RandRange(dynamicData.speedMin, dynamicData.speedMax);
            Vector3 dir;

            switch (velocityShape)
            {
            case VelocityShape::Sphere:
                dir = RandomUnitVector3();
                break;

            case VelocityShape::Directional:
                dir = emitDir;
                dir.Normalize();
                break;

            case VelocityShape::Cone:
                dir = RandomDirectionInCone(emitDir, coneAngleDeg);
                break;

            case VelocityShape::Disk:
                dir = RandomDirectionOnPlane(emitDir);
                break;
            }
            p.vel = dir * speed;

            // size
            p.size.x = RandRange(dynamicData.sizeMin.x, dynamicData.sizeMax.x);
            p.size.y = RandRange(dynamicData.sizeMin.y, dynamicData.sizeMax.y);

            // rotation
            p.rotation = RandRange(dynamicData.rotationMin, dynamicData.rotationMax);
            p.angularVel = RandRange(dynamicData.angularMin, dynamicData.angularMax);

            // color
            p.color = RandRange(dynamicData.colorMin, dynamicData.colorMax);
        }
        else if (particleMode == ParticleMode::Fixed)
        {
            p.rotation = fixedData.rotation;
            p.size = fixedData.size;
            p.color = fixedData.startColor;

            // life
            float anim = sheet.GetFilpbookDuration();
            switch (filpbookPlayMode)
            {
            case FlipbookPlayMode::Once_Then_Die:
                p.life = anim;
                break;

            case FlipbookPlayMode::Once_Then_Hold:
                p.life = anim + holdTime;
                break;

            case FlipbookPlayMode::Loop:
                p.life = infinite;
                break;
            }
        }

        particles.push_back(p);
    }
}

// Random Utils
float Rand01()
{
    return (float)rand() / (float)RAND_MAX;
}

float RandRange(float a, float b)
{
    return a + (b - a) * Rand01();
}

Vector3 RandomUnitVector3()
{
    float x = RandRange(-1.0f, 1.0f);
    float y = RandRange(-1.0f, 1.0f);
    float z = RandRange(-1.0f, 1.0f);
    Vector3 v(x, y, z);

    if (v.LengthSquared() < 1e-6f)
        return Vector3(0, 1, 0);

    v.Normalize();
    return v;
}

Vector3 RandomDirectionInCone(const Vector3& axis, float angleDeg)
{
    Vector3 n = axis;
    if (n.LengthSquared() < 1e-6f)
        n = Vector3::Up;
    n.Normalize();

    // cone half-angle (rad)
    float angleRad = XMConvertToRadians(angleDeg);

    // cos(theta) 범위
    float cosMax = cosf(angleRad);
    float cosTheta = RandRange(cosMax, 1.0f);
    float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

    // 원뿔 원주 각
    float phi = RandRange(0.0f, XM_2PI);

    // axis에 수직인 basis 생성
    Vector3 tangent;
    if (fabsf(n.y) < 0.999f)
        tangent = n.Cross(Vector3::Up);
    else
        tangent = n.Cross(Vector3::Right);
    tangent.Normalize();

    Vector3 bitangent = n.Cross(tangent);

    // 방향 합성
    Vector3 dir =
        tangent * (cosf(phi) * sinTheta) +
        bitangent * (sinf(phi) * sinTheta) +
        n * cosTheta;

    dir.Normalize();
    return dir;
}

Vector3 RandomDirectionOnPlane(const Vector3& normal)
{
    Vector3 n = normal;
    if (n.LengthSquared() < 1e-6f)
        n = Vector3::Up;
    n.Normalize();

    // normal에 수직인 basis
    Vector3 tangent;
    if (fabsf(n.y) < 0.999f)
        tangent = n.Cross(Vector3::Up);
    else
        tangent = n.Cross(Vector3::Right);
    tangent.Normalize();

    Vector3 bitangent = n.Cross(tangent);

    float angle = RandRange(0.0f, XM_2PI);

    Vector3 dir =
        tangent * cosf(angle) +
        bitangent * sinf(angle);

    dir.Normalize();
    return dir;
}

Vector4 RandRange(const Vector4& a, const Vector4& b)
{
    return {
        RandRange(a.x, b.x),
        RandRange(a.y, b.y),
        RandRange(a.z, b.z),
        RandRange(a.w, b.w)
    };
}