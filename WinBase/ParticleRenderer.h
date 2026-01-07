#pragma once
#include "Effect.h"
#include "QuadMesh.h"
#include <vector>
using namespace std;

class ParticleRenderer // 현재는 Filpbook 기준
{
private:
    QuadMesh quad;

public:
    void ParticlePass(const vector<Effect*>& effects);
};

