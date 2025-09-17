#include <shared.fxh>

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 materialColor = { 1, 1, 1, 1 };
    return saturate(dot((float3) lightDirection, input.normal) * (lightColor * materialColor));
}