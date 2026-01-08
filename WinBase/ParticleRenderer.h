#pragma once
#include "Effect.h"
#include "ParticleQuadMesh.h"
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
using std::vector;
using Microsoft::WRL::ComPtr;

/*
    [ Particle Renderer ]

     Quad(Vertex Buffer) 하나로 여러 Particle(Instance Buffer)를 그린다.
     넘겨받은 effect 배열에서 살아있는 particle의 데이터로 instance 배열을 만들고,
     Map을 통해

     ** Local Batch **
      일단은 전체 배칭을 하지 않고(키 생성, 정렬, 다른 렌더러도 지금 배칭 안하고있음의 이유)
      Effect별로 Draw Call을 호출하기로 함! -> 한 이펙트 안의 Particle들은 Draw Call 한번으로 처리
      배칭 키로 사용될게 결국 SRV랑 Billboard모드인데 그게 Effect 단위니까.

      즉,
      - 현재 : Effect별 DrawCall
      - 추후 : Bating을 통해 여러 Effect 모든 파티클을 타입별로 Draw Call
*/

class ParticleRenderer // 현재는 Filpbook 기준
{
private:
    ParticleQuadMesh quad;
    ComPtr<ID3D11Buffer> instanceBuffer = nullptr;

public:
    void Init();
    void ParticlePass(const Matrix& view, const Matrix& projection, const vector<Effect>& effects);
};