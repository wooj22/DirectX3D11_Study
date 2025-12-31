/*
    [ PBR Lighting Pass Pixel Shader ] 
    * Deferred Rendering *

    Full Screen Quad를 그리며, G-buffer에 기록된 정보를 활용하여
    최종 가시 픽셀에 대해서만 라이팅 연산을 진행합니다.

    - Direct BRDF(Cook-Torrance)
    - Indirect IBL(BRDF)
    - Shadow Mapping Support
*/

#include <shared.fxh>
#include <PBR_Common.fxh>

// --- Texture Bind Slot ------------------
Texture2D shadowMap : register(t6);
TextureCube IBL_IrradianceMap : register(t9);
TextureCube IBL_SpecularEnvMap : register(t10);
Texture2D IBL_BRDF_LUT : register(t11);

// Gbuffer Textures
//Texture2D positionTex : register(t14);
Texture2D albedoTex : register(t15);
Texture2D normalTex : register(t16);
Texture2D metalRoughTex : register(t17);
Texture2D emissiveTex : register(t18);
Texture2D depthTex : register(t19);


// --- Sampler Bind Slot ------------------
SamplerState samLinear : register(s0);
SamplerComparisonState samShadow : register(s1);
SamplerState samLinearClamp : register(s2);



float4 main(PS_FullScreen_Input input) : SV_TARGET
{   
    // --- [ World Position ]  ----------------------------------
    float depth = depthTex.Sample(samLinearClamp, input.uv).r;

    if (depth >= 0.999999f)
        return 0;    // 배경 픽셀 스킵
    
    float4 ndc;
    ndc.x = input.uv.x * 2.0f - 1.0f;
    ndc.y = (1.0f - input.uv.y) * 2.0f - 1.0f;
    ndc.z = depth;
    ndc.w = 1.0f;
    
    float4 worldH = mul(ndc, inverseProjection);
    
    float w = worldH.w;
    float3 worldPos = worldH.xyz / w;

    
    // --- [ Read Gbuffer ]  ----------------------------------
    float2 samUV = input.uv;
    
    float4 base_sample = albedoTex.Sample(samLinearClamp, samUV);
    float3 base_color  = base_sample.rgb;
    float  alpha       = base_sample.a;
    
    float3 N = normalTex.Sample(samLinearClamp, samUV);
    N = DecodeNormal(N);        
    
    float2 matalrough = metalRoughTex.Sample(samLinearClamp, samUV).rg;
    float  metallic   = matalrough.x;
    float  roughness  = matalrough.y;
    
    float3 emissive   = emissiveTex.Sample(samLinearClamp, samUV).rgb;
    
    
    
    // --- [ ShadowMapping ] ------------------------------------
    float shadowFactor = 1.0f;
    
    // SumLight는 하나만 있음
    if (isSunLight) 
    {
        float4 posShadow = mul(float4(worldPos, 1), shadowView);
        posShadow = mul(posShadow, shadowProjection);
    
        float currentShadowDepth = posShadow.z / posShadow.w;
        float2 uv = posShadow.xy / posShadow.w;
        uv.y = -uv.y;
        uv = uv * 0.5 + 0.5;
    
        if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)
        {
            float2 offsets[9] =
            {
                float2(-1, -1), float2(0, -1), float2(1, -1),
            float2(-1, 0), float2(0, 0), float2(1, 0),
            float2(-1, 1), float2(0, 1), float2(1, 1)
            };
            float2 shadowmapsize = { 3000, 3000 };
            float2 texelSize = 1.0 / shadowmapsize; // 텍셀 크기 (ShadowMap 해상도 기준)
            shadowFactor = 0.0f;
       
       //  PCF - 9 texel 평균으로 그림자 팩터 계산
       [unroll]
            for (int i = 0; i < 9; i++)
            {
                float2 sampleUV = uv + offsets[i] * texelSize;
                shadowFactor += shadowMap.SampleCmpLevelZero(samShadow, sampleUV, currentShadowDepth - 0.001);
            }
            shadowFactor = shadowFactor / 9.0f;
        }
    }
    
    
    
    
    // --- [Vector / Attenuation]  ------------------------------------------
    float3 L = 0;
    float3 V = normalize(cameraPos - worldPos);
    float attenuation = 1.0f;
    float spotFactor = 1.0f;
    
    // Directional Light
    if (lightType == 0) 
    {
        L = normalize(-lightDirection);
        attenuation = 1.0f;
        spotFactor = 1.0f;
    }
    // Point Light
    else if (lightType == 1) 
    {
        float3 toLight = lightPos - worldPos; // surface -> light
        float dist = length(toLight);

        // range 밖은 영향 0
        if (dist >= lightRange || lightRange <= 0.0001f)
        {
            attenuation = 0.0f;
            spotFactor = 0.0f;
        }

        L = toLight / max(dist, 1e-4f);

        // 감쇠
        float rangeAtt = saturate(1.0f - dist / lightRange);
        rangeAtt *= rangeAtt;
        float invSq = 1.0f / max(dist * dist, 1e-4f);
        attenuation = rangeAtt * invSq;

        spotFactor = 1.0f;
    }
    // Spot Light
    else if(lightType == 2)
    {
        float3 toLight = lightPos - worldPos; // surface -> light
        float dist = length(toLight);

        if (dist >= lightRange || lightRange <= 0.0001f)
            return float4(0, 0, 0, 0);

        L = toLight / max(dist, 1e-4f);

        // 거리 감쇠
        float rangeAtt = saturate(1.0f - dist / lightRange);
        rangeAtt *= rangeAtt;
        float invSq = 1.0f / max(dist * dist, 1e-4f);
        attenuation = rangeAtt * invSq;

        // 스팟 콘 감쇠 (inner~outer 부드럽게)
        // lightDirection이 "빛이 나가는 방향"이라고 가정하면:
        // spotlight forward = normalize(lightDirection)
        float3 spotDir = normalize(lightDirection);

        // 라이트 위치에서 픽셀을 바라보는 방향(light -> surface)
        float3 lightToSurfaceDir = normalize(worldPos - lightPos);

        // spotDir(라이트 forward) 와 lightToSurfaceDir의 각도 코사인
        float cosTheta = dot(spotDir, lightToSurfaceDir);

        // innerAngle/outerAngle이 degree라면 radians() 필요
        float cosInner = cos(radians(innerAngle));
        float cosOuter = cos(radians(outerAngle));

        // outer가 inner보다 커야 정상(외각이 더 넓음) => cosOuter <= cosInner
        // smoothstep(x0,x1,v): x0~x1 구간을 0~1로 부드럽게
        spotFactor = smoothstep(cosOuter, cosInner, cosTheta);

        // 콘 밖이면 영향 0
        if (spotFactor <= 0.0001f)
        {
            attenuation = 0.0f;
            spotFactor = 0.0f;
        }
    }
    
    // half vector, dot products
    float3 H = normalize(L + V);
    float NdotL = max(dot(N, L), 0.0f);
    float NdotV = max(dot(N, V), 0.0f);
    
    
    
    // --- [Direct Light]  ------------------------------------
    // Specular BRDF (Cook-Torrance)
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), base_color, metallic);
    float D = D_NDFGGXTR(N, H, roughness);   // 미세면 정렬정도
    float3 F = F_Schlick(H, V, F0);          // 프레넬 반사율
    float G = G_Smith(N, V, L, roughness);   // shadowing & masking
    
    float denom = 4.0f * max(NdotL, 0.001) * max(NdotV, 0.001);
    float3 SpecularBRDF = (D * F * G) / denom;
    
    // Diffuse BRDF (Lambertian)
    float3 kd = lerp(1.0 - F, 0.0, metallic); // 표면산란 계수
    float3 DiffuseBRDF = (base_color / PI) * kd;
    
    // Final DirectLight
    float3 DirectColor = (SpecularBRDF + DiffuseBRDF) * lightColor * directIntensity * (NdotL * attenuation * spotFactor);
    
    
    
    
    // --- [Indirect Light]  ----------------------------------
    float3 IndirectColor = { 0, 0, 0 };
    if (useIBL)
    {
        // Diffuse Term --------------------------
        // Irradiance - diffuse BRDF 적분값
        float3 Irradiance = IBL_IrradianceMap.Sample(samLinear, N).rgb;
        float3 DiffuseIBL = base_color * Irradiance * kd;
        
        // Specular Term -----------------------
        uint specularTextureLevels, width, height;
        IBL_SpecularEnvMap.GetDimensions(0, width, height, specularTextureLevels);
        float maxLevel = max(1.0, (float) (specularTextureLevels - 1));
        float mip = saturate(roughness) * maxLevel;
        
        // Prefiltered - 환경 Radiance + D(미세면 분포) + roughness 관련 적분값
        float3 R = normalize(reflect(-V, N));
        float3 PrefilteredColor = IBL_SpecularEnvMap.SampleLevel(samLinear, R, mip).rgb;

        // LUT - F + G 적분값
        float2 BRDF_LUT = IBL_BRDF_LUT.Sample(samLinearClamp, float2(NdotV, roughness)).rg;
        
        // Specular IBL
        float3 SpecularIBL = PrefilteredColor * (F0 * BRDF_LUT.x + BRDF_LUT.y);

        // Final InDirectLight
        IndirectColor = (DiffuseIBL + SpecularIBL) * indirectIntensity;
    }
    

    
    // --- [Final Color]  ----------------------------------
    float3 finalColor = (DirectColor * shadowFactor) + IndirectColor + emissive;

     // LDR 단독패스일 때만 감마보정
    if (useDefaultGamma && !isHDR)
        finalColor = LinearToSRGB(finalColor);

    return float4(finalColor, alpha);
}
