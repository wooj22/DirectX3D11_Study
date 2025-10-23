#include <shared.fxh>

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D emissiveMap : register(t3);

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
        N = normalize(mul(input.normal, (float3x3) world));
    }

    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 H = normalize(L + V);

    
    // ambient
    float3 ambient = indirectLight * ambientHighlight * lightColor.rgb;

    
    // diffuse
    if (useDiffuse)
        diffuse_color = diffuseMap.Sample(samLinear, input.texCoord).rgb;
    float diff = max(dot(N, L), 0.0f);
    float3 diffuse = directLight * diffuseHighlight * diffuse_color * diff * lightColor.rgb;

    
    // specular
    if (useSpecular)
        specular_color = specularMap.Sample(samLinear, input.texCoord).rgb;
    float spec = pow(max(dot(N, H), 0.0f), shininess);
    float3 specular = directLight * specularHighlight * specular_color * spec * lightColor.rgb;

    
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
