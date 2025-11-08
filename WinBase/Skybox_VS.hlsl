#include <shared.fxh>

PS_Skybox_INPUT main(VS_Skybox_INPUT input)
{
    PS_Skybox_INPUT output;

    output.position = mul(float4(input.position, 1.0f), view);
    output.position = mul(output.position, projection);

    // CubeMap용 방향 벡터
    output.texCoord = input.position;

    return output;
}
