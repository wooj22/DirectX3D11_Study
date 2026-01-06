#pragma once
#include <vector>
using namespace std;

class QuadMesh;
class Effect;

class ParticleRenderer // 현재는 Filpbook 기준
{
private:
    QuadMesh quad;

public:
    void ParticlePass(const vector<Effect*>& effects);
};

