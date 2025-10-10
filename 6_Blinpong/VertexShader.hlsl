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
    
    float3 cameraPos;
}

struct VS_INPUT
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 worldPos : WORLD_POSITION;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    // clip position
    output.pos = mul(input.pos, world);         // local -> world
    output.worldPos = output.pos.xyz;               // (world pos ÀúÀå)
    output.pos = mul(output.pos, view);         // world -> view
    output.pos = mul(output.pos, projection);   // view -> clip
    
    // world normal
    output.normal = normalize(mul(input.normal, (float3x3) world));
    
    // uv
    output.texCoord = input.texCoord;
    
    return output;
}