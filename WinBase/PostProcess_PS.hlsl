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

    // [ PostProcess ] ------------------------------
    float3 colorGrade = mapped;

    // White Balance
    if (useWhiteBalance)
        colorGrade = ApplyWhiteBalance(colorGrade);
      
    // LGG
    if (useLGG)
        colorGrade = ApplyLGG(colorGrade);
    
    // Color Adjustments
   if (useColorAdjustments)
        colorGrade = ApplyColorAdjustments(colorGrade);
    
      
    

    // [ ScreenFx ] ------------------------------
    // Plasma
    float3 screenFx = colorGrade;
    if (enablePlasmaOverlay)
        screenFx = ApplyPlasmaOverlay(input.uv, screenFx);
    
    // Film Grain
    if(enableFilmGrain)
        screenFx = ApplyFilmGrain(input.uv, screenFx);

    
    // [ Defalut Gamma ] --------------------------
    float3 finalColor = screenFx;
    if (useDefalutGamma && isHDR)
        finalColor = LinearToSRGB(finalColor);
    
    
    // [ Vinette ] --------------------------------
    if (useVinette)
        finalColor = ApplyVignette(uv, finalColor);

    return float4(finalColor, 1.0f);
}