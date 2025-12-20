#include <PBR_Common.fxh>


// ----------------------------------------
// [ Post Process ] 
// ----------------------------------------
// Color Adjustments
float3 ApplyColorAdjustments(float3 color)
{
    // 색상 이동
    if (useHueShift)
    {
        // RGB -> YIQ
        float Y = dot(color, float3(0.299, 0.587, 0.114));
        float I = dot(color, float3(0.596, -0.274, -0.322));
        float Q = dot(color, float3(0.211, -0.523, 0.312));

        float cosA = cos(hueShift);
        float sinA = sin(hueShift);

        float I2 = I * cosA - Q * sinA;
        float Q2 = I * sinA + Q * cosA;

        // YIQ -> RGB
        color.r = Y + 0.956 * I2 + 0.621 * Q2;
        color.g = Y - 0.272 * I2 - 0.647 * Q2;
        color.b = Y - 1.106 * I2 + 1.703 * Q2;
    }

    // 대비
    color = ((color - 0.5) * contrast + 0.5);

    // 채도
    float gray = dot(color, float3(0.299, 0.587, 0.114));
    color = lerp(float3(gray, gray, gray), color, saturation);

    // 톤
    if (useTint)
        color = lerp(color, color * colorTint, colorTint_strength);

    return saturate(color);
}

// Film Grain
float3 ApplyFilmGrain(float3 color)
{
    return color;
}

// Vinette
float3 ApplyVignette(float2 uv, float3 color)
{
    // 화면 종횡비 보정
    // screenTexelSize = (1/width, 1/height)
    float aspect = screenTexelSize.y / max(screenTexelSize.x, 1e-8); 
    float2 p = uv - vignetteCenter;
    p.x *= aspect;

    // 중심에서 코너까지의 최대 거리로 정규화 0(center) ~ 1(corner)
    float2 corner = float2(0.5 * aspect, 0.5);
    float rMax = max(length(corner), 1e-6);
    float d = length(p) / rMax;

    // intensity : 어두운 영역 범위(inner)
    // smoothness : 부드러운 경계(outer)
    float intensity = saturate(vignette_intensity);
    float smoothness = saturate(vignette_smoothness);
    
    float inner = saturate(1.0 - intensity);
    float outer = saturate(inner + max(smoothness, 1e-4));
    float mask = smoothstep(inner, outer, d);

    // vinette color
    float t = mask * intensity;
    return lerp(color, vignetteColor, t);
    
    return color;
}

// Lift/ gamma/ Gain
float3 ApplyLGG(float3 color)
{
    // Lift (shadows offset)
    if (useLift)
    {
        color += lift * lift_strength;
        color = max(color, 0.0);
    }

    // Gamma (midtones curve)
    if (useGamma)
    {
        // float3 v = {1,1,1} == 1.0.xxx
        float3 gammaPower = lerp(1.0.xxx, 1.0.xxx + gamma, gamma_strength);
        gammaPower = max(gammaPower, 0.05.xxx); // 폭발 방지
        color = pow(max(color, 1e-4.xxx), 1.0 / gammaPower);
    }

    // Gain (highlights scale)
    if (useGain)
    {
        float3 gainScale = lerp(1.0.xxx, 1.0.xxx + gain, gain_strength);
        gainScale = max(gainScale, 0.0.xxx); // 음수 방지
        color *= gainScale;
    }

    return color;
}

// White Balance
float3 ApplyWhiteBalance(float3 color)
{
    // Temperature : Blue <-> Yellow
    float3 tempOffset = float3(0.10, 0.0, -0.10) * temperature;

    // Tint : Magenta <-> Green 
    float3 tintOffset = float3(-0.05, 0.10, -0.05) * tint;

    color += tempOffset + tintOffset;
    return color;
}



// ----------------------------------------
// [ Random -> Noise -> FBM ->Domain Warping ]
// ----------------------------------------
// Random
float Random(float2 uv)
{
    float v = sin(dot(uv, float2(22.8897, 98.537))) * randomIntensity;
    return frac(v);
}

// Perlin Fade
float2 PerlinFade(float2 t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// Value Noise
float Noise(float2 uv)
{
    float2 cellID = floor(uv);
    float2 f = frac(uv);

    float a = Random(cellID);
    float b = Random(cellID + float2(1, 0));
    float c = Random(cellID + float2(0, 1));
    float d = Random(cellID + float2(1, 1));

    float2 u = PerlinFade(f);

    float top = lerp(a, b, u.x);
    float bottom = lerp(c, d, u.x);
    return lerp(top, bottom, u.y);
}

// fbm: 0..~1
float FBM(float2 uv)
{
    float value = 0.0;
    float amp = 0.5;
    float freq = 1.0;

    [unroll]
    for (int i = 0; i < 5; i++)
    {
        value += amp * Noise(uv * freq);
        freq *= 2.0;
        amp *= 0.5;
    }
    return value;
}

float2 Warp(float2 uv)
{
    float2 offset1 = float2(5.2, 1.3);
    float2 offset2 = float2(1.7, 9.2);

    float wx = FBM(uv + offset1);
    float wy = FBM(uv + offset2);
    
    return (float2(wx, wy) * 2.0 - 1.0) * warpStrength;
}


// double domain warping
float DomainWarpValue(float2 uv)
{
    float2 w1 = Warp(uv);
    float2 w2 = Warp(uv + w1 * 0.5);
    return FBM(uv + w2);
}

// warp vector for distortion
float2 DomainWarpVector(float2 uv)
{
    float2 w1 = Warp(uv);
    float2 w2 = Warp(uv + w1 * 0.5);

    // use two decorrelated samples to build vector
    float vx = FBM(uv + w2 + float2(13.7, 9.1));
    float vy = FBM(uv + w2 + float2(4.2, 27.9));
    return float2(vx, vy) * 2.0 - 1.0; // -1..1
}

// ----------------------------------------
// simple palette for plasma
float3 palette(float t)
{
    float3 a = float3(0.50, 0.50, 0.50);
    float3 b = float3(0.50, 0.50, 0.50);
    float3 c = float3(1.00, 1.00, 1.00);
    float3 d = float3(0.00, 0.33, 0.67);
    return a + b * cos(6.2831853 * (c * t + d));
}

// ----------------------------------------
// [ Screen Space Effect ]
// ----------------------------------------
float2 ApplyRipple(float2 uv)
{
    float2 flowDir = normalize(float2(0.8, -0.6));

    // 저/고주파 분리
    float2 pLow = uv * (cellScale * 1.2) + flowDir * (time * 0.35);
    float2 pHigh = uv * (cellScale * 4.0) + flowDir * (time * 0.85) + float2(13.7, 9.1);

    float2 vLow = DomainWarpVector(pLow);
    float2 vHigh = DomainWarpVector(pHigh);

    // 물결 느낌 유지하면서도 눈에 띄게
    float2 wv = vLow * 1.0 + vHigh * 0.5;

    // 강도 즉폭
    wv *= 0.5;

    // 비선형으로 강화 (큰 값 더 크게)
    float l = max(length(wv), 1e-4);
    wv = (wv / l) * pow(l, 0.75);

    return uv + wv * (distortionStrength * 2.2);
}

float3 ApplyPlasmaOverlay(float2 uv, float3 ldrColor)
{
    // 화면 전체에 깔리는 움직임
    float2 p = uv * cellScale + float2(time * 0.18, time * 0.12);

    // 0 ~ 1
    float n = DomainWarpValue(p);

    // 대비/형태 강화: 전체적으로 보이되 패턴이 또렷하게
    float nContrast = pow(saturate(n), 1.35); // 전체 대비
    float bands = smoothstep(0.15, 0.95, nContrast); // 전체 마스크

    // 컬러
    float3 col = palette(nContrast);

    // 플라즈마 띠 강조
    float ridge = smoothstep(0.55, 0.85, nContrast);
    col += ridge * 0.35;

    // 전체적으로 보이게
    float a = saturate(plasmaIntensity * (0.45 + 0.55 * bands));
    return saturate(lerp(ldrColor, col, a));
}

float3 ApplyFilmGrain(float2 uv, float3 ldrColor)
{
    float2 p = uv * (cellScale * 120.0) + float2(time * 17.0, time * 9.0);
    float g = DomainWarpValue(p) * 2.0 - 1.0; // -1..1

    // stronger in darks
    float lum = dot(ldrColor, float3(0.2126, 0.7152, 0.0722));
    float amt = grainIntensity * lerp(1.0, 0.4, saturate(lum));

    return saturate(ldrColor + g * amt);
}

// ----------------------------------------
// ACES Filmic Tone Mapping : HDR -> LDR 압축 (0~1)
// ----------------------------------------
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate(x * (a * x + b) / (x * (c * x + d) + e));
}