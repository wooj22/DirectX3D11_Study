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

     ** Effect Batch **
     Effect의 Sprite Sheet, Billboard Mode에 따라 DrawCall을 묶어서 호출한다.
     -> CB 상태가 다르기 때문에
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