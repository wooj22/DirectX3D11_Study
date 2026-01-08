#pragma once
#include "FilpbookEffect.h"
#include "ParticleQuadMesh.h"
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
using std::vector;
using Microsoft::WRL::ComPtr;

// Filpbook Effect Àü¿ë Renderer
class FilpbookParticleRenderer
{
private:
    ParticleQuadMesh quad;
    ComPtr<ID3D11Buffer> instanceBuffer = nullptr;

public:
    void Init();
    void ParticlePass(const Matrix& view, const Matrix& projection, const vector<FilpbookEffect>& effects);
};

