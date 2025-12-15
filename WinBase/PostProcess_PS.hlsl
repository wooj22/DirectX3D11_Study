#include <shared.fxh>

// PostProcess PixelShader
// 렌더링 후처리 단계
// ToneMapping + Exposure + ColorGrading
// TODO :: Bloom, Vignette, Film Grain


Texture2D sceneHDR : register(t12);
SamplerState samplerLinear : register(s0);

// ColorGrading
float3 ApplyColorGrading(float3 color)
{
    // 색상 이동
    // TODO :: hue shift

    // 대비
    color = ((color - 0.5) * contrast + 0.5);

    // 채도
    float gray = dot(color, float3(0.299, 0.587, 0.114));
    color = lerp(float3(gray, gray, gray), color, saturation);

    // 톤
    if (useTint)
        color += colorTint;

    return saturate(color);
}


// ACES Filmic Tone Mapping : HDR -> LDR 압축 (0~1)
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate(x * (a * x + b) / (x * (c * x + d) + e));
}

float4 main(PS_FullScreen_Input input) : SV_TARGET
{
    // hdr scene smapling
    float3 hdr = sceneHDR.Sample(samplerLinear, input.uv).rgb;

    // exposure
    hdr *= exposure;
    
    // tone mapping
    float3 mapped = ACESFilm(hdr);
    
    // Color Grading
    float3 colorGrade = ApplyColorGrading(mapped);

    return float4(colorGrade, 1.0f);
}