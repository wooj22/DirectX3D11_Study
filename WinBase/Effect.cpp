#include "Effect.h"
#include "Time.h"


void Effect::Play()
{
    playing = true;
    for (auto& e : emitters)
    {
        e.playing = true;
        e.elapsed = 0.0f;
        e.emitAcc = 0.0f;
        e.particles.clear();

        e.position = position + e.localOffset;

        // burst
        if (e.burstCount > 0)
            e.Spawn(e.burstCount);
    }
}

void Effect::Stop()
{
    playing = false;
    for (auto& e : emitters) e.playing = false;
}

void Effect::Update()
{
    if (!enabled || !playing) return;

    allFinished = true;

    // emitter udpate
    for (auto& e : emitters)
    {
        e.position = e.localOffset + position;
        e.Update();

        if (e.playing || !e.particles.empty())
            allFinished = false;
    }

    // loop
    if (allFinished)
    {
        if (looping)
            Play();
        else
            playing = false;
    }
}