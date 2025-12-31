// [ Light Volume Vertex Shader ]
// Deferred Rendering의 LightingPass에서 사용되는 Light Volume의 Vertex Shader

#include <shared.fxh>

PS_LightVolume_INPUT main(VS_Position_INPUT input)
{
    PS_LightVolume_INPUT output;
    
    output.position = mul(float4(input.position, 1.0f), world);
    output.position = mul(output.position, view);
    output.position = mul(output.position, projection);
    
    return output;
}