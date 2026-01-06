#pragma once
#include <directxtk/simplemath.h>
using namespace DirectX::SimpleMath;

/*
    [ Skybox Renderer ]
    
    SKybox Render Stage를 맞추고, 전달받은 Skybox를 Draw합니다.

    ** Skybox Renderer의 Renderable 객체 **
     SkyBox
*/

class SkyBox;

class SkyboxRenderer
{
public:
    void SkyboxPass(const Matrix& view, const Matrix& projection, const SkyBox& skybox);
};

