#include <shared.fxh>

// PBR Pixel Shader
// ShadowMapping (현재 연계 가능한 VS : BaseLit_Skinned_VS, BaseLit_Static_VS)

// --- Texture Bind Slot ------------------
Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);      // 사용 x
Texture2D emissiveMap : register(t3);
Texture2D shadowMap : register(t6);
Texture2D metallicMap : register(t7);
Texture2D roughnessMap : register(t8);

// --- Sampler Bind Slot ------------------
SamplerState samLinear : register(s0);
SamplerComparisonState samShadow : register(s1);



// --- Cook -Torrance BRDF Functions ------
static const float PI = 3.141592f;
static const float Epsilon = 0.00001f;

// D (Normal Distribution Function)
float D_NDFGGXTR(float3 N, float3 H, float roughness)
{
    float NdotH = dot(N, H);
    float alpha = roughness * roughness;
    float alphaSq = alpha * alpha;
    
    return alphaSq / (PI * pow((NdotH * NdotH) * (alphaSq - 1) + 1, 2));
}

// F (Fresnel reflection)
float3 F_Schlick(float3 H, float3 V, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - saturate(dot(H, V)), 5.0);
}

// GSub (SchlickGGX)
float G_Sub(float3 N, float3 V, float k)
{
    float NdotV = dot(N, V);
    return NdotV / (NdotV * (1.0 - k) + k);
}

// G (Geometric Attenuation Function)
float G_SchlickGGX(float3 N, float3 V, float3 L, float roughness)
{
    float alpha = roughness * roughness;
    float k = pow((alpha + 1), 2) / 8.0; // directinoal light
    
    return G_Sub(N, V, k) * G_Sub(N, L, k);
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
    if(useMetallic)
        metallic = metallicMap.Sample(samLinear, input.texCoord).r;

    // roughness
    if(useRoughness)
        roughness = roughnessMap.Sample(samLinear, input.texCoord).r;
    
    // alpha
    if(useDiffuse)
        alpha = diffuseMap.Sample(samLinear, input.texCoord).a;
    
    
    // --- [Override] ----------------------------------
    if(useMetallicOverride) metallic = metallicOverride;
    if(useRoughnessOverride) roughness = roughnessOverride;
    
    
    // --- [Facotr] -----------------------------------
    metallic *= metallicFactor;
    roughness *= roughnessFactor;
    

    
    // --- [Vector]  ----------------------------------
    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 H = normalize(L + V);
    
    
    // --- [BRDF]  ----------------------------------
    // Specular BRDF (Cook-Torrance)
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), base_color, metallic); 
    float D = D_NDFGGXTR(N, H, roughness);
    float3 F = F_Schlick(H, V, F0);
    float G = G_SchlickGGX(N, V, L, roughness);
    
    float3 SpecularBRDF = (D * F * G) / (4.0f * max(dot(N, V), Epsilon) * max(dot(N, L), Epsilon) + Epsilon);

    
    // Diffuse BRDF (Lambertian)
    float3 DiffuseBRDF = (1.0f - F) * (1.0f - metallic) * (base_color / PI);
    

    // final color
    float3 finalColor = (SpecularBRDF + DiffuseBRDF) * lightColor * dot(N, L);
    return float4(finalColor, alpha);
}
