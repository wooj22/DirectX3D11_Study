#include "Shared.fxh"

PS_INPUT main(float4 pos : POSITION)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.pos = mul(pos, world); // local -> world
    output.pos = mul(output.pos, view); // world -> view
    output.pos = mul(output.pos, projection); // view -> clip
    return output;
}