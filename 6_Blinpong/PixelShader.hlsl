Texture2D txColorMap : register(t0);
SamplerState samLinear : register(s0);

// constant buffer
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    
    float4 lightDirection;
    float4 lightColor;
    
    float indirectLight;
    float directLight;
    
    float ambientReflection;
    float diffuseReflection;
    float specularReflection;
    float shininess;
    float2 padding1;
    
    float3 cameraPos;
    float padding2;
}

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 worldPos : WORLD_POSITION;
};

// ºí¸°Æþ ½¦ÀÌµù
float4 main(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPos - input.worldPos);
    float3 H = normalize(L + V);

    // È®»ê±¤
    float3 ambient = indirectLight * ambientReflection * lightColor.rgb;

    // ³­¹Ý»ç±¤
    float diff = max(dot(N, L), 0.0f);
    float3 diffuse = directLight * diffuseReflection * diff * lightColor.rgb;
   
    // Á¤¹Ý»ç±¤
    float spec = pow(max(dot(N, H), 0.0f), shininess);
    float3 specular = directLight * specularReflection * spec * lightColor.rgb;

    // ÃÖÁ¾ »ö»ó
    float3 finalLight = ambient + diffuse + specular;
    float3 textureColor = txColorMap.Sample(samLinear, input.texCoord);
    
    float3 finalColor = finalLight * textureColor;
    return float4(finalColor, 1.0f);
}