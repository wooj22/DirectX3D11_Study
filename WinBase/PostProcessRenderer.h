#pragma once
#include "IRenderer.h"

/*
    [ PostProcess Renderer ]

    최종 sceneHDR을 LDR로 톤매핑하고, 
    후처리 후 BackBuffer에 최종 렌더합니다.


    ** PostProcess Renderer의 Renderable 객체 **
     x
*/


class PostProcessRenderer : public IRenderer
{
private:
    // clear color
    float clearColor[4] = { 0,0,0,1 };

public:
    // interface (확장성)
    ~PostProcessRenderer() override = default;
    void Initialize() override {};
    void RenderPass() override {};

    // non interface function (Legucy)
    void Init() {};
    void PostProcessPass();
};

