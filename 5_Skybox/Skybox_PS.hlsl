TextureCube skyboxTexture : register(t1);
SamplerState samplerLinear : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // ���� ���ͷ� CubeMap ����
    return skyboxTexture.Sample(samplerLinear, normalize(input.texCoord));
}
