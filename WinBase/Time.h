#pragma once
#include <windows.h>

class Time
{
private:
    static LARGE_INTEGER frequency;
    static LARGE_INTEGER initCounter;
    static LARGE_INTEGER prevCounter;
    static LARGE_INTEGER currentCounter;

    static float deltaTime;
    static float fixedDeltatime;
    static float unscaledDelta;
    static float unscaledFixedDeltaTime;
    static float timeScale;

public:
    static void Init();
    static void Update();

    static float GetDeltaTime();
    static float GetUnscaledDeltaTime();
    static float GetFixedDeltaTime();
    static float GetUnscaledFixedDeltatime();
    static float GetTotalTime();

    static void SetTimeScale(float scale);
    static float GetTimeScale();
};