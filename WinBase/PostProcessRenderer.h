#pragma once

/*
    [ PostProcess Renderer ]

    최종 sceneHDR을 LDR로 톤매핑하고, 
    후처리 후 BackBuffer에 최종 렌더합니다.
*/


class PostProcessRenderer
{
private:
    // clear color
    float clearColor[4] = { 0,0,0,1 };

public:
    void PostProcessPass();
};

