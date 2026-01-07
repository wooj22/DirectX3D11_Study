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
*/

class ParticleRenderer // 현재는 Filpbook 기준
{
private:
    ParticleQuadMesh quad;
    ComPtr<ID3D11Buffer> instanceBuffer;

public:
    ParticleRenderer();
    void Init() {}
    void ParticlePass(const vector<Effect*>& effects);
};