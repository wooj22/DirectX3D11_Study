#include <shared.fxh>

PS_INPUT main(VS_Weight_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    // TODO :: ½ºÅ°´×!!!
    
     // clip space
    output.pos = mul(float4(input.pos, 1.0f), world); // local -> world
    output.worldPos = output.pos.xyz;             // (world pos ÀúÀå)
    output.pos = mul(output.pos, view);           // world -> view
    output.pos = mul(output.pos, projection);     // view -> clip
    
    // world TBN
    float3 tangent = normalize(mul(input.tangent, (float3x3) world));
    float3 bitangent = normalize(mul(input.bitangent, (float3x3) world));
    output.normal = normalize(mul(input.normal, (float3x3) world));
    output.TBN = float3x3(tangent, bitangent, output.normal);
    
    // uv
    output.texCoord = input.texCoord;

    return output;
}