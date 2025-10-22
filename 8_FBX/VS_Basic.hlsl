#include <shared.fxh>

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
     // clip position
    output.pos = mul(float4(input.pos, 1.0f), world); // local -> world
    output.worldPos = output.pos.xyz; // (world pos ÀúÀå)
    output.pos = mul(output.pos, view); // world -> view
    output.pos = mul(output.pos, projection); // view -> clip
    
    // world TBN
    output.tangent = normalize(mul(input.tangent, (float3x3) world));
    output.bitangent = normalize(mul(input.bitangent, (float3x3) world));
    output.normal = normalize(mul(input.normal, (float3x3) world));
    
    // uv
    output.texCoord = input.texCoord;

    return output;
}