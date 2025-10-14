#include <shared.fxh>

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specualrMap : register(t2);
SamplerState samLinear : register(s0);


float4 main(PS_INPUT input) : SV_TARGET
{
    // tbn matrix
    float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
    
    // world nomal
    float3 local_normal = normalMap.Sample(samLinear, input.texCoord).xyz * 2.0f - 1.0f;
    float3 world_normal = normalize(mul(local_normal, TBN));
    
    float3 N = normalize(world_normal);
    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 H = normalize(L + V);

    // ambient
    float3 ambient = indirectLight * ambientHighlight * lightColor.rgb;

    // diffuse
    float3 diffuse_color = diffuseMap.Sample(samLinear, input.texCoord);
    float diff = max(dot(N, L), 0.0f);
    float3 diffuse = directLight * diffuseHighlight * diffuse_color * diff * lightColor.rgb;
   
    // specular
    float3 specular_color = specualrMap.Sample(samLinear, input.texCoord);
    float spec = pow(max(dot(N, H), 0.0f), shininess);
    float3 specular = directLight * specularHighlight * specular_color * spec * lightColor.rgb;

    // final color (ºí¸°Æþ)
    float3 finalColor = ambient + diffuse + specular;
    return float4(finalColor, 1.0f);
}