#include <shared.fxh>

// StaticMesh의 ShadowMap (Depth Texture) 생성을 위한 VertexShader
// 그림자매핑의 첫번쨰 Pass에서 사용되며, PixelShader는 실행되지 않는다.

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    // wolrd
    Matrix finalWorld = mul(model, world);
    output.finalWorld = finalWorld;
    
    // view clip space (shadowView, shadowProjection)
    output.pos = mul(float4(input.pos, 1.0f), finalWorld);
    output.worldPos = output.pos;
    output.pos = mul(output.pos, shadowView);
    output.pos = mul(output.pos, shadowProjection);

    return output;
}