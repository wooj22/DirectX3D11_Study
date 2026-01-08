#include "FilpbookEffect.h"
#include "../WinBase/Time.h"

void FilpbookEffect::Play(Vector3 pos)
{
    particle.pos = pos;
    particle.age = 0.0f;
    alive = true;
    frame = 0;
}

void FilpbookEffect::Update()
{
    if (!alive) return;

    float dt = Time::GetDeltaTime();
    particle.age += dt;

    // life cheak
    if (particle.age >= particle.life)
    {
        if (loop)
            particle.age = fmodf(particle.age, particle.life);
        else
        {
            alive = false;
            return;
        }
    }

    // filpbook frame
    int total = sheet.cols * sheet.rows;
    frame = (int)floorf(particle.age * sheet.fps);

    if (loop) frame %= total;
    else frame = min(frame, total - 1);
}