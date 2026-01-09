#define NOMINMAX        // min, max
#include "Emitter.h"
#include "Time.h"
#include <algorithm>

void Emitter::Update()
{
    float dt = Time::GetDeltaTime();
    elapsed += dt;

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

    // particle udpate
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
        p.pos += p.vel * dt;
        p.rotation += p.angularVel * dt;

        // color
        if (!sheet.frameAnimation)
        {
            float t = p.age / p.life;
            t = std::clamp(t, 0.0f, 1.0f);

            float a = 1.0f - t;
            a = a * a;

            p.color.w = startColor.w * a;
        }
        

        // frame (filpbook)
        if (sheet.fps > 0.0f && sheet.frameCount > 1)
        {
            int f = (int)(p.age * sheet.fps);

            if (sheet.frameAnimation)
            {
                f %= sheet.frameCount;
                if (f < 0) f += sheet.frameCount;
            }
            else
            {
                if (f < 0) f = 0;
                if (f > sheet.frameCount - 1) f = sheet.frameCount - 1;
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
}

void Emitter::Spawn(int count)
{
    if (count <= 0) return;
    if (!enabled || !playing) return;

    // limit
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
        p.life = RandRange(lifeMin, lifeMax);
        if (p.life <= 0.0f) p.life = 0.01f;

        p.pos = position;

        // velocity
        float speed = RandRange(speedMin, speedMax);
        Vector3 dir = RandomUnitVector3();
        p.vel = dir * speed;

        // size / rotation
        p.size.x = RandRange(sizeMin.x, sizeMax.x);
        p.size.y = RandRange(sizeMin.y, sizeMax.y);

        p.rotation = RandRange(rotationMin, rotationMax);
        p.angularVel = RandRange(angularMin, angularMax);

        // color
        p.color = startColor;

        // flipbook start frame
        p.frame = 0;

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