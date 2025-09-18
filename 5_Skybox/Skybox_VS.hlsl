cbuffer SkyboxCB : register(b1)
{
    matrix world;
    matrix view;
    matrix projection;
};

struct VS_INPUT
{
    float3 position : POSITION;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD0; // CubeMap ª˘«√øÎ πÊ«‚ ∫§≈Õ
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    float4 worldPos = mul(float4(input.position, 1.0f), world);
    output.position = mul(worldPos, view);
    output.position = mul(output.position, projection);

    // CubeMapøÎ πÊ«‚ ∫§≈Õ (World Space)
    output.texCoord = input.position;

    return output;
}
