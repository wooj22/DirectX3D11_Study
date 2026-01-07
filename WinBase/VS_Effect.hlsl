#include <shared.fxh>

float2 Rotate2D(float2 v, float rad)
{
    float s = sin(rad);
    float c = cos(rad);
    return float2(v.x * c - v.y * s, v.x * s + v.y * c);
}

PS_Particle_INPUT main(VS_Particle_INPUT input)
{
    PS_Particle_INPUT output = (PS_Particle_INPUT) 0;
    
    // Frame
    uint f = (useInstanceFrame != 0) ? (uint) input.frame : (uint) frameIndex;
    if (totalFrames > 0)
        f = min(f, (uint) (totalFrames - 1));

    // Flipbook UV
    uint cols = (uint) max(1.0, atlasGrid.x);
    uint rows = (uint) max(1.0, atlasGrid.y);

    uint fx = f % cols;
    uint fy = f / cols; // row
    fy = min(fy, rows - 1);

    float2 uv = input.uv * invAtlasGrid + float2(fx, fy) * invAtlasGrid;

    // Billboard basis (Right/Up)
    float3 camRight = float3(view._11, view._21, view._31);
    float3 camUp = float3(view._12, view._22, view._32);

    // Translation
    float2 local = input.corner * (input.size * baseSizeScale);
    local = Rotate2D(local, input.rotation);

    float3 worldPos = input.pos + camRight * local.x + camUp * local.y;

    float4 viewPos = mul(float4(worldPos, 1.0f), view);
    float4 clipPos = mul(viewPos, projection);

    output.pos = clipPos;
    output.uv = uv;
    output.color = input.color;
    
    return output;
}