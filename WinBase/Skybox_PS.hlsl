#include <shared.fxh>

// Unlit, Diffuse Texture PixelShader

TextureCube skyboxTexture : register(t4);
SamplerState samplerLinear : register(s0);

float4 main(PS_Skybox_INPUT input) : SV_TARGET
{
    // πÊ«‚ ∫§≈Õ∑Œ CubeMap ª˘«√
    return skyboxTexture.Sample(samplerLinear, normalize(input.texCoord));
}
