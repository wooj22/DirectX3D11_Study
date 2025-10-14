#include <shared.fxh>

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specualrMap : register(t2);
SamplerState samLinear : register(s0);

// ºí¸°Æþ ½¦ÀÌµù
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

    // ambient light
    float3 ambient = indirectLight * ambientReflection * lightColor.rgb;

    // diffuse light
    float diff = max(dot(N, L), 0.0f);
    float3 diffuse = directLight * diffuseReflection * diff * lightColor.rgb;
   
    // specular light
    float spec = pow(max(dot(N, H), 0.0f), shininess);
    float3 specular = directLight * specularReflection * spec * lightColor.rgb;

    // final color
    float3 finalLight = ambient + diffuse + specular;
    float3 textureColor = diffuseMap.Sample(samLinear, input.texCoord);
    
    float3 finalColor = finalLight * textureColor;
    return float4(finalColor, 1.0f);
}