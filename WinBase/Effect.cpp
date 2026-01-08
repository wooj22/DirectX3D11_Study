#include "Effect.h"
#include "Time.h"

void Effect::Play(Vector3 pos)
{
    for (auto& p : particles)
    {
        p.pos = pos;
        p.age = 0.0f;
    }
}

void Effect::Update()
{
    float dt = Time::GetDeltaTime();
    int total = sheet.cols * sheet.rows;

    // particle udapte
    for (auto& p : particles)
    {
        if(!p.alive) continue;

        // life
        p.age += dt;
        if (p.age >= p.life)
        {
            if (loop)
                p.age = fmodf(p.age, p.life);
            else
            {
                p.alive = false;
                return;
            }
        }

        // frame
        p.frame = (int)floorf(p.age * sheet.fps);
        if (loop) p.frame %= total;
        else p.frame = min(p.frame, total - 1);
    }
}