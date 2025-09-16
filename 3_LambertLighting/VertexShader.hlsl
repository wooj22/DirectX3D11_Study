#include "Shared.fxh"

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    // clip position
    output.pos = mul(input.pos, world);         // local -> world
    output.pos = mul(output.pos, view);         // world -> view
    output.pos = mul(output.pos, projection);   // view -> clip
    
    // world normal
    output.normal = normalize(mul(input.normal, (float3x3) world));
    
    return output;
}