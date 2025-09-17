#pragma once
#include "Windows.h"

class Input
{
private:
    static HWND hWnd;
    static POINT mouseClient;
    static POINT prevMouseClient;
    static SHORT prevState[256];
    static SHORT currState[256];

    static float horizontalAxis;
    static float verticalAxis;

    // Mouse Mode
    enum MouseMode { MODE_ABSOLUTE, MODE_RELATIVE };
    static MouseMode mouseMode;
    static POINT mouseDelta;

public:
    static void Init(HWND hwnd);
    static void Update();

    static bool GetKey(int vKey);
    static bool GetKeyDown(int vKey);
    static bool GetKeyUp(int vKey);

    static bool GetMouseButton(int button);
    static bool GetMouseButtonDown(int button);
    static bool GetMouseButtonUp(int button);

    static float GetAxisHorizontal();
    static float GetAxisVertical();

    static void SetMouseMode(bool relative);      // true → relative, false → absolute
    static POINT GetMouseDelta();                 // 상대 이동량 반환
};