#include <shared.fxh>

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 materialColor = txColorMap.Sample(samLinear, input.texCoord);
    return saturate(dot((float3) lightDirection, input.normal) * (lightColor * materialColor));
}