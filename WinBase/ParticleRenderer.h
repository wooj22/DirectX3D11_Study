#pragma once
#include "Effect.h"
#include "EffectQuadMesh.h"
#include <vector>
using namespace std;

class ParticleRenderer // 현재는 Filpbook 기준
{
private:
    EffectQuadMesh quad;

public:
    void ParticlePass(const vector<Effect*>& effects);
};

