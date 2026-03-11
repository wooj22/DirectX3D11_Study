#pragma once
#include "IRenderer.h"
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

/*
    [ Skybox Renderer ]
    
    SKybox Render Stage를 맞추고, 전달받은 Skybox를 Draw합니다.

    ** Skybox Renderer의 Renderable 객체 **
     SkyBox
*/

class SkyBox;

class SkyboxRenderer : public IRenderer
{
public:
    // interface (확장성)
    ~SkyboxRenderer() override = default;
    void Initialize() override {};
    void RenderPass() override {};

    // non interface function (Legucy)
    void Init() {};
    void SkyboxPass(const Matrix& view, const Matrix& projection, const SkyBox& skybox);
};

