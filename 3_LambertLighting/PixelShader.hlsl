#include <shared.fxh>

float4 main(PS_INPUT input) : SV_TARGET
{
    return input.color = (1.0f, 1.0f, 1.0f);
}