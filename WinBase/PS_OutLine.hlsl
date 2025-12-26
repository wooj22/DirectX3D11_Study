// OutLine Pixel Shader

#include <shared.fxh>

float4 main(PS_OutLine_INPUT input) : SV_TARGET
{
    //float3 outlineColor = float3(1.0f, 1.0f, 1.0f);
    return float4(outlineColor, 1.0f);
}