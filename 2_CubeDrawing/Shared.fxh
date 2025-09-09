// .fxh file
// ���̴� ���ϵ��� �����ϴ� ����ü�� ���ǵ��� ����

// constant buffer �������
// vs���� ������ ��� �����͵��� cpu�� �����Ѵ�.
// ���� ��Ģ ����!
cbuffer ConstantBuffer : register(b0)
{
    matrix world;           // float 4x4, 64byte
    matrix view;            // flaot 4x4, 64Byte
    matrix projection;      // flaot 4x4, 64Byte
}

// vs output, ps input
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};