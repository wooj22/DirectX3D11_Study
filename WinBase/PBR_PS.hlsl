#include <PBRCommom.fxh>
#include <shared.fxh>


// BlinnPhong Pixel Shader
// ShadowMapping (현재 연계 가능한 VS : BaseLit_Skinned_VS, BaseLit_Static_VS)

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D emissiveMap : register(t3);
Texture2D shadowMap : register(t6);

SamplerState samLinear : register(s0);
SamplerComparisonState samShadow : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
    // defualt color
    float3 diffuse_color = float3(1.0f, 1.0f, 1.0f);
    float3 specular_color = float3(1.0f, 1.0f, 1.0f);
    float3 emissive_color = float3(0.0f, 0.0f, 0.0f);
    float alpha = 1.0f;
    
    // shadowFactor
    float shadowFactor = 1.0f;
    
    // ShadowMapping
    float currentShadowDepth = input.posShadow.z / input.posShadow.w;
    float2 uv = input.posShadow.xy / input.posShadow.w;
    uv.y = -uv.y;
    uv = uv * 0.5 + 0.5;
    
    if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0)
    {
        // 단일 texel
        //float shadowMapDepth = shadowMap.Sample(samLinear, uv).r;
        
        //if (currentShadowDepth > shadowMapDepth + 0.001)
        //    shadowFactor = 0.0f;
        //else
        //    shadowFactor = 1.0;
        
        // PCF
        shadowFactor = shadowMap.SampleCmpLevelZero(samShadow, uv, currentShadowDepth - 0.001);
    }
    
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
    float3 finalColor = (diffuse + specular) * shadowFactor + ambient + emissive_color;
    return float4(finalColor, alpha);
}
