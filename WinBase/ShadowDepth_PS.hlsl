// 알파 픽셀을 ShadowMap에 기록하지 않기 위한 Pixel Shader
// 필요 없다면 Depth Only Pass는 pixel shader 실행 x

#include <shared.fxh>

// --- Texture Bind Slot ------------------
Texture2D diffuseMap : register(t0);

// --- Sampler Bind Slot ------------------
SamplerState samLinear : register(s0);


float4 main(PS_INPUT input) : SV_TARGET
{
    float alpha = 1.0f;
    if (useDiffuse)
        alpha = diffuseMap.Sample(samLinear, input.texCoord).a;
    
    clip(alpha - 0.5f);
    return 0;
}