/*
    [ PBR Geomatry Pass Pixel Shader ] 
    * Deferred Rendering *
    
    MTR을 활용하여 라이팅 연산에 필요한 정보들을 Gbuffer에 기록
    TODO :: position 기록 삭제
*/

#include <shared.fxh>

// --- Texture Bind Slot ------------------
Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D emissiveMap : register(t3);
Texture2D shadowMap : register(t6);
Texture2D metallicMap : register(t7);
Texture2D roughnessMap : register(t8);

// --- Sampler Bind Slot ------------------
SamplerState samLinear : register(s0);



PS_Output main(PS_INPUT input) : SV_TARGET
{
    PS_Output output = (PS_Output) 0;
    
    // --- [ Default ] ----------------------------------
    
    float3 base_color = float3(1.0f, 1.0f, 1.0f);
    float3 normal = float3(0, 0, 0);
    float3 emissive = float3(0.0f, 0.0f, 0.0f);
    float metallic = 0.0f;
    float roughness = 0.0f;
    float alpha = 1.0f;
    
    
    // ---[ Material Texture ]----------------------------------
    
    // base color
    if (useDiffuse)
    {
        base_color = diffuseMap.Sample(samLinear, input.texCoord).rgb;
        alpha = diffuseMap.Sample(samLinear, input.texCoord).a;
    }
    
    if (alpha < 0.5)
        discard;
    
    // normal
    if (useNormal)
    {
        float3 local_normal = normalMap.Sample(samLinear, input.texCoord).xyz * 2.0f - 1.0f;
        float3 world_normal = normalize(mul(local_normal, input.TBN));
        normal = normalize(world_normal);
    }
    else
    {
        normal = normalize(mul(input.normal, (float3x3) input.finalWorld));
    }
    
    // emission
    if (useEmissive)
        emissive = emissiveMap.Sample(samLinear, input.texCoord).rgb;
    
    // metallic
    if (useMetallic)
        metallic = metallicMap.Sample(samLinear, input.texCoord).r;

    // roughness
    if (useRoughness)
    {
        roughness = roughnessMap.Sample(samLinear, input.texCoord).r;
        if (roughnessFromShininess)
            roughness = 1 - roughness;
    }
    
    
    // --- [Override] ----------------------------------
    
    if (useBaseColorOverride)
        base_color = baseColorOverride;
    if (useEmissiveOverride)
        emissive = emissiveOverride;
    if (useMetallicOverride)
        metallic = metallicOverride;
    if (useRoughnessOverride)
        roughness = roughnessOverride;
    
    roughness = max(roughness, 0.04);
    
    // --- [Factor] -----------------------------------
    
    emissive *= emissiveFactor;
    metallic *= metallicFactor;
    float rf = max(roughnessFactor, 0.04);
    roughness *= rf;


    // ---[ Write G-Buffer ]----------------------------------

    output.WorldPos   = float4(input.worldPos, 1.0f);
    output.Base_color = float4(base_color, alpha);
    output.Normal     = float4(normal, 0);
    output.Material   = float4(metallic, roughness, 0, 0);
    output.Emissive   = float4(emissive, 1.0f);

    return output;
}
