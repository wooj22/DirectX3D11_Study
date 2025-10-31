cbuffer SkyboxCB : register(b1)
{
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
    float3 texCoord : TEXCOORD0;        // CubeMap ª˘«√øÎ πÊ«‚ ∫§≈Õ
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;

    output.position = mul(float4(input.position, 1.0f), view);
    output.position = mul(output.position, projection);

    // CubeMapøÎ πÊ«‚ ∫§≈Õ
    output.texCoord = input.position;

    return output;
}
