// PostProcess PixelShader
// 렌더링 후처리 단계

// [Post Process]
// 노출 + ToneMapping
//  - Color Adjustments (채도, 대비, Hue Shift, Tint)
//  - Bloom
//  - Film Grain
//  - Vignette
//  - Lift, Gamma, Gain
//  - White Balance (온도, 색조)

// [Screen Space Effect]
// 물결, 필름그레인, 플라즈마 효과

#include <PostFxCommon.fxh>

Texture2D sceneHDR : register(t12);
SamplerState samplerLinear : register(s0);


float4 main(PS_FullScreen_Input input) : SV_TARGET
{ 
    // UV 왜곡
    float2 uv = input.uv;
    
    // UV 왜곡
    // ScreenFx - Ripple
    if (enableRipple)
        uv = ApplyRipple(input.uv);
        
    // HDR sample
    float3 hdr = sceneHDR.Sample(samplerLinear, uv).rgb;

    // Exposure
    float exposureScale = pow(2.0, exposure);
    hdr *= exposureScale;

    // tone Mapping
    float3 mapped = ACESFilm(hdr);

    // Color Adjustments
    float3 colorGrade = ApplyColorAdjustments(mapped);

    // ScreenFx - Plasma
    float3 finalColor = colorGrade;
    if (enablePlasmaOverlay)
        finalColor = ApplyPlasmaOverlay(input.uv, finalColor);
    
    // ScreenFx - Film Grain
    if(enableFilmGrain)
        finalColor = ApplyFilmGrain(input.uv, finalColor);

    // gamma
    if (useDefalutGamma && isHDR)
        finalColor = LinearToSRGB(finalColor);

    return float4(finalColor, 1.0f);
}