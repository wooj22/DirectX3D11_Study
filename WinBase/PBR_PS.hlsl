/*
    [ PBR Pixel Shader ]
    - Direct BRDF(Cook-Torrance)
    - Indirect IBL(BRDF)
    - Texture Map Support
        - Diffuse Map (Albedo Map)
        - Normal Map
        - Emissive Map
        - Metallic Map
        - Roughness Map
        - IBL_IrradianceMap
        - IBL_SpecularEnvMap
        - IBL_BRDF_LUT
    - Shadow Mapping Support
*/

#include <shared.fxh>

// --- Texture Bind Slot ------------------
Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D emissiveMap : register(t3);
Texture2D shadowMap : register(t6);
Texture2D metallicMap : register(t7);
Texture2D roughnessMap : register(t8);
TextureCube IBL_IrradianceMap : register(t9);
TextureCube IBL_SpecularEnvMap : register(t10);
Texture2D IBL_BRDF_LUT : register(t11);


// --- Sampler Bind Slot ------------------
SamplerState samLinear : register(s0);
SamplerComparisonState samShadow : register(s1);


// --- Cook -Torrance BRDF Functions ------
static const float PI = 3.141592f;
static const float EPSILON = 0.00001f;

// D (Normal Distribution Function)
float D_NDFGGXTR(float3 N, float3 H, float roughness)
{
    float NdotH = saturate(dot(N, H));
    float alpha = roughness * roughness;
    float lower = (NdotH * NdotH) * (alpha - 1.0) + 1.0;
    
    return alpha / max(EPSILON, PI * lower * lower);
}

// F (Fresnel reflection)
float3 F_Schlick(float3 H, float3 V, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - saturate(dot(H, V)), 5.0);
}

// GSub (SchlickGGX)
float G_SchlickGGX(float3 N, float3 V, float k)
{
    float NdotV = saturate(dot(N, V));
    return NdotV / (NdotV * (1.0 - k) + k);
}

// G (Geometric Attenuation Function)
float G_Smith(float3 N, float3 V, float3 L, float roughness)
{
    float alpha = roughness * roughness;
    float k = pow((alpha + 1), 2) / 8.0; // directinoal light

    return G_SchlickGGX(N, V, k) * G_SchlickGGX(N, L, k);
}




float4 main(PS_INPUT input) : SV_TARGET
{
    // --- [Default] ----------------------------------
    // color
    float3 base_color = float3(1.0f, 1.0f, 1.0f);
    float3 emissive_color = float3(0.0f, 0.0f, 0.0f);
    float metallic = 0.5f;
    float roughness = 0.5f;
    float alpha = 1.0f;
    
    // shadowFactor
    float shadowFactor = 1.0f;
    
    
    // --- [ShadowMapping] ---------------------------
    float currentShadowDepth = input.posShadow.z / input.posShadow.w;
    float2 uv = input.posShadow.xy / input.posShadow.w;
    uv.y = -uv.y;
    uv = uv * 0.5 + 0.5;
    
    if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)
    {
        // PCF
        shadowFactor = shadowMap.SampleCmpLevelZero(samShadow, uv, currentShadowDepth - 0.001);
    }

    
    // --- [Material]  ----------------------------------
    // base color
    if (useDiffuse)
        base_color = diffuseMap.Sample(samLinear, input.texCoord).rgb;
    
    // normal
    float3 N;
    if (useNormal)
    {
        float3 local_normal = normalMap.Sample(samLinear, input.texCoord).xyz * 2.0f - 1.0f;
        float3 world_normal = normalize(mul(local_normal, input.TBN));
        N = normalize(world_normal);
    }
    else
    {
        N = normalize(mul(input.normal, (float3x3) input.finalWorld));
    }
    
    // emission
    if (useEmissive)
        emissive_color = emissiveMap.Sample(samLinear, input.texCoord).rgb;
    
    // metallic
    if (useMetallic)
        metallic = metallicMap.Sample(samLinear, input.texCoord).r;

    // roughness
    if (useRoughness)
        roughness = roughnessMap.Sample(samLinear, input.texCoord).r;
    
    // alpha
    if (useDiffuse)
        alpha = diffuseMap.Sample(samLinear, input.texCoord).a;
    
    
    
    // --- [Override] ----------------------------------
    if (useMetallicOverride)
        metallic = metallicOverride;
    if (useRoughnessOverride)
        roughness = roughnessOverride;
    if (useBaseColorOverride)
        base_color = baseColorOverride;
    roughness = max(roughness, 0.04);
    
    // --- [Facotr] -----------------------------------
    metallic *= metallicFactor;
    float rf = max(roughnessFactor, 0.04);
    roughness *= rf;
    
    // --- [Vector]  ----------------------------------
    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 H = normalize(L + V);
    
    
    // --- [Direct Light]  ----------------------------------
    // Specular BRDF (Cook-Torrance)
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), base_color, metallic);
    float D = D_NDFGGXTR(N, H, roughness);      // 미세면 정렬정도
    float3 F = F_Schlick(H, V, F0);             // 프레넬 반사율
    float G = G_Smith(N, V, L, roughness);      // shadowing & masking
    
    float3 SpecularBRDF = (D * F * G) / (4.0f * saturate(dot(N, L)) * saturate(dot(N, V)));

    
    // Diffuse BRDF (Lambertian)
    float3 kd = lerp(1.0 - F, 0.0, metallic);   // 표면산란 계수
    float3 DiffuseBRDF = (base_color / PI) * kd;
    
    // Final DirectLight
    float3 directBRDF = (SpecularBRDF + DiffuseBRDF) * lightColor * directIntensity * dot(N, L);

    
    // --- [Indirect Light]  ----------------------------------
    float3 indirectIBL = { 0, 0, 0 };
    if (useIBL)
    {
        // 1) IBL Diffuse Term
        // Irradiance - diffuse BRDF 적분값
        float3 Irradiance = IBL_IrradianceMap.Sample(samLinear, N).rgb;     
        float DiffuseIBL = kd * base_color * Irradiance * PI;
        
        // 2) IBL Specular Term
        uint specularTextureLevels, width, height;
        IBL_SpecularEnvMap.GetDimensions(0, width, height, specularTextureLevels);
        
        // Prefiltered - 환경 Radiance + D(미세면 분포) + roughness 관련 적분값
        float3 R = reflect(-V, N);
        float3 PrefilteredColor = IBL_SpecularEnvMap.SampleLevel(samLinear, R, roughness * specularTextureLevels).rgb;

        // LUT - F + G 적분값
        float2 BRDF_LUT = IBL_BRDF_LUT.Sample(samLinear, float2(saturate(dot(N, L)), roughness)).rg;
        
        // Specular IBL
        float3 SpecularIBL = PrefilteredColor * (F0 * BRDF_LUT.x + BRDF_LUT.y);

        // Final InDirectLight
        indirectIBL = DiffuseIBL + SpecularIBL;
    }
    
    
    // --- [Final Color]  ----------------------------------
    float3 finalColor = (directBRDF + indirectIBL) * shadowFactor + emissive_color;
    return float4(finalColor, alpha);
}
