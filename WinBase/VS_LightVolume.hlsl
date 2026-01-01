// [ Light Volume Vertex Shader ]
// Deferred Rendering의 LightingPass에서 사용되는 Light Volume의 Vertex Shader

#include <shared.fxh>

PS_FullScreen_Input main(VS_Position_INPUT input)
{
    PS_FullScreen_Input output = (PS_FullScreen_Input) 0;
    
    output.position = mul(float4(input.position, 1.0f), world);
    output.position = mul(output.position, view);
    output.position = mul(output.position, projection);
    
    // uv
    float4 clip = output.position;
    float2 ndc = clip.xy / clip.w;
    output.uv = ndc * 0.5f + 0.5f;
    output.uv.y = 1.0f - output.uv.y;

    return output;
}