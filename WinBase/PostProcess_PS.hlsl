// PostProcess PixelShader
// 렌더링 후처리 단계

// [Post Process]
// ToneMapping + Exposure + ColorGrading
// TODO :: Bloom, Vignette, Film Grain

// [Screen Space Effect]
// 물결, 필름그레인, 플라즈마 효과

#include <PostFxCommon.fxh>

Texture2D sceneHDR : register(t12);
SamplerState samplerLinear : register(s0);


float4 main(PS_FullScreen_Input input) : SV_TARGET
{ 
    // UV distortion
    float2 uv = input.uv;
    if (enableWaterDistortion)
        uv = ApplyWaterDistortion(input.uv);

    // HDR sample
    float3 hdr = sceneHDR.Sample(samplerLinear, uv).rgb;

    // exposure
    float exposureScale = pow(2.0, exposure);
    hdr *= exposureScale;

    // tone mapping
    float3 mapped = ACESFilm(hdr);

    // color grading
    float3 colorGrade = ApplyColorGrading(mapped);

    // screen space Effect
    float3 finalColor = colorGrade;
    if (enablePlasmaOverlay)
        finalColor = ApplyPlasmaOverlay(input.uv, finalColor);
    if(enableFilmGrain)
        finalColor = ApplyFilmGrain(input.uv, finalColor);

    // gamma
    if (useGamma && isHDR)
        finalColor = LinearToSRGB(finalColor);

    return float4(finalColor, 1.0f);
}