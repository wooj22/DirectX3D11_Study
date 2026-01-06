#pragma once

/*
    [ Bloom Renderer ]

    씬의 밝은 부분을 추출하여 더 밝게 하고, 빛이 번지는 효과를 추가한 BloomTexture를 생성합니다.
    PostProcess 단계에서 해당 이미지를 sceneHDR에 더해줍니다.

    ** Bloom Renderer의 Renderable 객체 **
     x

    ** Bloom Pass **
     1. Bloom Prefilter Pass          : sceneHDR의 밝은 부분을 추출
     2. Bloom Downsample Blur Pass    : 다운샘플 + 블러처리 하면서 MipMap Chain 형성
     3. Bloom Upsample Combine Pass   : 업샘플 + 가산합산 하면서 최종 Bloom Texture 생성
*/

class BloomRenderer
{
private:
    float clearColor[4] = { 0,0,0,1 };

public:
    void Init() {};
    void BloomPass();
};

