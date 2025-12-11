#include <shared.fxh>

// BlinnPhong, Toon Shading Pixel Shader
// Diffuse와 Specualr를 툰쉐이딩 합니다 (Level, RampTexuture)

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D emissiveMap : register(t3);

Texture2D diffuseRamp : register(t4);
Texture2D specualrRamp : register(t5);

SamplerState samLinear : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    // defualt
    float3 diffuse_color = float3(1.0f, 1.0f, 1.0f);
    float3 specular_color = float3(1.0f, 1.0f, 1.0f);
    float3 emissive_color = float3(0.0f, 0.0f, 0.0f);
    float alpha = 1.0f;
    
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

    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 H = normalize(L + V);

    
    // ambient
    float3 ambient = indirectIntensity * ambientHighlight * lightColor.rgb;

    
    // diffuse (toon shading)
    // 1) level로 단계적 음영을 주는 방법
    //int levels = 3;
    //float scaleFactor = 1.0 / (float) levels;
    //float diff = floor(saturate(dot(N, L)) * levels) * scaleFactor;

    // 2) 램프 텍스처로 색상 매핑
    float3 diff = diffuseRamp.Sample(samLinear, float2(saturate(dot(N, L)), 0)).rgb;
    
    if (useDiffuse)
        diffuse_color = diffuseMap.Sample(samLinear, input.texCoord).rgb;
    float3 diffuse = directIntensity * diffuseHighlight * diffuse_color * diff * lightColor.rgb;

    
    // specular (toon shading)
    // 1) level로 단계적 음영을 주는 방법
    int specLevels = 3;
    float scaleFactor = 1.0 / (float) specLevels;
    float spec = floor(pow(saturate(dot(N, H)), shininess) * specLevels) * scaleFactor;
    
     // 2) 램프 텍스처로 색상 매핑
    //float spec = specualrRamp.Sample(samLinear, float2(pow(saturate(dot(N, H)), shininess), 0)).rbg;
    
    if (useSpecular)
        specular_color = specularMap.Sample(samLinear, input.texCoord).rgb;
    float3 specular = directIntensity * specularHighlight * 0.5 * specular_color * spec * lightColor.rgb;

    
    // emissive
    if (useEmissive)
        emissive_color = emissiveMap.Sample(samLinear, input.texCoord).rgb;

    // alpha
    if (useDiffuse)
        alpha = diffuseMap.Sample(samLinear, input.texCoord).a;
    
    // final color
    float3 finalColor = ambient + diffuse + specular + emissive_color;
    return float4(finalColor, alpha);
}
