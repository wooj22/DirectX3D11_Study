#pragma once
#include <iostream>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxtk/simplemath.h>
#include <directxtk/CommonStates.h>
#include <directxtk/Effects.h>
#include "../WinBase/DebugDraw.h"
using namespace std;
using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;
using namespace DirectX;

class FrustumRenderer
{
private:
    unique_ptr<CommonStates>  m_states;
    unique_ptr<PrimitiveBatch<VertexPositionColor>> m_batch;
    unique_ptr<BasicEffect>   m_effect;
    ComPtr<ID3D11InputLayout> m_layout = nullptr;
public:
    void Init(const Matrix& frustumView, const Matrix& frustumProj);

    // Debug Draw
    void FrustumDebugDraw(const Matrix& frustumView, const Matrix& frustumProj,
        const Matrix& renderView, const Matrix& renderProj, FXMVECTOR color = Colors::Red);
};

