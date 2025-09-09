// .fxh file
// 쉐이더 파일들이 공유하는 구조체와 정의들을 포함

// constant buffer 상수버퍼
// vs에서 연산할 행렬 데이터들을 cpu와 맵핑한다.
// 정렬 규칙 주의!
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