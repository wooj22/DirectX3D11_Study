#include <PBR_Common.fxh>

// Unlit, Diffuse Texture PixelShader

TextureCube skyboxTexture : register(t4);
SamplerState samplerLinear : register(s0);

float4 main(PS_Skybox_INPUT input) : SV_TARGET
{
    float3 color = skyboxTexture.Sample(samplerLinear, normalize(input.texCoord));
    
    // LDR 단독패스일 때만 감마보정
    if (useDefaultGamma && !isHDR)
        color = LinearToSRGB(color);
    
    return float4(color, 1);
}
